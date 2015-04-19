/***************
Date: 2015.4.8
Author: QinShuo
Description: Generate timecourse of a image series
***************/


#ifndef _GENERATE_TIME_COURSE_H_
#define _GENERATE_TIME_COURSE_H_

#include <vector>


#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkCommand.h"
#include "vtkImageIdealHighPass.h"
#include "vtkImageAppend.h"
#include "vtkImageAccumulate.h"
#include "vtkImageExtractComponents.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkImageThreshold.h"
#include "vtkImageMask.h"
#include "vtkImageAccumulate.h"
#include "vtkImageMathematics.h"
#include "vtkImageCast.h"
//#include "vtkImageHistogramStatistics.h"

class GenerateTimecourse
{
public:
	GenerateTimecourse();
	~GenerateTimecourse();

	//parameters
	float threshold_up,threshold_low;


	//functions
	void SetLabelValue(float in) {threshold_low = in;threshold_up = in;};
	void SetLabelMap(vtkSmartPointer <vtkImageData> img);
	//this two function is alternative
	void AddInputData(vtkSmartPointer<vtkImageData>);
	void SetInputData(std::vector<vtkSmartPointer <vtkImageData> >);
	//no use here
	void SetTemplete(vtkSmartPointer <vtkImageData> img){this->templete_img = img;};
	
	//output
	std::vector<double> GetTimecourse();
	std::vector<float > GetPointTimecourse(int,int,int);

protected:
	std::vector<vtkSmartPointer <vtkImageData> > data_container;
	//structure image of brain
	vtkSmartPointer<vtkImageData>                templete_img;
	//label map of brain areas
	vtkSmartPointer<vtkImageData>                label_map;
	vtkSmartPointer<vtkImageData>				 single_value_region;
	std::vector< double >						 time_course;

	void Get_label_area(float th_l,float th_u,vtkSmartPointer<vtkImageData>);
	double Mask_Region_Mean(vtkSmartPointer< vtkImageData >);
};


#endif
 