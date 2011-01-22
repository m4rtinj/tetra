/*
 *  vector.c
 *  
 *  Vektorok, vektormuveletek
 *  2010-2011 - Martin Jozsef
 */

#include "vector.h"
#include <math.h>

vector vector_add( vector a, vector b ) {
    vector v;
    v.x = a.x + b.x;
    v.y = a.y + b.y;
    v.z = a.z + b.z;
    return v;
}

vector vector_diff( vector a, vector b ) {
    vector v;
    v.x = a.x - b.x;
    v.y = a.y - b.y;
    v.z = a.z - b.z;
    return v;
}

vector vector_constMult( vector a, double c ) {
    vector v;
    v.x = c * a.x;
    v.y = c * a.y;
    v.z = c * a.z;
    return v;
}

double vector_length( vector a ) {
    return sqrt( a.x * a.x + a.y * a.y + a.z * a.z );
}

vector negativeVector( vector a ) {
    a.x = -a.x;
    a.y = -a.y;
    a.z = -a.z;
    return a;
}

double dotProduct( vector a, vector b ) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vector crossProduct( vector a, vector b ) {
    vector n;
    n.x = a.y * b.z - a.z * b.y;
    n.y = a.z * b.x - a.x * b.z;
    n.z = a.x * b.y - a.y * b.x;
    return n;
}

double tripleProduct( vector a, vector b, vector c ) {
    return dotProduct( a, crossProduct( b, c ) );
}

vector normalOfPlane( vector a, vector b, vector c ) {
    return crossProduct( vector_diff( a, b ), vector_diff( a, c ) );
}

vector massPoint( vector a, vector b, vector c, vector d ) {
    vector v;
    v.x = ( a.x + b.x + c.x + d.x ) / 4;
    v.y = ( a.y + b.y + c.y + d.y ) / 4;
    v.z = ( a.z + b.z + c.z + d.z ) / 4;
    return v;
}
