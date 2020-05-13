
#include <benchmark/benchmark.h>
#include <inttypes.h>

void JulianToYMD_Fortran(int jd, int* y, int* m, int* d);
int64_t YMDToUnix_Fortran(int year, int month, int day, int h, int m, int s);
int64_t YMDToUnix_Table(int year, int month, int day, int h, int m, int s);
int64_t YMDToUnix_DaysFromCivil(int year, int month, int d, int h, int m, int s);
int64_t YMDToUnix_Fast(int year, int month, int d, int h, int m, int s);

static int64_t YMDToUnix_Libc(int year, int month, int d, int h, int m, int s) {
  struct tm time;
  time.tm_sec = s;
  time.tm_min = m;
  time.tm_hour = h;
  time.tm_mday = d;
  time.tm_mon = month - 1;
  time.tm_year = year - 1900;
  return timegm(&time);
}

// Benchmarking code. //////////////////////////////////////////////////////////

template <int64_t F(int, int, int, int, int, int)>
void BenchmarkAlgorithm(benchmark::State& state) {
  for (unsigned i = 0; state.KeepRunning(); i++) {
    int year = i & 0x7ff;
#ifdef __APPLE__
    year = 1900 + (i & 0x7f);
#endif
    int64_t val = F(year, i & 7, i & 0xf, i & 0xf, i & 0x1f, i & 0x1f);
    benchmark::DoNotOptimize(val);
  }
}

#ifdef THREADS
#define THREAD_ARG ->Threads(THREADS)
#else
#define THREAD_ARG
#endif

static void BM_YMDToUnix_Fortran(benchmark::State& state) {
  BenchmarkAlgorithm<YMDToUnix_Fortran>(state);
}
BENCHMARK(BM_YMDToUnix_Fortran) THREAD_ARG;

static void BM_YMDToUnix_Table(benchmark::State& state) {
  BenchmarkAlgorithm<YMDToUnix_Table>(state);
}
BENCHMARK(BM_YMDToUnix_Table) THREAD_ARG;

static void BM_YMDToUnix_DaysFromCivil(benchmark::State& state) {
  BenchmarkAlgorithm<YMDToUnix_DaysFromCivil>(state);
}
BENCHMARK(BM_YMDToUnix_DaysFromCivil) THREAD_ARG;

static void BM_YMDToUnix_Fast(benchmark::State& state) {
  BenchmarkAlgorithm<YMDToUnix_Fast>(state);
}
BENCHMARK(BM_YMDToUnix_Fast) THREAD_ARG;

static void BM_timegm_libc(benchmark::State& state) {
  BenchmarkAlgorithm<YMDToUnix_Libc>(state);
}
BENCHMARK(BM_timegm_libc) THREAD_ARG;

// main() / verification code. /////////////////////////////////////////////////

template <int64_t F(int, int, int, int, int, int)>
static void TestAlgorithm(int year, int month, int day, int h, int m, int s,
                          int64_t unix_time, const char *name) {
  int64_t val = F(year, month, day, h, m, s);
  if (val != unix_time) {
    printf("%s(%d, %d, %d, %d, %d, %d) = %" PRId64 " != %" PRId64 "\n",
           name, year, month, day, h, m, s, val, unix_time);
  }
}

static void TestAllAlgorithms(int year, int month, int day, int h, int m, int s,
                              int64_t unix_time) {
  bool test_gmtime = true;

  TestAlgorithm<YMDToUnix_Fast>(year, month, day, h, m, s, unix_time,
                                "YMDToUnix_Fast");
  TestAlgorithm<YMDToUnix_Table>(year, month, day, h, m, s, unix_time,
                                 "YMDToUnix_Table");
  TestAlgorithm<YMDToUnix_DaysFromCivil>(year, month, day, h, m, s, unix_time,
                                         "YMDToUnix_DaysFromCivil");
  TestAlgorithm<YMDToUnix_Fortran>(year, month, day, h, m, s, unix_time,
                                   "YMDToUnix_Fortran");

#ifdef __APPLE__
  // On macOS timegm() returns -1 for years before 1900.
  // Also we don't want to run it too many times or it takes too long to get
  // to the benchmarking part.
  test_gmtime = year >= 1900 && year < 1950;
#endif

  if (test_gmtime) {
    TestAlgorithm<YMDToUnix_Libc>(year, month, day, h, m, s, unix_time,
                                  "YMDToUnix_Libc");
  }
}

static void TestAllAlgorithmsForJulianDay(int64_t jd) {
  int64_t unix_day = jd - 2440588;
  int64_t unix_time = unix_day * 86400;
  int y, m, d;

  JulianToYMD_Fortran(jd, &y, &m, &d);
  TestAllAlgorithms(y, m, d, 0, 0, 0, unix_time);
}

int main(int argc, char** argv) {
  // Ensure non-inlined versions are linked in so we can size-profile them.
  benchmark::DoNotOptimize(&YMDToUnix_Fortran);
  benchmark::DoNotOptimize(&YMDToUnix_Fast);
  benchmark::DoNotOptimize(&YMDToUnix_Table);
  benchmark::DoNotOptimize(&YMDToUnix_DaysFromCivil);

  // Check all algorithms for correctness.
  for (int64_t jd = 0; jd < 10000000; jd++) {
    TestAllAlgorithmsForJulianDay(jd);
  }

  // jd=1000000000 -> 2733194-11-27
  TestAllAlgorithms(1000000, 1, 1, 0, 0, 0, 31494784780800);

  // Test around a leap second boundary.
  // UTC                      Unix Time
  // 1998-12-31T23:59:59.00   915148799
  // 1998-12-31T23:59:60.00   915148800
  // 1999-01-01T00:00:00.00   915148800
  TestAllAlgorithms(1998, 12, 31, 23, 59, 59, 915148799);
  TestAllAlgorithms(1998, 12, 31, 23, 59, 60, 915148800);
  TestAllAlgorithms(1999, 1, 1, 0, 0, 0, 915148800);

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
