/** 
 * @file svdcmp.h
 * 
 * Singular value decomposition.
 */

#ifndef SVDCMP_H_
#define SVDCMP_H_


#ifdef __cplusplus

extern "C" {
#endif
	void svdcmp(mat A, unsigned int M, unsigned int N, vect W, mat V);
#ifdef __cplusplus
}
#endif

#endif /*SVDCMP_H_*/
