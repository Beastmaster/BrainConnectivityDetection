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

double compute_Means(double** src,int rows,int cols);
double compute_Deviation(double** src,int rows,int cols);
void compute_Z_score(double** src,int rows,int cols,double** dest);
void fastICA(double** X, int rows, int cols, int compc, double** K, double** W, double** A, double** S);

#ifdef __cplusplus
}
#endif

#endif /*LIBICA_H_*/
