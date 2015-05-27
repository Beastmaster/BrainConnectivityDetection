//by qinshuo    2015.3.20
//
//function:
//
//run Pearson Correlation for timecourse
//use STL mainly



#ifndef __PEARSONCORRELATION_H_
#define __PEARSONCORRELATION_H_

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>

using namespace std;
/**
*	compares two vector lengths
*   if not equal = 1
*/
template<typename T>
int comp(vector<T> v1, vector<T> v2){
	if(v1.size() != v2.size()){
		cout << "vectors not the same size\n";
		return 1;
	}
	else
		return 0;
}

/**
*	Euclidean distance between two vectors of type T such that T has binary +,-,* 
*/
template<typename T>
double euclidean(vector<T> v1, vector<T> v2){
	if (comp(v1, v2))
	{
		std::cerr<<"error: length not equal"<<std::endl;
	}
	T diff, sum;

	diff = v1[0] - v2[0];
	sum = diff * diff;

	for (unsigned int i=1; i < v1.size(); i++){
		diff = v1[i] - v2[i];
		sum += diff * diff;
	}
	return sqrt(double(sum));
}

/**
*	Jaccard Coefficient.	Use for asymetric binary values
*/
template<typename T>
double jaccard(vector<T> v1, vector<T> v2){
	if (comp(v1, v2))
	{
		std::cerr<<"error: length not equal"<<std::endl;
	}
	int f11 = 0, f00 = 0;

	for (unsigned int i=0; i < v1.size(); i++){
		if(v1[i] == v2[i]){
			if(v1[i])
				f11++;
			else
				f00++;
		}
	}
	return double(f11) / double(v1.size() - (f11+f00));
}

/**
*	Cosine
*/
template<typename T>
double cosine(vector<T> v1, vector<T> v2){
	if (comp(v1, v2))
	{
		std::cerr<<"error: length not equal"<<std::endl;
	}

	T lv1 = v1[0] * v1[0];
	T lv2 = v2[0] * v2[0];
	T dot12 = v1[0] * v2[0];

	for (unsigned int i=0; i < v1.size(); i++){
		lv1 += v1[i] * v1[i];
		lv2 += v2[i] * v2[i];
		dot12 += v1[i] * v2[i];
	}
	return double(dot12) / ( sqrt(double(lv1)) * sqrt(double(lv2)) );
}

/**
*	The mean of a vector
*/
template<typename T>
double mean(vector<T> v1){
	T sum = v1[0];
	for (unsigned int i=1; i < v1.size(); i++)
		sum += v1[i];
	return double(sum) / double(v1.size());
}

/**
*	The Covariance
*/
template<typename T>
double covariance(vector<T> v1, vector<T> v2){
	if (comp(v1, v2))
	{
		std::cerr<<"error: length not equal"<<std::endl;
	}
	double mean1 = mean(v1), mean2 = mean(v2);
	double sum = (double(v1[0]) - mean1) * (double(v2[0]) - mean2);
	for (unsigned int i=1; i < v1.size(); i++){
		sum += (double(v1[i]) - mean1) * (double(v2[i]) - mean2);
	}
	return double(sum) / double(v1.size()-1);
}

/**
*	standard deviation the covariance where both vectors are the same.
*/
template<typename T>
double std_dev(vector<T> v1){
	return sqrt(covariance(v1, v1));
}

/**
*	Pearson Correlation
*/
template<typename T>
double pearson(vector<T> v1, vector<T> v2){
	if (std_dev(v1) * std_dev(v2) == 0){
		cout << "( a standard deviaton was 0 )";
		return 0; // I dont know what to do here???
	}
	return covariance(v1,v2) / ( std_dev(v1) * std_dev(v2));
}





#endif


