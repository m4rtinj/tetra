/*
 *  nasreader.h
 *
 *  Created on: 06.10.2010
 *      Author: Martin József
 */

#ifndef NASREADER_H_
#define NASREADER_H_

#include "tetranet.h"
#include "common.h"
#include <stdio.h>

unsigned long nasreader_getPointNr( FILE *f );
unsigned long nasreader_getTetraNr( FILE *f );
void nasreader_readFirstPoint( FILE *f, tPoint *p );
bool nasreader_readNextPoint( FILE *f, tPoint *p );
void nasreader_readFirstTetra( FILE *f, tPointRef *p );
bool nasreader_readNextTetra( FILE *f, tPointRef *p );

#endif /* NASREADER_H_ */
