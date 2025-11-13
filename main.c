#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h> 
typedef struct {
    int valid_bit;
    unsigned long long tag;
   
} cache_line_t;

// A cache -- an array of cache_line_t
cache_line_t* cache;

int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;

void printUsage() {
    printf("Usage: ./cache_simulator [-hv] -s <s> -b <b> -t <tracefile>\n");
    printf("  -h: Optional help flag\n");
    printf("  -v: Optional verbose flag (prints trace info)\n");
    printf("  -s <s>: Number of set index bits (S = 2^s)\n");
    printf("  -b <b>: Number of block offset bits (B = 2^b)\n");
    printf("  -t <tracefile>: Name of the valgrind trace file\n");
}

void accessCache(unsigned long long address, int s_bits, int b_bits);

int main(int argc, char** argv) {
    int s_bits = 0;     // Number of set index bits
    int b_bits = 0;     // Number of block offset bits
    char* trace_file = NULL;
    int verbose = 0;
  
  char c;
    while ((c = getopt(argc, argv, "hvs:b:t:")) != -1) {
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
            case 'h':
                printUsage();
                exit(0);
            default:
                printUsage();
                exit(1);
        }
    }

    // Check if all required arguments are present
    if (s_bits == 0 || b_bits == 0 || trace_file == NULL) {
        printf("Error: Missing required arguments.\n");
        printUsage();
        exit(1);
    }

  int S = 1 << s_bits;

  cache = (cache_line_t*) calloc(S, sizeof(cache_line_t));
    if (cache == NULL) {
        printf("Error: Could not allocate memory for cache.\n");
        exit(1);
    }

    printf("Cache parameters:\n");
    printf("  s_bits (s): %d\n", s_bits);
    printf("  b_bits (b): %d\n", b_bits);
    printf("  trace_file: %s\n", trace_file);
    printf("  verbose:    %d\n", verbose);
    printf("\n");
    printf("Cache calculated:\n");
    printf("  Sets (S):   %d\n", S);
    printf("  (Block size B will be %llu bytes)\n", (1ULL << b_bits));
    printf("\n");

    FILE* file_pointer;

    file_pointer = fopen(trace_file, "r");
    
    if(file_pointer == NULL) {
      printf("Error: trace file not found.");
      exit(1);
    }
    
    char line_buffer[256];
    
    char op;
    unsigned long long addr;
    int size;
    while((fgets(line_buffer, sizeof(line_buffer), file_pointer))) {
      sscanf(line_buffer, " %c %llx,%d", &op, &addr, &size);
      switch (op) {
        case 'L':
          accessCache(addr, s_bits, b_bits);
          break;
        case 'S':
          accessCache(addr, s_bits, b_bits);  
          break;
        case 'M':
          accessCache(addr, s_bits, b_bits);
          accessCache(addr, s_bits, b_bits);
          break;
        case 'I':
          continue;
        default:
          continue;
      }
    }

    printf("Summary (for now):\n");
    printf("Hits: %d\n", hit_count);
    printf("Misses: %d\n", miss_count);
    printf("Evictions: %d\n", eviction_count);


    free(cache);

    return 0;
}

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
