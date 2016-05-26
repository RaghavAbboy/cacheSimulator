//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.h"
#include <stdio.h>

//
// TODO:Student Information
//
const char *studentName = "Raghav Abboy";
const char *studentID   = "A53081298";
const char *email       = "rabboy@eng.ucsd.edu";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;     // Number of sets in the I$
uint32_t icacheAssoc;    // Associativity of the I$
uint32_t icacheHitTime;  // Hit Time of the I$

uint32_t dcacheSets;     // Number of sets in the D$
uint32_t dcacheAssoc;    // Associativity of the D$
uint32_t dcacheHitTime;  // Hit Time of the D$

uint32_t l2cacheSets;    // Number of sets in the L2$
uint32_t l2cacheAssoc;   // Associativity of the L2$
uint32_t l2cacheHitTime; // Hit Time of the L2$
uint32_t inclusive;      // Indicates if the L2 is inclusive

uint32_t blocksize;      // Block/Line size
uint32_t memspeed;       // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

// Structures to build a cache
struct way {
  uint32_t valid;
  uint32_t tag;
  uint32_t lru;
};

struct set {
  struct way *ways;
};

struct cache {
  struct set* sets;
};

struct cache icache;
struct cache dcache;
struct cache l2cache;

//Utilities
//Loop variables
int i,j;
//Validity flags
int iValid, dValid, l2Valid;
//Current L1 cache
char currCache;

//Bit lengths
int icacheIndexBits;
int dcacheIndexBits;
int l2cacheIndexBits;
int blockOffsetBits;

//Current Index and Tag values
uint32_t icacheIndex;
uint32_t icacheTag;

uint32_t dcacheIndex;
uint32_t dcacheTag;

uint32_t l2cacheIndex;
uint32_t l2cacheTag;

//------------------------------------//
//          Helper Functions          //
//------------------------------------//

//Function to get the next vacant way in a set, returns -1 if there is none
int getVacantWayIndex (char cacheType, int setIndex) {
  int wayIndex = -1;
  struct set set_curr;

  switch(cacheType) {
    case 'I':
      set_curr = icache.sets[setIndex];
      //search for a vacant spot
      for(j=0; j<icacheAssoc; j++) {
        if(set_curr.ways[j].valid == 0) { return j; }
      }
      break;
    case 'D':
      set_curr = dcache.sets[setIndex];
      //search for a vacant spot
      for(j=0; j<dcacheAssoc; j++) {
        if(set_curr.ways[j].valid == 0) { return j; }
      }
      break;
    case 'L':
      set_curr = l2cache.sets[setIndex];
      //search for a vacant spot
      for(j=0; j<l2cacheAssoc; j++) {
        if(set_curr.ways[j].valid == 0) { return j; }
      }
      break;
    default:
      break;
  }
  return wayIndex;
}

//Function to get the LRU way index in a set
int getLRUwayIndex (char cacheType, int setIndex) {
  int wayIndex;
  int lruCandidate;
  struct set set_curr;


  switch(cacheType) {
    case 'I':
      set_curr = icache.sets[setIndex];
      //find the way which is LRU
      lruCandidate = icacheAssoc - 1;
      for(j=0; j<icacheAssoc; j++) {
        if(set_curr.ways[j].lru == lruCandidate) { return j; }
      }
      break;
    case 'D':
      set_curr = dcache.sets[setIndex];
      //find the way which is LRU
      lruCandidate = dcacheAssoc - 1;
      for(j=0; j<dcacheAssoc; j++) {
        if(set_curr.ways[j].lru == lruCandidate) { return j; }
      }
      break;
    case 'L':
      set_curr = l2cache.sets[setIndex];
      //find the way which is LRU
      lruCandidate = l2cacheAssoc - 1;
      for(j=0; j<l2cacheAssoc; j++) {
        if(set_curr.ways[j].lru == lruCandidate) { return j; }
      }
      break;
    default:
      break;
  }
  return 0;
}

//Function to get the next candidate way index in a set for replacement
int getWayIndex (char cacheType, int setIndex) {
  int wayIndex = getVacantWayIndex(cacheType, setIndex);

  //If a vacant spot is found, return that way Index
  if(wayIndex != -1) { return wayIndex; }

  //If no vacant spot, get the LRU way Index
  wayIndex = getLRUwayIndex(cacheType, setIndex);
  return wayIndex;
}

//Function to access wayIndex, thereby updating the LRU values in a set
void accessAndUpdateLRU (char cacheType, int setIndex, int wayIndex) {
  struct set set_curr;
  struct way way_curr;
  int oldLru;

  switch(cacheType) {
    case 'I':
      // printf("I cache LRU update.\n");
      set_curr = icache.sets[setIndex];
      way_curr = set_curr.ways[wayIndex];
      oldLru = way_curr.lru;

      for(j=0; j<icacheAssoc; j++) {
        if(j==wayIndex) {icache.sets[setIndex].ways[j].lru = 0; continue; }
        if(icache.sets[setIndex].ways[j].lru < oldLru) { icache.sets[setIndex].ways[j].lru++; }
      }
      break;
    case 'D':
      // printf("D cache LRU update.\n");
      set_curr = dcache.sets[setIndex];
      way_curr = set_curr.ways[wayIndex];
      oldLru = way_curr.lru;

      for(j=0; j<dcacheAssoc; j++) {
        if(j==wayIndex) {dcache.sets[setIndex].ways[j].lru = 0; continue; }
        if(dcache.sets[setIndex].ways[j].lru < oldLru) { dcache.sets[setIndex].ways[j].lru++; }
      }
      break;
    case 'L':
      // printf("L2 cache LRU update.\n");
      set_curr = l2cache.sets[setIndex];
      way_curr = set_curr.ways[wayIndex];
      oldLru = way_curr.lru;

      for(j=0; j<l2cacheAssoc; j++) {
        if(j==wayIndex) {l2cache.sets[setIndex].ways[j].lru = 0; continue; }
        if(l2cache.sets[setIndex].ways[j].lru < oldLru) { l2cache.sets[setIndex].ways[j].lru++; }
      }
      break;
    default:
      break;
  }
}

//Functions to print out the contents of a cache
void print_icache() {
  if(!iValid) { return; }
  printf("-------------------I Cache------------------------\n");
  //print out cache contents
  for(i=0; i<icacheSets; i++) {
    //each set on a line
    struct set set_curr = icache.sets[i];
    printf("Set %d: ----> ", i);
    //print set's contents
    for(j=0; j<icacheAssoc; j++) {
      struct way way_curr = set_curr.ways[j];
      printf("| V:%d Tag:%d lru:%d |", way_curr.valid, way_curr.tag, way_curr.lru);
    }
    printf("\n");
  }
  printf("--------------------------------------------------\n");
}

void print_dcache() {
  if(!dValid) { return; }
  printf("-------------------D Cache------------------------\n");
  //print out cache contents
  for(i=0; i<dcacheSets; i++) {
    //each set on a line
    struct set set_curr = dcache.sets[i];
    printf("Set %d: ----> ", i);
    //print set's contents
    for(j=0; j<dcacheAssoc; j++) {
      struct way way_curr = set_curr.ways[j];
      printf("| V:%d Tag:%d lru:%d |", way_curr.valid, way_curr.tag, way_curr.lru);
    }
    printf("\n");
  }
  printf("--------------------------------------------------\n");
}

void print_l2cache() {
  if(!l2Valid) { return; }
  printf("-------------------L2 Cache------------------------\n");
  //print out cache contents
  for(i=0; i<l2cacheSets; i++) {
    //each set on a line
    struct set set_curr = l2cache.sets[i];
    printf("Set %d: ----> ", i);
    //print set's contents
    for(j=0; j<l2cacheAssoc; j++) {
      struct way way_curr = set_curr.ways[j];
      printf("| V:%d Tag:%d lru:%d |", way_curr.valid, way_curr.tag, way_curr.lru);
    }
    printf("\n");
  }
  printf("---------------------------------------------------\n");
}

// Calculate the logarithm of x to the base 2
int intLog2 (uint32_t x) {
  if(x <= 1) { return 0; }
  int pdt = 2;
  int result = 1;
  while (pdt < x) {
    result++;
    pdt *= 2;
  }
  return result;
}

//Calculate x^n
int power(int x, int n) {
  int i;
  int result = 1;
  for(i=0; i<n; i++) { result *= x; }
  return result;
}

//------------------------------------//
//          Cache Functions           //
//------------------------------------//

// Initialize the Cache Hierarchy
//
void init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;

  //Initialize cache by allocating memory
  printf("init_cache called.\n");

  iValid = !(icacheSets < 1 || icacheAssoc < 1);
  dValid = !(dcacheSets < 1 || dcacheAssoc < 1);
  l2Valid = !(l2cacheSets < 1 || l2cacheAssoc < 1);

  printf("Validity Flags - i:d:l2 = %d:%d:%d\n", iValid, dValid, l2Valid);

  //Calculate the number of bits for Index and BlockOffset
  icacheIndexBits = intLog2(icacheSets);
  dcacheIndexBits = intLog2(dcacheSets);
  l2cacheIndexBits = intLog2(l2cacheSets);
  blockOffsetBits = intLog2(blocksize);

  printf("Index Bits: I:%d, D:%d, L2:%d, #BlockOffset: %d\n", icacheIndexBits, dcacheIndexBits, l2cacheIndexBits, blockOffsetBits);

  //----------------------------------------------------------------------
  //Create icache
  if(iValid) {
    //Create an array of sets
    icache.sets = malloc(icacheSets * sizeof(struct set));
    //Initialize all sets to default values
    for(i=0; i<icacheSets; i++) {
      icache.sets[i].ways = malloc(icacheAssoc * sizeof(struct way));
      //Initialize each way with default values
      for(j=0; j<icacheAssoc; j++) {
        icache.sets[i].ways[j].valid = 0;
        icache.sets[i].ways[j].tag = 0;
        icache.sets[i].ways[j].lru = j;
      }
    }
  }
  //----------------------------------------------------------------------

  //----------------------------------------------------------------------
  //Create dcache
  if(dValid) {
    //Create an array of sets
    dcache.sets = malloc(dcacheSets * sizeof(struct set));
    //Initialize all sets to default values
    for(i=0; i<dcacheSets; i++) {
      dcache.sets[i].ways = malloc(dcacheAssoc * sizeof(struct way));
      //Initialize each way with default values
      for(j=0; j<dcacheAssoc; j++) {
        dcache.sets[i].ways[j].valid = 0;
        dcache.sets[i].ways[j].tag = 0;
        dcache.sets[i].ways[j].lru = j;
      }
    }
  }
  //----------------------------------------------------------------------

  //----------------------------------------------------------------------
  //Create l2cache
  if(l2Valid) {
    //Create an array of sets
    l2cache.sets = malloc(l2cacheSets * sizeof(struct set));
    //Initialize all sets to default values
    for(i=0; i<l2cacheSets; i++) {
      l2cache.sets[i].ways = malloc(l2cacheAssoc * sizeof(struct way));
      //Initialize each way with default values
      for(j=0; j<l2cacheAssoc; j++) {
        l2cache.sets[i].ways[j].valid = 0;
        l2cache.sets[i].ways[j].tag = 0;
        l2cache.sets[i].ways[j].lru = j;
      }
    }
  }
  //----------------------------------------------------------------------

  print_icache();
  print_dcache();
  print_l2cache();

  // printf("Vacant way for l2 cache at set 3: %d\n", getLRUwayIndex('L', 3));
  // accessAndUpdateLRU('L', 0, 7);
  // print_l2cache();
  // printf("LRU way index for D cache at set 11: %d\n", getLRUwayIndex('D', 11));
}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t icache_access(uint32_t addr)
{
  // printf("icache_access called ------------\n");
  // printf("Address: 0x%x\n", addr);

  //Base cases when icacheSets=0, icacheAssoc=0
  if(icacheSets == 0 || icacheAssoc == 0) { return l2cache_access(addr); }

  //Prepare variables
  int icacheAccessTime = 0;

  //Calculate index, tag
  uint32_t address = addr;

  //remove the block offset bits
  address = address >> blockOffsetBits;
  icacheIndex = (icacheIndexBits == 0)? 0 : address % power(2,icacheIndexBits);
  icacheTag = address / power(2,icacheIndexBits);

  // printf("Index: %d, Tag: %d\n", icacheIndex, icacheTag);

  //Check for a hit
  struct set set_curr = icache.sets[icacheIndex];
  for(j=0; j<icacheAssoc; j++) {
    struct way way_curr = set_curr.ways[j];
    if(way_curr.valid && way_curr.tag == icacheTag) {
      accessAndUpdateLRU('I', icacheIndex, j);
      return icacheHitTime;
    }
  }

  //Miss, so handle it appropriately
  //Add hit time to access time
  int wayIndex = getWayIndex('I', icacheIndex);
  icache.sets[icacheIndex].ways[wayIndex].valid = 1;
  icache.sets[icacheIndex].ways[wayIndex].tag = icacheTag;
  accessAndUpdateLRU('I', icacheIndex, wayIndex);

  icacheAccessTime = icacheHitTime + l2cache_access(addr);

  //TODO: Update icacheStatistics before returning
  //remove
  // print_icache();
  return icacheAccessTime;
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t dcache_access(uint32_t addr)
{
  // printf("dcache_access called ------------\n");
  // printf("Address: 0x%x\n", addr);

  //Base cases when icacheSets=0, icacheAssoc=0
  if(dcacheSets == 0 || dcacheAssoc == 0) { return l2cache_access(addr); }

  //Prepare variables
  int dcacheAccessTime = 0;

  //Calculate index, tag
  uint32_t address = addr;

  //remove the block offset bits
  address = address >> blockOffsetBits;
  dcacheIndex = (dcacheIndexBits == 0)? 0 : address % power(2,dcacheIndexBits);
  dcacheTag = address / power(2,dcacheIndexBits);

  // printf("Index: %d, Tag: %d\n", dcacheIndex, dcacheTag);

  //Check for a hit
  struct set set_curr = dcache.sets[dcacheIndex];
  for(j=0; j<dcacheAssoc; j++) {
    struct way way_curr = set_curr.ways[j];
    if(way_curr.valid && way_curr.tag == dcacheTag) {
      accessAndUpdateLRU('D', dcacheIndex, j);
      return dcacheHitTime;
    }
  }

  //Miss, so handle it appropriately
  //Add hit time to access time
  int wayIndex = getWayIndex('D', dcacheIndex);
  dcache.sets[dcacheIndex].ways[wayIndex].valid = 1;
  dcache.sets[dcacheIndex].ways[wayIndex].tag = dcacheTag;
  accessAndUpdateLRU('D', dcacheIndex, wayIndex);

  dcacheAccessTime = dcacheHitTime + l2cache_access(addr);

  //TODO: Update icacheStatistics before returning
  //remove
  //print_dcache();
  return dcacheAccessTime;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t l2cache_access(uint32_t addr)
{
  // printf("l2cache_access called ------------\n");
  // printf("Address: 0x%x\n", addr);

  //Base cases when l2cacheSets=0, l2cacheAssoc=0
  if(l2cacheSets == 0 || l2cacheAssoc == 0) { return memspeed; }

  //Prepare variables
  int l2cacheAccessTime = 0;

  //Calculate index, tag
  uint32_t address = addr;

  //remove the block offset bits
  address = address >> blockOffsetBits;
  l2cacheIndex = (l2cacheIndexBits == 0)? 0 : address % power(2,l2cacheIndexBits);
  l2cacheTag = address / power(2,l2cacheIndexBits);

  // printf("Index: %d, Tag: %d\n", l2cacheIndex, l2cacheTag);

  //Check for a hit
  struct set set_curr = l2cache.sets[l2cacheIndex];
  for(j=0; j<l2cacheAssoc; j++) {
    struct way way_curr = set_curr.ways[j];
    if(way_curr.valid && way_curr.tag == l2cacheTag) {
      accessAndUpdateLRU('L', l2cacheIndex, j);
      return l2cacheHitTime;
    }
  }

  //Miss, so handle it appropriately

  if(inclusive == 0) {
    //Non inclusive case
    int wayIndex = getWayIndex('L', l2cacheIndex);
    l2cache.sets[l2cacheIndex].ways[wayIndex].valid = 1;
    l2cache.sets[l2cacheIndex].ways[wayIndex].tag = l2cacheTag;
    accessAndUpdateLRU('L', l2cacheIndex, wayIndex);
  }
  else {
    //Inclusive case
    //Kick out the current block while checking for their presence in I/D caches

    //Get the address of the current block waiting to be kicked out
    uint32_t wayIndex = getWayIndex('L', l2cacheIndex);
    uint32_t oldTag = l2cache.sets[l2cacheIndex].ways[wayIndex].tag;

    uint32_t oldAddress = oldTag << l2cacheIndexBits;
    oldAddress = oldAddress | l2cacheIndex;
    // oldAddress = oldAddress << blockOffsetBits;

    //I-cache entry eviction
    //Calculate the index and tag of icache
    icacheIndex = (icacheIndexBits == 0)? 0 : oldAddress % power(2,icacheIndexBits);
    icacheTag = oldAddress / power(2,icacheIndexBits);

    //Check whether the same entry exists in I cache
    struct set set_curr = icache.sets[icacheIndex];
    for(j=0; j<icacheAssoc; j++) {
      struct way way_curr = set_curr.ways[j];
      if(way_curr.valid && way_curr.tag == icacheTag) {
        //If found, invalidate the entry
        icache.sets[icacheIndex].ways[j].valid = 0;
      }
    }

    //D-cache entry eviction
    //Calculate the index and tag of icache
    dcacheIndex = (dcacheIndexBits == 0)? 0 : oldAddress % power(2,dcacheIndexBits);
    dcacheTag = oldAddress / power(2,dcacheIndexBits);

    //Check whether the same entry exists in D cache
    set_curr = dcache.sets[dcacheIndex];
    for(j=0; j<dcacheAssoc; j++) {
      struct way way_curr = set_curr.ways[j];
      if(way_curr.valid && way_curr.tag == dcacheTag) {
        //If found, invalidate the entry
        dcache.sets[dcacheIndex].ways[j].valid = 0;
      }
    }

    //Now, replace the current entry with a new one
    wayIndex = getWayIndex('L', l2cacheIndex);
    l2cache.sets[l2cacheIndex].ways[wayIndex].valid = 1;
    l2cache.sets[l2cacheIndex].ways[wayIndex].tag = l2cacheTag;
    accessAndUpdateLRU('L', l2cacheIndex, wayIndex);

    // printf("Inclusive case implementation pending.\n");
  }

  //Add hit time to access time
  l2cacheAccessTime = l2cacheHitTime + memspeed;

  //TODO: Update l2cacheStatistics before returning
  //remove
  // print_l2cache();
  return l2cacheAccessTime;
}





