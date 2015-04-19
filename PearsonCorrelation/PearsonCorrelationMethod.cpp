#include "PearsonCorrelationMethod.h"




pearson_method::pearson_method()
{

}
pearson_method::~pearson_method()
{

}

void pearson_method::Add_Row(std::vector<double> input)
{
	temp_vector_holder.push_back(input);
}

void pearson_method::Add_Row_name(std::string name)
{
	temp_name_holder.push_back(name);
}

col_type2 pearson_method::Calculate_Correlation()
{
	if (components.empty())
	{
		return correlation_matrix;
	}
	if (components.back().first.empty())
	{
		components.pop_back();
	}
	if (components.back().second.empty())
	{
		components.pop_back();
	}

	for (int i=0;i<components.size();i++)
	{
		row_type2 row_temp;
		row_temp.first = components[i].first;
		for (int j=0;j<components.size();j++)
		{
			double relat_t = pearson(components[i].second,components[j].second);
			row_temp.second.push_back(relat_t);
		}
		correlation_matrix.push_back(row_temp);
	}

	return correlation_matrix;
}


void pearson_method::Get_Range(double& low,double& high,col_type2 correlation_matrix)
{
	low  = 0.0;
	high = 0.0;

	if (correlation_matrix.empty())
	{
		return;
	}
	
	//assign a value first
	low = correlation_matrix[0].second[0];
	high = low;

	for (int i=0;i<correlation_matrix.size();i++)
	{
		for (int j=0;j<correlation_matrix[i].second.size();j++)
		{
			if (low>correlation_matrix[i].second[j])
			{
				low = correlation_matrix[i].second[j];
			}
			if (high<correlation_matrix[i].second[j])
			{
				high = correlation_matrix[i].second[j];
			}
		}
	}

}


//clear to 0 between th_low and th_high
col_type2 pearson_method::Thredhold(double th_low,double th_high,col_type2 correlation_matrix)
{
	//find empty first
	if (correlation_matrix.empty())
	{
		return correlation_matrix;
	}

	for (int i=0;i<correlation_matrix.size();i++)
	{
		for (int j=0;j<correlation_matrix[i].second.size();j++)
		{
			if (correlation_matrix[i].second[j]<th_high)
			{
				if (correlation_matrix[i].second[j]>th_low)
				{
					correlation_matrix[i].second[j] = 0;
				}
			}
		}
	}

	return correlation_matrix;
}

void pearson_method::Make_Matrix()
{
	if (this->temp_name_holder.size() != this->temp_vector_holder.size())
	{
		std::cout<<"name size and col size mis-match"<<std::endl;
		return;
	}

	for(int i = 0;i<this->temp_name_holder.size();i++)
	{
		row_type2 temp_pair = 
		std::make_pair(temp_name_holder[i],temp_vector_holder[i]);

		this->components.push_back(temp_pair);
	}

	temp_name_holder.clear();
	temp_vector_holder.clear();
}


