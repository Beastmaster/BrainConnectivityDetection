/*=auto=========================================================================

by qinshuo ----2015.3.20
Get timecourse of fmri series

=========================================================================auto=*/

#ifndef __VTKGETTIMECOURSE_H__
#define __VTKGETTIMECOURSE_H__


#include "vtkMultipleInputsImageFilter.h"
#include "vtkFloatArray.h"
#include "vtkShortArray.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkCommand.h"
#include "vtkImageFFT.h"
#include "vtkImageRFFT.h"
#include "vtkImageIdealHighPass.h"
#include "vtkImageAppend.h"
#include "vtkImageAccumulate.h"
#include "vtkImageExtractComponents.h"
#include "vtkExtractVOI.h"
#include "vtkImageViewer.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

//standard lib
#include <iostream>
#include <vector>


template <typename PointDataType> // type of point data
class vtkGetTimeCourse
{
public:
	vtkGetTimeCourse();
	~vtkGetTimeCourse();

	//add data to data_container
	void Add_Data( vtkSmartPointer<vtkImageData> xx) {data_container.push_back(xx);};
	void Add_Data( std::vector< vtkSmartPointer < vtkImageData> > );

	//detect empty of data container
	bool is_Data_Container_Empty() { return  this->data_container.empty();};

	std::vector< PointDataType > GetTimeCourse(int*);
	std::vector< PointDataType > GetTimeCourse(std::vector<int*>);


private:
	PointDataType AccessPointData(int*,const vtkSmartPointer<vtkImageData>);
	void Clear_Overlay_Region();
protected:
	//hold all vtk image data
	std::vector< vtkSmartPointer<vtkImageData> > data_container;
	//hold region coordinate
	std::vector<int*>							 overlay_region;
};




#endif
