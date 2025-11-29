#ifndef LIMITS_H
#define LIMITS_H

#define CHAR_BIT        8
#define SCHAR_MIN       (-128)
#define SCHAR_MAX       127
#define UCHAR_MAX       255

#ifdef __CHAR_UNSIGNED__
#define CHAR_MIN        0
#define CHAR_MAX        UCHAR_MAX
#else
#define CHAR_MIN        SCHAR_MIN
#define CHAR_MAX        SCHAR_MAX
#endif

#define SHORT_MIN       (-32768)
#define SHORT_MAX       32767
#define USHORT_MAX      65535

#define INT_MIN         (-2147483647 - 1)
#define INT_MAX         2147483647
#define UINT_MAX        0xFFFFFFFFU

#define LONG_MIN        (-9223372036854775807L - 1)
#define LONG_MAX        9223372036854775807L
#define ULONG_MAX       0xFFFFFFFFFFFFFFFFUL

#define LLONG_MIN       (-9223372036854775807LL - 1)
#define LLONG_MAX       9223372036854775807LL
#define ULLONG_MAX      0xFFFFFFFFFFFFFFFFULL

#endif
