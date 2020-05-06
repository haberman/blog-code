
#include <benchmark/benchmark.h>
#include <inttypes.h>

void JulianToYMD_Fortran(int jd, int* y, int* m, int* d);
int64_t YMDToJulian_Fortran(int year, int month, int day, int h, int m, int s);
int64_t YMDToUnix_Table(int year, int month, int day, int h, int m, int s);
int64_t YMDToUnix_DaysFromCivil(int year, int month, int d, int h, int m, int s);
int64_t YMDToUnix_Fast(int year, int month, int d, int h, int m, int s);

// Benchmarking code. //////////////////////////////////////////////////////////

template <int64_t F(int, int, int, int, int, int)>
void BenchmarkAlgorithm(benchmark::State& state) {
  for (unsigned i = 0; state.KeepRunning(); i++) {
    int64_t val = F(i, i & 7, i, i, i, i);
    benchmark::DoNotOptimize(val);
  }
}

static void BM_YMDToJulian_Fortran(benchmark::State& state) {
  BenchmarkAlgorithm<YMDToJulian_Fortran>(state);
}
BENCHMARK(BM_YMDToJulian_Fortran);

static void BM_YMDToUnix_Table(benchmark::State& state) {
  BenchmarkAlgorithm<YMDToUnix_Table>(state);
}
BENCHMARK(BM_YMDToUnix_Table);

static void BM_YMDToUnix_Fast(benchmark::State& state) {
  BenchmarkAlgorithm<YMDToUnix_Fast>(state);
}
BENCHMARK(BM_YMDToUnix_Fast);

static void BM_YMDToUnix_DaysFromCivil(benchmark::State& state) {
  BenchmarkAlgorithm<YMDToUnix_DaysFromCivil>(state);
}
BENCHMARK(BM_YMDToUnix_DaysFromCivil);

static void BM_timegm_libc(benchmark::State& state) {
  for (unsigned i = 0; state.KeepRunning(); i++) {
    struct tm time;
    time.tm_sec = 0;
    time.tm_min = 0;
    time.tm_hour = 0;
    time.tm_mday = 0;
    time.tm_mon = 0;
    time.tm_year = (int)i - 1900;

    time_t value = timegm(&time);
    benchmark::DoNotOptimize(value);
  }
}
BENCHMARK(BM_timegm_libc);

// main() / verification code. /////////////////////////////////////////////////

int main(int argc, char** argv) {
  // Ensure non-inlined versions are linked in so we can size-profile them.
  benchmark::DoNotOptimize(&YMDToJulian_Fortran);
  benchmark::DoNotOptimize(&YMDToUnix_Fast);
  benchmark::DoNotOptimize(&YMDToUnix_Table);
  benchmark::DoNotOptimize(&YMDToUnix_DaysFromCivil);

  // Check all algorithms for correctness.
  for (int64_t jd = 0; jd < 10000000; jd++) {
    int64_t unix_day = jd - 2440588;
    int64_t unix_time = unix_day * 86400;
    int64_t julian_time = jd * 86400;
    int y, m, d;
    JulianToYMD_Fortran(jd, &y, &m, &d);
    if (YMDToUnix_Fast(y, m, d, 0, 0, 0) != unix_time) {
      printf("YMDToUnix_Fast(%d, %d, %d) = %" PRId64 " != % " PRId64 "\n",
             y, m, d, YMDToUnix_Fast(y, m, d, 0, 0, 0), unix_time);
    }
    if (YMDToUnix_Table(y, m, d, 0, 0, 0) != unix_time) {
      printf("YMDToUnix_Table(%d, %d, %d) = %" PRId64 " != % " PRId64 "\n",
             y, m, d, YMDToUnix_Table(y, m, d, 0, 0, 0), unix_time);
    }
    if (YMDToUnix_DaysFromCivil(y, m, d, 0, 0, 0) != unix_time) {
      printf("YMDToUnix_DaysFromCivil(%d, %d, %d) = %" PRId64 " != % " PRId64 "\n",
             y, m, d, YMDToUnix_DaysFromCivil(y, m, d, 0, 0, 0), unix_time);
    }
    if (YMDToJulian_Fortran(y, m, d, 0, 0, 0) != julian_time) {
      printf("YMDToJulian_Fortran(%d, %d, %d) = %" PRId64 " != %" PRId64 "\n",
             y, m, d, YMDToJulian_Fortran(y, m, d, 0, 0, 0), julian_time);
    }
  }

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
