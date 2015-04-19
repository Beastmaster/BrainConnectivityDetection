#ifndef  __PearsonCorrelationMethon_H__
#define  __PearsonCorrelationMethon_H__

#include "PearsonCorrelation_algorithm.hpp"
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string>

//float type
typedef std::pair<std::string, std::vector<float> > row_type;
typedef std::vector<row_type> col_type;
//double type
typedef std::pair<std::string, std::vector<double> > row_type2;
typedef std::vector<row_type2> col_type2;

class pearson_method
{
public: 
	pearson_method();
	~pearson_method();

	void Add_Row(std::vector<double>);
	void Add_Row_name(std::string);
	void Make_Matrix();
	col_type2 Calculate_Correlation();
	static void Get_Range(double&,double&,col_type2);
	static col_type2 Thredhold(double,double,col_type2);

	//hold pearson correlation matrix
	col_type2 correlation_matrix;

private:
	std::vector< std::vector<double> > temp_vector_holder;
	std::vector< std::string > temp_name_holder;

	//hold input
	col_type2 components;
};








#endif