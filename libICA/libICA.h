/**
 * @file libICA.h
 * 
 * Main FastICA functions.
 */

#ifndef LIBICA_H_
#define LIBICA_H_

#include "matrix.h"
#include "svdcmp.h"

#define MAX_ITERATIONS	1000
#define TOLERANCE		0.0001
#define ALPHA			1


#ifdef __cplusplus
extern "C" {
#endif

void fastICA(double** X, int rows, int cols, int compc, double** K, double** W, double** A, double** S);

#ifdef __cplusplus
}
#endif

#endif /*LIBICA_H_*/
