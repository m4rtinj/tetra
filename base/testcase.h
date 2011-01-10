#ifndef TESTCASE_H_
#define TESTCASE_H_

#include "tetranet.h"

/**
Elinditja a stoppert. Egy teszt kezdeten kell meghivni.
Egyelore csak egy stopper letezik, nem hasznalhato tobb parhuzamosan.
*/
void startClock();

/**
Megallitja a stoppert es kiirja a mert idot, hasznalt memoriat, egyebet.
@param name a mert teszteset neve
*/
void stopClock( char *name );

void test_explode( tTetranet tn );

void test_alfa( tTetranet tn );

void test_nearestp( tTetranet tn );

void test_pointLocation( tTetranet tn );

void test_massPointLocation( tTetranet tn );

void test_delete( tTetranet tn );

void test_flow( tTetranet tn );

#endif
