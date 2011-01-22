/*
 *  neighbour.h
 *  
 *  Oldalszomszedos tetraederek keresese
 *  2010-2011 - Martin Jozsef
 */

#ifndef NEIGHBOUR_H_
#define NEIGHBOUR_H_

#include "tetranet.h"

void neighbours_update( tTetranet tn );
void neighbours_insert( tTetranet tn, tTetraRef tr );
void neighbours_delete( tTetranet tn, tTetraRef tr );

#endif /* NEIGHBOUR_H_ */
