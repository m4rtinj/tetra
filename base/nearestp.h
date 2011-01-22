/*
 *  nearestp.h
 *  
 *  Kd-tree es nearest neighbour search
 *  2010-2011 - Martin Jozsef
 */

#ifndef NEARESTP_H_
#define NEARESTP_H_

#include "tetranet.h"

/**
 *  A keresest segito kd-tree elokeszitese
 */
void nearestp_update( tTetranet tn );

/**
 *  A legkozelebbi pont megkeresese
 *  @param p a kerdeses uj pont (koordinatakkal megadva)
 *  @return a ponthalmaz legkozelebbi pontjanak indexe
 */
tPointRef nearestp_search( tTetranet tn, tPoint p );

/**
 *  Uj pont beszurasa a keresofaba
 *  @param p a beszurando pont indexe
 */
void nearestp_addPoint( tTetranet tn, tPointRef p );

/**
 *  A kd-tree altalt hasznalt memoria felszabaditasa
 */
void nearestp_free( tTetranet tn );

#endif /* NEARESTP_H_ */
