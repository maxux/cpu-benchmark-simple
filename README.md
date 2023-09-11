# cpu-benchmark-simple

Execute a simple task (computing CRC64 in chain), in a single thread and multi-thread environment using SIMD and reports executing time.
This simple task can be used to compare raw CPU power using SIMD available on large set of CPU. This « benchmark » is primarily used to
compare sets of CPU on same task, not to really benchmark CPU on large sets of computing power.

# Usage

There are only two options available: `-j` to get json output and `-v` to get verbose output.

# Dependencies

There is no external dependencies than a C compiler needed.
