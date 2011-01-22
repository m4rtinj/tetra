/*
 *  vector.h
 *  
 *  Vektorok, vektormuveletek
 *  2010-2011 - Martin Jozsef
 */

#ifndef VECTOR_H_
#define VECTOR_H_

/// vektor, harom dimenzios koordinatakkal
typedef struct {
    double x;
    double y;
    double z;
} vector;

/// vektor iranyanak megforditasa
vector negativeVector( vector a );

/// ket vektor osszege
vector vector_add( vector a, vector b );

/// a - b vektor, azaz b-bol a-ba mutato vektor
vector vector_diff( vector a, vector b );

/// vektor konstansszorosa
vector vector_constMult( vector a, double c );

/// vektor hossza
double vector_length( vector a );

/// skalar szorzat
double dotProduct( vector a, vector b );

/// vektorialis szorzat
vector crossProduct( vector a, vector b );

/// vegyes szorzat
double tripleProduct( vector a, vector b, vector c );

/// harom ponttal megadott sik normalvektora
vector normalOfPlane( vector a, vector b, vector c );

/// tetraeder tomegkozeppontja
vector massPoint( vector a, vector b, vector c, vector d );

#endif
