/*
 * neighbour.h
 *
 *  Created on: 2010.02.16.
 *      Author: martinj
 */

#ifndef NEIGHBOUR_H_
#define NEIGHBOUR_H_

#include "tetranet.h"

void neighbours_update( tTetranet tn );
void neighbours_insert( tTetranet tn, tTetraRef tr );
void neighbours_delete( tTetranet tn, tTetraRef tr );

#endif /* NEIGHBOUR_H_ */
