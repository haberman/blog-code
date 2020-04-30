
#include <benchmark/benchmark.h>

////////////////////////////////////////////////////////////////////////////////
// Algorithms as published in
//   Fliegel, H. F., and Van Flandern, T. C., "A Machine Algorithm for
//     Processing Calendar Dates," Communications of the Association of
//     Computing Machines, vol. 11 (1968), p. 657.  */

static void JulianToYMD_Fortran(int jd, int *y, int *m, int *d) {
  int L = jd + 68569;
  int N = 4 * L / 146097;
  L = L - (146097 * N + 3) / 4;
  int I = 4000 * (L + 1) / 1461001;
  L = L - 1461 * I / 4 + 31;
  int J = 80 * L / 2447;
  int K = L - 2447 * J / 80;
  L = J / 11;
  J = J + 2 - 12 * L;
  I = 100 * (N - 49) + I + L;

  *y = I;
  *m = J;
  *d = K;
}

int YMDToJulian_Fortran(int y, int m, int d) {
  return d - 32075 + 1461*(y + 4800 + (m - 14)/12)/4
        + 367*(m - 2 - (m - 14)/12*12)/12 - 3
             *((y + 4900 + (m - 14)/12)/100)/4;
}

////////////////////////////////////////////////////////////////////////////////
// Algorithms as published in upb:
//   https://github.com/protocolbuffers/upb
// Copyright (c) 2009-2011, Google Inc.
// SPDX-License-Identifier: BSD-3-Clause

/* YMDToUnix_Table(1970, 1, 1) == 1970-01-01 == 0. */
int YMDToUnix_Table(int year, int month, int day) {
  static const uint16_t month_yday[12] = {0,   31,  59,  90,  120, 151,
                                          181, 212, 243, 273, 304, 334};
  int year_base = 4800;       /* Before minimum year, divisible by 100 & 400 */
  uint32_t epoch = 2472692;   /* Days between year_base and 1970 (Unix epoch) */
  uint32_t febs_since_base = year + year_base - (month <= 2 ? 1 : 0);
  uint32_t leap_days_since_base = 1 + (febs_since_base / 4) -
                                      (febs_since_base / 100) +
                                      (febs_since_base / 400);
  uint32_t days_since_base =
      365 * (year + year_base) + month_yday[month - 1] + (day - 1)
      + leap_days_since_base;

  /* Convert from 0-epoch (0001-01-01 BC) to Unix Epoch (1970-01-01 AD).
   * Since the "BC" system does not have a year zero, 1 BC == year zero. */
  return days_since_base - epoch;
}

int YMDToUnix_Fast(int y, int m, int d) {
  unsigned year_base = 4800;  /* Before minimum year, divisible by 100 & 400 */
  unsigned epoch = 2472632;   /* Days between year_base and 1970 (Unix epoch) */
  unsigned carry = (unsigned)m - 3 > m;
  unsigned m_adj = m - 3 + (carry ? 12 : 0);   /* Month, counting from March */
  unsigned y_adj = y + year_base - carry;  /* Year, positive and March-based */
  unsigned base_days = (365 * 4 + 1) * y_adj / 4;    /* Approx days for year */
  unsigned centuries = y_adj / 100;
  unsigned extra_leap_days = (3 * centuries + 3) / 4; /* base_days correction */
  unsigned year_days = (367 * (m_adj + 1)) / 12 - 30;  /* Counting from March */
  return base_days - extra_leap_days + year_days + (d - 1) - epoch;
}

// Benchmarking code. //////////////////////////////////////////////////////////

static void BM_YMDToJulian_Fortran(benchmark::State& state) {
  for (int i = 0; state.KeepRunning();) {
    i = YMDToJulian_Fortran(i, 1, 1) & 0xffff;
    benchmark::DoNotOptimize(i);
  }
}
BENCHMARK(BM_YMDToJulian_Fortran);

static void BM_YMDToUnix_Table(benchmark::State& state) {
  for (int i = 0; state.KeepRunning();) {
    i = YMDToUnix_Table(i, 1, 1) & 0xffff;
    benchmark::DoNotOptimize(i);
  }
}
BENCHMARK(BM_YMDToUnix_Table);

static void BM_YMDToUnix_Fast(benchmark::State& state) {
  for (int i = 0; state.KeepRunning();) {
    i = YMDToUnix_Fast(i, 1, 1) & 0xffff;
    benchmark::DoNotOptimize(i);
  }
}
BENCHMARK(BM_YMDToUnix_Fast);

// main() / verification code. /////////////////////////////////////////////////

int main(int argc, char** argv) {
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
    if (YMDToJulian_Fortran(y, m, d) != jd) {
      printf("YMDToJulian_Fortran(%d, %d, %d) = %d != %d\n", y, m, d,
             YMDToJulian_Fortran(y, m, d), jd);
    }
  }

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
