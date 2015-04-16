#include "GenerateTimeCourse.h"



GenerateTimecourse::GenerateTimecourse()
{
	threshold_up,threshold_low = 0;

	templete_img = NULL;
	label_map    = NULL;
	signal_value_region = NULL;
}

GenerateTimecourse::~GenerateTimecourse()
{

}


//extract area of a specific value 
//use threshold method as a simple trick
void GenerateTimecourse::Get_label_area(float th_l,float th_u,vtkSmartPointer<vtkImageData> img)
{
	signal_value_region = vtkSmartPointer<vtkImageData>::New();

	vtkSmartPointer<vtkImageThreshold> imageThreshold = 
		vtkSmartPointer<vtkImageThreshold>::New();
	imageThreshold->SetInput(img);

	//threshold between: include the edge value
	float lower = th_l;
	float upper = th_u;

	//set threshold value range
	imageThreshold->ThresholdBetween(lower,upper);
	//replace value out of threshold by 0
	imageThreshold->ReplaceOutOn();
	imageThreshold->SetOutValue(0);
	//replace value between threshold by 1
	imageThreshold->ReplaceInOn();
	imageThreshold->SetInValue(1);

	imageThreshold->Update();
	signal_value_region->DeepCopy(imageThreshold->GetOutput());
}

//parameter: image to mask
double GenerateTimecourse::Mask_Region_Mean(vtkSmartPointer<vtkImageData> img)
{
	if (signal_value_region == NULL)
	{
		return 0.0;
	}
	
	//mask 
	vtkSmartPointer<vtkImageMask> maskFilter = 
		vtkSmartPointer<vtkImageMask>::New();
	//set mask properties
	maskFilter->SetMaskedOutputValue(0.0);
	//set mask data
	maskFilter->SetImageInput(img);
	maskFilter->SetMaskInput(signal_value_region);
	maskFilter->Update();

	//compute means
	vtkSmartPointer<vtkImageAccumulate> accumulate_Filter = 
		vtkSmartPointer<vtkImageAccumulate>::New();
	accumulate_Filter->SetComponentExtent(0, 0, 0, 0, 0, 0);
	accumulate_Filter->SetComponentOrigin(0.0, 0.0, 0.0);
	accumulate_Filter->SetComponentSpacing(1.0, 1.0, 1.0);
	accumulate_Filter->SetInput(maskFilter->GetOutput());
	accumulate_Filter->Update();

	//use means[0] to get value!!!!
	return accumulate_Filter->GetMean()[0];   
}

std::vector<double> GenerateTimecourse::GetTimecourse()
{
	int img_cnt = this->data_container.size();
	if (img_cnt == 0)
	{
		return this->time_course;
	}

	//get a single area
	this->Get_label_area(this->threshold_low,this->threshold_up,this->label_map);
	
	//compute means
	for(int i=0;i<img_cnt;i++)
	{
		this->time_course.push_back(Mask_Region_Mean(data_container[i]));
	}

	return this->time_course;
}

void GenerateTimecourse::AddInputData(vtkSmartPointer<vtkImageData> img)
{
	this->data_container.push_back(img);
}
void GenerateTimecourse::SetInputData(std::vector<vtkSmartPointer <vtkImageData> >img_v)
{
	this->data_container = img_v;
}
