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

//------------------------------------//
//          Helper Functions          //
//------------------------------------//
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
}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t icache_access(uint32_t addr)
{
  printf("icache_access called ------------\n");
  printf("Address: %x\n", addr);

  return memspeed;
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t dcache_access(uint32_t addr)
{
  //
  //TODO: Implement D$
  //
  return memspeed;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t l2cache_access(uint32_t addr)
{
  //
  //TODO: Implement L2$
  //
  return memspeed;
}

