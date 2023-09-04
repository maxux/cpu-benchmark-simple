#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "cpubench.h"

typedef struct benchmark_t {
    struct timeval time_begin;
    struct timeval time_end;
    uint64_t seed;
    uint64_t final;
    size_t length;

} benchmark_t;

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
    printf(COLOR_CYAN "[+] initializing grid-cpu-benchmark-simple" COLOR_RESET "\n");

    uint64_t seed = 0x0a0b0c0d12345ff0;

    printf("[+] seed: 0x%016lx\n", seed);

    benchmark_t cpubench = {
        .seed = seed,
    };

    // single-thread
    printf("[+] testing single-thread\n");
    benchmark(&cpubench);

    {
    double timed = time_spent(&cpubench.time_end) - time_spent(&cpubench.time_begin);
    printf("[+] single thread score: %.3f\n", timed);
    }

    // multi-thread
    long cpucount = sysconf(_SC_NPROCESSORS_ONLN);
    double totaltime = 0;

    printf("[+] testing multi-threads (%ld threads)\n", cpucount);

    #pragma omp parallel for num_threads(cpucount)
    for(long a = 0; a < cpucount; a++) {
        benchmark_t cpubench = {
            .seed = seed,
        };

        benchmark(&cpubench);

        double timed = time_spent(&cpubench.time_end) - time_spent(&cpubench.time_begin);

        #pragma omp atomic
        totaltime += timed;
    }

    printf("\r[+] multi-threads score: %.3f\n", totaltime);

    return 0;
}
