#include "PearsonCorrelationMethod.h"




pearson_method::pearson_method()
{}
pearson_method::~pearson_method()
{}

void pearson_method::Add_Row(std::vector<float> input)
{
	if (components.back().first.empty())
	{
		row_type pair_temp;
		pair_temp.second = input;

		components.push_back(pair_temp);
	}
	else
	{
		components.back().second = input;
	}

}

void pearson_method::Add_Row_name(std::string name)
{
	if (components.back().first.empty())
	{
		components.back().first=name;
	}
	else
	{
		row_type pair_temp;
		pair_temp.first = name;
		components.push_back(pair_temp);
	}
}

void pearson_method::Calculate_Correlation()
{
	if (components.empty())
	{
		return;
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
	}

}


void pearson_method::Get_Range(double low,double high)
{
	low  = 0.0;
	high = 0.0;

	if (this->correlation_matrix.empty())
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
void pearson_method::Thredhold(double th_low,double th_high)
{
	//find empty first
	if (this->correlation_matrix.empty())
	{
		return;
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
}


