# Algorithms for UTC -> UnixTime

This directory contains benchmarks for the algorithms published in
[Optimizing UTC → Unix Time Conversion For Size And
Speed](http://blog.reverberate.org/2020/05/12/optimizing-date-algorithms.html).

The `main()` function validates the correctness of all the algorithms
before running benchmarks.

The benchmark requires
[Bazel](https://github.com/bazelbuild/bazel/releases) to build.
At build time Bazel will automatically download and build the
benchmarking framework.

From this directory, run:

```
$ bazel build -c opt :benchmark
$ ../bazel-bin/date-algorithms/benchmark
```

To run the benchmark multithreaded, run:

```
$ bazel build -c opt --copt=-DTHREADS=6 :benchmark
$ ../bazel-bin/date-algorithms/benchmark
```

## Benchmark Results

Here are benchmark results from the time this article was published
(May 2020).

### Linux

This was run on a Linux Desktop i7-8700K with glibc 2.30.

```
Running bazel-bin/date-algorithms
Run on (12 X 4700 MHz CPU s)
CPU Caches:
  L1 Data 32K (x6)
  L1 Instruction 32K (x6)
  L2 Unified 256K (x6)
  L3 Unified 12288K (x1)
------------------------------------------------------------------
Benchmark                           Time           CPU Iterations
------------------------------------------------------------------
BM_YMDToUnix_Fortran                5 ns          5 ns  148069867
BM_YMDToUnix_Table                  2 ns          2 ns  288071123
BM_YMDToUnix_DaysFromCivil          4 ns          4 ns  176474060
BM_YMDToUnix_Fast                   3 ns          3 ns  269155478
BM_timegm_libc                     46 ns         46 ns   15360235
```

### macOS

This was run on a MacBook Pro running macOS 10.15.4.

```
Running bazel-bin/date-algorithms
Run on (12 X 2600 MHz CPU s)
CPU Caches:
  L1 Data 32K (x6)
  L1 Instruction 32K (x6)
  L2 Unified 262K (x6)
  L3 Unified 9437K (x1)
------------------------------------------------------------------
Benchmark                           Time           CPU Iterations
------------------------------------------------------------------
BM_YMDToUnix_Fortran                6 ns          6 ns  116857534
BM_YMDToUnix_Table                  3 ns          3 ns  213395767
BM_YMDToUnix_DaysFromCivil          5 ns          5 ns  147238851
BM_YMDToUnix_Fast                   3 ns          3 ns  211537241
BM_timegm_libc                   5421 ns       5413 ns     122749
```

Here is the same test with six threads:

```
Running bazel-bin/date-algorithms
Run on (12 X 2600 MHz CPU s)
CPU Caches:
  L1 Data 32K (x6)
  L1 Instruction 32K (x6)
  L2 Unified 262K (x6)
  L3 Unified 9437K (x1)
----------------------------------------------------------------------------
Benchmark                                     Time           CPU Iterations
----------------------------------------------------------------------------
BM_YMDToUnix_Fortran/threads:6                1 ns          6 ns  119878410
BM_YMDToUnix_Table/threads:6                  1 ns          3 ns  206599374
BM_YMDToUnix_DaysFromCivil/threads:6          1 ns          5 ns  139911858
BM_YMDToUnix_Fast/threads:6                   1 ns          3 ns  208116546
BM_timegm_libc/threads:6                  14795 ns      86855 ns       8178
```
