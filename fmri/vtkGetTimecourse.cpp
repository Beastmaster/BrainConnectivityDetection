/*=auto=========================================================================

by qinshuo ----2014.12.2
for multiple input

=========================================================================auto=*/
#include "vtkGetTimecourse.h"

template <typename PointDataType>
vtkGetTimeCourse<PointDataType>::vtkGetTimeCourse()
{

}

template <typename PointDataType>
vtkGetTimeCourse<PointDataType>::~vtkGetTimeCourse()
{
	//clean data_container
	this->data_container.clear();
	//clean region condatiner
	this->Clear_Overlay_Region();
}

template <typename PointDataType>
void vtkGetTimeCourse<PointDataType>::Add_Data(std::vector< vtkSmartPointer < vtkImageData> > data_vector_add)
{
	this->data_container = data_vector_add;
}


template <typename PointDataType>
std::vector< PointDataType > vtkGetTimeCourse<PointDataType>::GetTimeCourse(int* coor)
{
	std::vector< PointDataType > timecourse_temp;
	//iterator through data container to get time course
	for (std::vector<int*>::iterator data_ite = this->data_container.begin(); 
		data_ite!=this->data_container.end(); data_ite++ )
	{
		timecourse_temp.push_back(this->AccessPointData(coor));
	}
	return timecourse_temp;
}



template <typename PointDataType>
std::vector< PointDataType > vtkGetTimeCourse<PointDataType>:: GetTimeCourse(std::vector<int*> coor_vector)
{
	//get region size , used to get mean
	int region_size = coor_vector.size();

	//temp vector to hold timecourse
	std::vector< int > timecourse_temp;

	for (std::vector< vtkSmartPointer<vtkImageData> >::iterator data_tem = data_container.begin();
		data_tem!=data_container.end();data_tem++)
	{
		int total_pixel_value = 0;

		for (std::vector<int*>::iterator coor_tem=coor_vector.begin();
			coor_tem!=coor_vector.end();coor_tem++)
		{
			total_pixel_value += 
				this->AccessPointData((*coor_tem),*data_tem);
		}
		timecourse_temp.push_back(total_pixel_value);
	}

	return timecourse_temp;
}

//get pixel data of a vtkimagedata
template <typename PointDataType>
PointDataType vtkGetTimeCourse<PointDataType>::AccessPointData(int* coor,const vtkSmartPointer<vtkImageData> data)
{
	int dims[3];
	data->GetDimensions(dims);

	//get if out of region
	for (int i = 0;i<3;i++)
	{
		if (coor[i]>dims[i])
		{
			std::cerr<<"pixel out of range"<<std::endl;
			return;
		}
	}

	//get pixel data
	return *((PointDataType*) data->GetScalarPointer(coor));
}


template <typename PointDataType>
void vtkGetTimeCourse<PointDataType>::Clear_Overlay_Region()
{
	//if overlay region is not empty, then clean it
	if (this->overlay_region.empty())
	{
		return;
	}
	else
	{
		for (std::vector<int*>::iterator it = this->overlay_region.begin();
			it!=this->overlay_region.end();it++)
		{
			if ((*it)!=NULL)
			{
				delete (*it);
			}
		}
		this->overlay_region.clear();
	}
}



