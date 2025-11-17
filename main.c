#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h> 

typedef struct {
    int valid_bit;
    unsigned long long tag;
    unsigned long long lru_counter;
   
} cache_line_t;

// A cache -- an array of cache_line_t
cache_line_t* cache;


// Global variables
int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;
int verbose = 0;

unsigned long long global_lru_timestamp = 0;

void printUsage() {
    printf("Usage: ./cache_simulator [-hv] -s <s> -b <b> -t <tracefile>\n");
    printf("  -h: Optional help flag\n");
    printf("  -v: Optional verbose flag (prints trace info)\n");
    printf("  -s <s>: Number of set index bits (S = 2^s)\n");
    printf("  -E <E>: Associativity (Number of lines per set)\n");
    printf("  -b <b>: Number of block offset bits (B = 2^b)\n");
    printf("  -t <tracefile>: Name of the valgrind trace file\n");

}

void accessCache(unsigned long long address, int s_bits, int b_bits, int E_lines);

int main(int argc, char** argv) {
    int s_bits = 0;     // Number of set index bits
    int E_lines = 0;
    int b_bits = 0;     // Number of block offset bits
    char* trace_file = NULL;
    
  
  char c;
    while ((c = getopt(argc, argv, "hvs:b:E:t:")) != -1) {
        switch (c) {
            case 's':
                s_bits = atoi(optarg);
                break;
            case 'b':
                b_bits = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'E':// argument for Associativity
                E_lines = atoi(optarg);
                break;
            case 'h':
                printUsage();
                exit(0);
            default:
                printUsage();
                exit(1);
        }
    }

    // Check if all required arguments are present
    if (s_bits == 0 || E_lines == 0 || b_bits == 0 || trace_file == NULL) {
        printf("Error: Missing required arguments.\n");
        printUsage();
        exit(1);
    }

  int S = 1 << s_bits; //Documents and Settings
  
  // Total nimber of lines in the cache is S * E

  // We will treat this 1D array as a 2D array.
  // The set as "cache[Index]" starts at "cache[Index * E_lines]"

  cache = (cache_line_t*) calloc(S * E_lines, sizeof(cache_line_t));
    if (cache == NULL) {
        printf("Error: Could not allocate memory for cache.\n");
        exit(1);
    }

    printf("Cache parameters:\n");
    printf("  s_bits (s): %d\n", s_bits);
    printf("  b_bits (b): %d\n", b_bits);
    printf("  E_lines (E): %d\n", E_lines);
    printf("  trace_file: %s\n", trace_file);
    printf("  verbose:    %d\n", verbose);
    printf("\n");
    printf("Cache calculated:\n");
    printf("  Sets (S):   %d\n", S);
    printf("  (Block size B will be %llu bytes)\n", (1ULL << b_bits));
    printf("\n");


  // Reading the file.
    FILE* file_pointer;

    file_pointer = fopen(trace_file, "r");
    
    if(file_pointer == NULL) {
      printf("Error: trace file not found.");
      exit(1);
    }
    
  // Reading the file line by line.
    char line_buffer[256];
    char op;
    unsigned long long addr;
    int size;
    while((fgets(line_buffer, sizeof(line_buffer), file_pointer))) {
      if (line_buffer[0] != ' ') continue;
      if (sscanf(line_buffer, " %c %llx,%d", &op, &addr, &size) < 3) continue;
      if (verbose) printf("%c %llx %d", op, addr, size);

      switch (op) {
        case 'L':
          accessCache(addr, s_bits, b_bits, E_lines);
          break;
        case 'S':
          accessCache(addr, s_bits, b_bits, E_lines);  
          break;
        case 'M':
          accessCache(addr, s_bits, b_bits, E_lines);
          accessCache(addr, s_bits, b_bits, E_lines);
          break;
        case 'I':
          continue;
        default:
          continue;
      }
      if(verbose) {
        printf("\n");
      }
    }
  fclose(file_pointer);

    printf("Summary (for now):\n");
    printf("Hits: %d\n", hit_count);
    printf("Misses: %d\n", miss_count);
    printf("Evictions: %d\n", eviction_count);


    free(cache);

    return 0;
}

void accessCache(unsigned long long address, int s_bits, int b_bits, int E_lines) {
  // printf("Accessing address: 0x%llx\n", address);
  // Finding the address_tag.
  global_lru_timestamp++; // new edit

  // Decomposing the address
  unsigned long long address_tag = address >> (s_bits + b_bits);
  unsigned long long set_index_mask = (1 << s_bits) - 1;
  unsigned long long address_index = (address >> b_bits) & set_index_mask;
  
  for (int i = 0; i < E_lines; i++) {
    cache_line_t* line = &cache[(address_index * E_lines) + i];

    if(line->valid_bit == 1 && line->tag == address_tag) {
      hit_count++;
      if(verbose) printf(" hit");
      line->lru_counter = global_lru_timestamp;
      return;
    }
  }
  miss_count++;
  if(verbose) printf(" miss");

  for(int i = 0; i < E_lines; i++) {
    cache_line_t* line = &cache[(address_index * E_lines) + i];
    if(line->valid_bit == 0) {
      line->valid_bit = 1;
      line->tag = address_tag;
      line->lru_counter = global_lru_timestamp;
      return;
    }
  }

  // Handle Evictions
  eviction_count++;
  if(verbose) printf(" eviction");


  // Finding the smallest counter
  int lru_index = 0;
  unsigned long long lru_time = cache[(address_index * E_lines) + 0].lru_counter;

  for (int i = 1; i < E_lines; i++) {
    cache_line_t* line = &cache[(address_index * E_lines) + i];
    if(line->lru_counter < lru_time) {
      lru_time = line->lru_counter;
      lru_index = i;
    }
  }

  cache_line_t* lowest_line = &cache[(address_index * E_lines) + lru_index];

  lowest_line->tag = address_tag;
  lowest_line->lru_counter = global_lru_timestamp;

  return;

  //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
  // LOCIG OF THE DIRECT MAPPED FOR REFERENCE
  //
  // cache_line_t* line = &cache[address_index];
  //
  // // Hit miss logic.
  //
  // if (line->valid_bit == 1 && line->tag == address_tag) {
  //   hit_count++;
  // }
  // else {
  //   miss_count++;
  //
  //   if(line->valid_bit == 1) {
  //     eviction_count++;
  //   }
  //   line->valid_bit = 1;
  //   line->tag = address_tag;
  // }
  //<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><> 
}
