#ifndef  __PearsonCorrelationMethon_H__
#define  __PearsonCorrelationMethon_H__

#include "PearsonCorrelation_algorithm.hpp"
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string>

typedef std::pair<std::string, std::vector<float> > row_type;
typedef std::pair<std::string, std::vector<double> > row_type2;
typedef std::vector<row_type> col_type;
typedef std::vector<row_type2> col_type2;

class pearson_method
{
public: 
	pearson_method();
	~pearson_method();

	void Add_Row(std::vector<float>);
	void Add_Row_name(std::string);
	void Calculate_Correlation();
	void Get_Range(double,double);
	void Thredhold(double,double);

	//hold pearson correlation matrix
	col_type2 correlation_matrix;

private:
	//hold input
	col_type components;
};








#endif