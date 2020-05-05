
#include <benchmark/benchmark.h>

void JulianToYMD_Fortran(int jd, int *y, int *m, int *d);
int YMDToJulian_Fortran(int y, int m, int d);
int YMDToUnix_Table(int year, int month, int day);
int YMDToUnix_DaysFromCivil(int y, int m, int d);
int YMDToUnix_Fast(int y, int m, int d);

// Benchmarking code. //////////////////////////////////////////////////////////

template <int F(int, int, int)>
int64_t UnixTime(int year, int month, int day, int h, int m, int s) {
  return F(year, month, day) + (h * 3600) + (m * 60) + s;
}

template <int F(int, int, int)>
void BenchmarkAlgorithm(benchmark::State& state) {
  for (unsigned i = 0; state.KeepRunning(); i++) {
    int64_t val = UnixTime<F>(i, i & 7, i, i, i, i);
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

static void BM_gmtime_libc(benchmark::State& state) {
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
BENCHMARK(BM_gmtime_libc);

// main() / verification code. /////////////////////////////////////////////////

int main(int argc, char** argv) {
  // Ensure non-inlined versions are linked in so we can size-profile them.
  benchmark::DoNotOptimize(&YMDToUnix_Fast);
  benchmark::DoNotOptimize(&YMDToUnix_Table);
  benchmark::DoNotOptimize(&YMDToJulian_Fortran);
  benchmark::DoNotOptimize(&YMDToUnix_DaysFromCivil);

  // Check all algorithms for correctness.
  for (int jd = 0; jd < 10000000; jd++) {
    int unix_day = jd - 2440588;
    int y, m, d;
    JulianToYMD_Fortran(jd, &y, &m, &d);
    if (YMDToUnix_Fast(y, m, d) != unix_day) {
      printf("YMDToUnix_Fast(%d, %d, %d) = %d != %d\n", y, m, d,
             YMDToUnix_Fast(y, m, d), unix_day);
    }
    if (YMDToUnix_Table(y, m, d) != unix_day) {
      printf("YMDToUnix_Table(%d, %d, %d) = %d != %d\n", y, m, d,
             YMDToUnix_Table(y, m, d), unix_day);
    }
    if (YMDToUnix_DaysFromCivil(y, m, d) != unix_day) {
      printf("YMDToUnix_DaysFromCivil(%d, %d, %d) = %d != %d\n", y, m, d,
             YMDToUnix_DaysFromCivil(y, m, d), unix_day);
    }
    if (YMDToJulian_Fortran(y, m, d) != jd) {
      printf("YMDToJulian_Fortran(%d, %d, %d) = %d != %d\n", y, m, d,
             YMDToJulian_Fortran(y, m, d), jd);
    }
  }

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
