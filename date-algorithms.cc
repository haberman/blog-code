
#include <stdint.h>
#include <limits>

////////////////////////////////////////////////////////////////////////////////
// This is a translation of the Fortran algorithms published in
//   Fliegel, H. F., and Van Flandern, T. C., "A Machine Algorithm for
//     Processing Calendar Dates," Communications of the Association of
//     Computing Machines, vol. 11 (1968), p. 657.  */

void JulianToYMD_Fortran(int jd, int *y, int *m, int *d) {
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

int jd(int y, int m, int d) {
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
int epoch_days_table(int year, int month, int day) {
  static const uint16_t month_yday[12] = {0,   31,  59,  90,  120, 151,
                                          181, 212, 243, 273, 304, 334};
  uint32_t year_adj = year + 4800;  /* Ensure positive year, multiple of 400. */
  uint32_t febs = year_adj - (month <= 2 ? 1 : 0);  /* Februaries since base. */
  uint32_t leap_days = 1 + (febs / 4) - (febs / 100) + (febs / 400);
  uint32_t days = 365 * year_adj + leap_days + month_yday[month - 1] + day - 1;
  return days - 2472692;  /* Adjust to Unix epoch. */
}

int epoch_days_fast(int y, int m, int d) {
  const uint32_t year_base = 4800;    /* Before min year, multiple of 400. */
  const uint32_t m_adj = m - 3;       /* March-based month. */
  const uint32_t carry = m_adj > m ? 1 : 0;
  const uint32_t adjust = carry ? 12 : 0;
  const uint32_t y_adj = y + year_base - carry;
  const uint32_t month_days = ((m_adj + adjust) * 62719 + 769) / 2048;
  const uint32_t leap_days = y_adj / 4 - y_adj / 100 + y_adj / 400;
  return y_adj * 365 + leap_days + month_days + (d - 1) - 2472632;
}

////////////////////////////////////////////////////////////////////////////////
// Algorithm as published by Howard Hinnant on
//   http://howardhinnant.github.io/date_algorithms.html
// Article includes the text: "Consider these donated to the public domain."

template <class Int>
constexpr
Int
days_from_civil(Int y, unsigned m, unsigned d) noexcept
{
    static_assert(std::numeric_limits<unsigned>::digits >= 18,
             "This algorithm has not been ported to a 16 bit unsigned integer");
    static_assert(std::numeric_limits<Int>::digits >= 20,
             "This algorithm has not been ported to a 16 bit signed integer");
    y -= m <= 2;
    const Int era = (y >= 0 ? y : y-399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);      // [0, 399]
    const unsigned doy = (153*(m + (m > 2 ? -3 : 9)) + 2)/5 + d-1;  // [0, 365]
    const unsigned doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
    return era * 146097 + static_cast<Int>(doe) - 719468;
}

// Adapting the functions above to timegm() semantics. /////////////////////////

template <int F(int, int, int)>
int64_t UnixTime(int year, int month, int day, int h, int m, int s) {
  int days = F(year, month, day);
  unsigned secs = (h * 3600) + (m * 60) + s;
  return days * int64_t{86400} + secs;
}

int64_t YMDToJulian_Fortran(int year, int month, int day, int h, int m, int s) {
  return UnixTime<jd>(year, month, day, h, m, s);
}

int64_t YMDToUnix_Table(int year, int month, int day, int h, int m, int s) {
  return UnixTime<epoch_days_table>(year, month, day, h, m, s);
}

int64_t YMDToUnix_Fast(int year, int month, int day, int h, int m, int s) {
  return UnixTime<epoch_days_fast>(year, month, day, h, m, s);
}

static int days_from_civil_signed(int y, int m, int d) {
  return days_from_civil<int>(y, m, d);
}

int64_t YMDToUnix_DaysFromCivil(int year, int month, int day, int h, int m,
                                int s) {
  return UnixTime<days_from_civil_signed>(year, month, day, h, m, s);
}
