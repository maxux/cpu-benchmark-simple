#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include "cpubench.h"

int verbflag = 0;

static struct option long_options[] = {
    {"json",    no_argument, 0, 'j'},
    {"verbose", no_argument, 0, 'v'},
    {"help",    no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

typedef struct benchmark_t {
    struct timeval time_begin;
    struct timeval time_end;
    uint64_t seed;
    uint64_t final;
    size_t length;

} benchmark_t;

#define verbose(...) { if(verbflag) { printf(__VA_ARGS__); } }

void diep(char *str) {
    perror(str);
    exit(EXIT_FAILURE);
}

static double time_spent(struct timeval *timer) {
    return (((size_t) timer->tv_sec * 1000000) + timer->tv_usec) / 1000000.0;
}

benchmark_t *benchmark(benchmark_t *source) {
    gettimeofday(&source->time_begin, NULL);

    size_t values = 64 * 1024 * 1024;
    uint64_t seed = source->seed;

    source->length = values * sizeof(seed);

    for(size_t offset = 0; offset < values; offset++) {
        seed = crc64((uint8_t *) &seed, sizeof(seed));
    }

    source->final = seed;

    gettimeofday(&source->time_end, NULL);

    return source;
}

int main(int argc, char *argv[]) {
    int option_index = 0;
    int json = 0;

    while(1) {
        int i = getopt_long_only(argc, argv, "", long_options, &option_index);

        if(i == -1)
            break;

        switch(i) {
            case 'j':
                json = 1;
                break;

            case 'v':
                verbflag = 1;
                break;

            case 'h':
                printf("Usage: %s [-jvh]\n\n", argv[0]);
                printf(" -j     JSON output\n");
                printf(" -v     Verbose output\n");
                printf(" -h     This help message\n");
                return 1;

            case '?':
            default:
               exit(EXIT_FAILURE);
        }

    }

    verbose(COLOR_CYAN "[+] initializing grid-benchmark-simple v%s client" COLOR_RESET "\n", CPUBENCH_VERSION);
    uint64_t seed = 0x0a0b0c0d12345ff0;

    verbose("[+] seed: 0x%016lx\n", seed);

    benchmark_t cpubench = {
        .seed = seed,
    };

    // single-thread
    verbose("[+] testing single-thread\n");
    benchmark(&cpubench);

    double stimed = time_spent(&cpubench.time_end) - time_spent(&cpubench.time_begin);
    verbose("[+] single thread score: %.3f\n", stimed);

    // multi-thread
    long cpucount = sysconf(_SC_NPROCESSORS_ONLN);
    double totaltime = 0;

    verbose("[+] testing multi-threads (%ld threads)\n", cpucount);

    #pragma omp parallel for num_threads(cpucount)
    for(long a = 0; a < cpucount; a++) {
        benchmark_t cpubench = {
            .seed = seed,
        };

        benchmark(&cpubench);

        double ltimed = time_spent(&cpubench.time_end) - time_spent(&cpubench.time_begin);
        verbose("\r[+] multi-threads unique thread score: %.3f\n", ltimed);

        #pragma omp atomic
        totaltime += ltimed;
    }

    verbose("\r[+] multi-threads score: %.3f\n", totaltime);

    if(json) {
        printf("{\"single\": %.3f, \"multi\": %.3f, \"threads\": %ld}\n", stimed, totaltime, cpucount);

    } else {
        printf("Single thread score: %.3f\n", stimed);
        printf("Multi threads score: %.3f [%ld threads]\n", totaltime, cpucount);
    }

    return 0;
}
