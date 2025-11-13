# Design and Implementation of a Configurable Cache Simulator for Memory Hierarchy Analysis

## Abstract

Modern CPUs rely heavily on cache performance; this project develops a simulator which is supposed to mimic the behavior of a hardware cache subsystem. This simulator is written in C language, due to it being an optimal choice between convenience and control.

The goal of this project is to simulate and analyze the effects of cache parameters on hit/miss performance and to explore the trade-offs in memory hierarchy design. 

## Introduction

### What is Cache ?

A cache is a small, fast storage device that acts as a staging area for the data objects in a larger, slower device. Caches are orders of magnitude faster than DRAM and as such are very important for speeding the overall operation of CPU without making CPU wait.

### Purpose of simulator 

A cache simulator is a valuable tool for computer architecture for several reasons:
    1. Performance Analysis
    2. Optimization of programs
    3. Explore different type of design.

### Some helpful definitions 

**Cache Hits** :- When a program needs a particular data object `d` from level `k+1`, it first looks for d in one of the blocks currently stored at level `k`. If `d` happens to be cached at level `k`, then that act is called cache hit.

**Cache Miss** :- If not found on level `k` then it is said as cache miss.



TODO: complete the theory part.
---

# Version 1: L1 Direct-Mapped cache Simulator in C

This is a simple L1 data cache simulator, written in C as an exercise to understand the core princiles of the cache memory hierarchies.

This simulator read Valgrind memory traces and reports the total number of hits, misses, and eviction.


It uses fgets and sscanf for parsing the trace file.
```c
void accessCache(unsigned long long address, int s_bits, int b_bits) {
  // printf("Accessing address: 0x%llx\n", address);
  // Finding the address_tag.
  unsigned long long address_tag = address >> (s_bits + b_bits);
  
  // Finding the Index mask
  unsigned long long set_index_mask = (1 << s_bits) - 1;
  unsigned long long address_index = (address >> b_bits) & set_index_mask;

  cache_line_t* line = &cache[address_index];
  
  // Hit miss logic.

  if (line->valid_bit == 1 && line->tag == address_tag) {
    hit_count++;
  }
  else {
    miss_count++;

    if(line->valid_bit == 1) {
      eviction_count++;
    }
    line->valid_bit = 1;
    line->tag = address_tag;
  }
}
```
Access cache is the core engine of the program.

It uses bit operations to seperate the tag, frame, offset from the memory address.

and then uses it to verify it's presence in our cache data structure for cache hit or miss.

```c
typedef struct {
    int valid_bit;
    unsigned long long tag;
   
} cache_line_t;

// A cache -- an array of cache_line_t
cache_line_t* cache;
```


