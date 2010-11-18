#ifndef COMMON_H_
#define COMMON_H_

/// Header fajl altalanos makrokhoz, egyebekhez

/// boolean tipus definialasa
typedef int bool;
#define FALSE 0
#define TRUE (!FALSE)

/// nagyon kicsi szam, long tipusu nulla
#define EPS 0.0000000001

/// a programvaltozat esetleg verzio jelzesere, ertekadas a main.c-ben, max 8 karakter
char version[9];
#endif
