#include "cachelab.h"
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DECIMAL_BASE 10
#define HEX_BASE 16
#define LINELEN 21

/**
 * @brief cache line
 */
typedef struct {
    bool valid;
    bool dirty;
    unsigned long tag;
    unsigned long time;

} cache_line;

cache_line **cache;

long set_number;
unsigned long block_size;
long associativity;
long set_bits;
long block_bits;
char *file_name = NULL;
csim_stats_t *stats;
bool is_v_mode = false;

unsigned long LRU_timer = 0;

/**
 * @brief Initialize statistics
 */
void initStats(void) {
    stats = malloc(sizeof(csim_stats_t));
    stats->hits = 0;
    stats->misses = 0;
    stats->evictions = 0;
    stats->dirty_bytes = 0;
    stats->dirty_evictions = 0;
}
/**
 * @brief free stats
 */
void freeStats(void) {
    free(stats);
}

/**
 * Initialize Cache
 */
void initCache(void) {
    set_number = 1 << set_bits;
    block_size = 1 << block_bits;

    cache = malloc((unsigned long)set_number * sizeof(cache_line *));

    for (long i = 0; i < set_number; i++) {
        cache[i] = malloc((unsigned long)associativity * sizeof(cache_line));
        for (long j = 0; j < associativity; j++) {
            cache[i][j].valid = false;
            cache[i][j].tag = 0;
            cache[i][j].time = 0;
            cache[i][j].dirty = false;
        }
    }
}

/**
 * @brief cache memory
 */
void freeCache(void) {
    for (long i = 0; i < set_number; i++) {
        free(cache[i]);
    }

    free(cache);
}

long findHit(unsigned long tag, long set_index) {
    for (long i = 0; i < associativity; i++) {

        if (cache[set_index][i].valid == true &&
            cache[set_index][i].tag == tag) {
            return i;
        }
    }
    return -1;
}

long findMiss(unsigned long tag, long set_index) {
    for (long i = 0; i < associativity; i++) {

        if (cache[set_index][i].valid == false) {
            return i;
        }
    }
    return -1;
}

long findEviction(unsigned long tag, long set_index) {
    unsigned long min = cache[set_index][0].time;
    long min_timer_index = 0;

    for (long i = 0; i < associativity; i++) {
        if (cache[set_index][i].time < min) {
            min = cache[set_index][i].time;
            min_timer_index = i;
        }
    }
    return min_timer_index;
}

/**
 * @brief access data and do the statistics
 */
void processData(char operation, unsigned long address, unsigned long size) {
    unsigned long tag = address >> (set_bits + block_bits);

    long set_index =
        (address >> block_bits) & ((1 << set_bits) - 1); // extract bits

    // hits option
    long hit_index = findHit(tag, set_index);

    if (hit_index != -1) {
        stats->hits++;
        LRU_timer++;
        cache[set_index][hit_index].time = LRU_timer;

        if (operation == 'S') {
            if (cache[set_index][hit_index].dirty == false) {
                stats->dirty_bytes += block_size;
                cache[set_index][hit_index].dirty = true;
            }
        }

        if (is_v_mode)
            printf("hits\n");

        return;
    }
    stats->misses++;

    long miss_index = findMiss(tag, set_index);

    if (miss_index != -1) {
        cache[set_index][miss_index].valid = true;
        cache[set_index][miss_index].tag = tag;
        LRU_timer++;
        cache[set_index][miss_index].time = LRU_timer;
        if (operation == 'S') {
            cache[set_index][miss_index].dirty = true;
            stats->dirty_bytes += block_size;
        }
        if (is_v_mode)
            printf("miss\n");

        return;
    }

    // eviction operation
    long eviction_index = findEviction(tag, set_index);

    if (cache[set_index][eviction_index].dirty) {
        stats->dirty_evictions += block_size;
        stats->dirty_bytes -= block_size;
    }
    if (operation == 'S') {
        cache[set_index][eviction_index].dirty = true;
        stats->dirty_bytes += block_size;

    } else {
        cache[set_index][eviction_index].dirty = false;
    }

    cache[set_index][eviction_index].tag = tag;
    cache[set_index][eviction_index].valid = true;
    LRU_timer++;
    cache[set_index][eviction_index].time = LRU_timer;

    stats->evictions++;

    if (is_v_mode)
        printf("eviction\n");
}

/** Process a memory-access trace file.
 *
 * @param trace Name of the trace file to process
 * @return 0 if successful, 1 if there were error
 */
int process_trace_file(const char *trace) {

    FILE *tfp = fopen(trace, "rt");
    if (!tfp) {
        fprintf(stderr, "Error opening '%s': %s\n", trace, strerror(errno));
        return 1;
    }
    char linebuf[LINELEN];
    int parse_error = 0;
    while (fgets(linebuf, LINELEN, tfp)) {
        char operation = *strtok(linebuf, " ,");
        unsigned long address = strtoul(strtok(NULL, " ,"), NULL, HEX_BASE);
        unsigned long size = strtoul(strtok(NULL, " ,"), NULL, HEX_BASE);

        if (is_v_mode)
            printf("%c %lu, %lu ", operation, address, size);

        processData(operation, address, size);
    }
    fclose(tfp);
    return parse_error;
}

/**
 * @brief print help message
 */
void printHelp(void) {
    printf("Mandatory arguments missing or zero.\n");
    printf("Usage: ./csim [-v] -s <s> -b <b> -E <E> -t <trace>\n");
    printf("       ./csim -h\n");
    printf("    -h          Print this help message and exit\n");
    printf("    -v          Verbose mode: report effects of each memory "
           "operation\n");
    printf("    -s <s>      Number of set index bits (there are 2**s sets)\n");
    printf("    -b <b>      Number of block bits (there are 2**b blocks)\n");
    printf("    -E <E>      Number of lines per set (associativity)\n");
    printf("    -t <trace>  File name of the memory trace to process\n\n");
    printf("The -s, -b, -E, and -t options must be supplied for all "
           "simulations.\n");
}

/**
 * @brief main function
 *
 * @param [in] argv Command Line argument,  [-v] -s <s> -E <E> -b <b> -t <trace>
 */
int main(int argc, char *argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "vhs:E:b:t:")) != -1) {
        switch (opt) {
        case 'v':
            printf("This is v mode\n");
            is_v_mode = true;

            break;

        case 'h':
            printHelp();
            break;

        case 's':
            set_bits = strtol(optarg, NULL, DECIMAL_BASE);
            break;

        case 'E':
            associativity = strtol(optarg, NULL, DECIMAL_BASE);
            break;

        case 'b':
            block_bits = strtol(optarg, NULL, DECIMAL_BASE);

            break;

        case 't':
            file_name = optarg;
            break;

        case ':':
            printf("This is : option\n");
            printHelp();
            break;

        case '?':
            printf("This is ? option\n");
            printHelp();

            break;

        default:
            printf("This is defalut\n");
            printHelp();

            break;
        }
    }
    initCache();
    initStats();
    process_trace_file(file_name);

    printSummary(stats);

    freeCache();
    freeStats();

    return 0;
}
