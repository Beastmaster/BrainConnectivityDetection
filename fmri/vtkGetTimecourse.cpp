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
void vtkGetTimeCourse<PointDataType>::GetTimeCourse(int* coor)
{
	std::vector< PointDataType > timecourse_temp;
	//iterator through data container to get time course
	for (std::vector<vtkSmartPointer<vtkImageData> >::iterator data_ite = this->data_container.begin(); 
		data_ite!=this->data_container.end(); data_ite++ )
	{
		timecourse_temp.push_back(this->AccessPointData(coor,*data_ite));
	}
	this->TimeCourse = timecourse_temp;
}



template <typename PointDataType>
void  vtkGetTimeCourse<PointDataType>:: GetRegionTimeCourse()
{
	std::vector<int*> coor_vector = overlay_region;

	//get region size , used to get mean
	int region_size = coor_vector.size();

	//temp vector to hold timecourse
	std::vector< PointDataType > timecourse_temp;

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
		timecourse_temp.push_back(total_pixel_value/region_size);
	}

	this->TimeCourse = timecourse_temp;
}

template <typename PointDataType>
void vtkGetTimeCourse<PointDataType>::SetSearchValue(PointDataType value_to_search)
{
	int imgDim[3];
	int vol=0;

	this->label_map->GetDimensions(imgDim);

	for (int kk = 0; kk < imgDim[2]; kk++)
	{
		for (int jj = 0; jj < imgDim[1]; jj++)
		{
			for (int ii = 0; ii < imgDim[0]; ii++)
			{
				float *value =(float *)label_map->GetScalarPointer(ii,jj,kk);
				if (*value == value_to_search)
				{
					int* coor_temp = new int[3];
					coor_temp[0] = ii;
					coor_temp[1] = jj;
					coor_temp[2] = kk;
					overlay_region.push_back(coor_temp);
				}
			}
		}
	}
}



//get pixel data of a vtkimagedata
template <typename PointDataType>
PointDataType vtkGetTimeCourse<PointDataType>::AccessPointData(int* coor,const vtkSmartPointer<vtkImageData> data)
{
	int dims[3];
	data->GetDimensions(dims);

	std::cout<<coor[0]<<coor[1]<<coor[2]<<std::endl;

	//get if out of region
	for (int i = 0;i<3;i++)
	{
		if (coor[i]>=dims[i])
		{
			std::cerr<<"pixel out of range"<<std::endl;
			return 0;
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



