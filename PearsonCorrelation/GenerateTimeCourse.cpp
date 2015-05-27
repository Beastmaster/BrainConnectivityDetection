#include "GenerateTimeCourse.h"



GenerateTimecourse::GenerateTimecourse()
{
	threshold_up,threshold_low = 0;

	templete_img = NULL;
	label_map    = NULL;
	single_value_region = NULL;
}

GenerateTimecourse::~GenerateTimecourse()
{

}


//extract area of a specific value 
//use threshold method as a simple trick
void GenerateTimecourse::Get_label_area(float th_l,float th_u,vtkSmartPointer<vtkImageData> img)
{
	single_value_region = vtkSmartPointer<vtkImageData>::New();

	vtkSmartPointer<vtkImageThreshold> imageThreshold = 
		vtkSmartPointer<vtkImageThreshold>::New();
	imageThreshold->SetInput(img);

	//threshold between: include the edge value
	double lower = double(th_l);
	double upper = double(th_u);

	//set threshold value range
	imageThreshold->ThresholdBetween(lower,upper);
	//replace value out of threshold by 0
	imageThreshold->ReplaceOutOn();
	imageThreshold->SetOutValue(0);
	//replace value between threshold by 1
	imageThreshold->ReplaceInOn();
	imageThreshold->SetInValue(1);

	imageThreshold->Update();
	single_value_region->DeepCopy(imageThreshold->GetOutput());

	//#include "MainWindow.h"
	//vtkSmartPointer<vtkImageCast> caster = 
	//	vtkSmartPointer<vtkImageCast>::New();
	//caster->SetInput(imageThreshold->GetOutput());
	//caster->SetOutputScalarTypeToFloat();
	//caster->Update();

	//v2iConnectorType::Pointer connector = v2iConnectorType::New();
	//connector->SetInput(caster->GetOutput());
	//try
	//{
	//	connector->Update();
	//}
	//catch (itk::ExceptionObject& e)
	//{
	//	std::cout<<"connect itk vtk error"<<std::endl;
	//	std::cerr<<e;
	//}
	//WriterType_b::Pointer nii_writer_parse = 
	//	WriterType_b::New();
	//nii_writer_parse->SetInput(connector->GetOutput());
	//std::string name1 = "masktest.nii";
	//nii_writer_parse->SetFileName(name1);
	//try
	//{
	//	nii_writer_parse->Update();
	//}
	//catch (itk::ExceptionObject& e)
	//{
	//	std::cout<<"connect itk vtk error"<<std::endl;
	//	std::cerr<<e;
	//}
}

//parameter: image to mask
double GenerateTimecourse::Mask_Region_Mean(vtkSmartPointer<vtkImageData> img)
{
	if (single_value_region == NULL)
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
	maskFilter->SetMaskInput(single_value_region);
	maskFilter->Update();
	
	//#include "MainWindow.h"
	//v2iConnectorType::Pointer connector = v2iConnectorType::New();
	//connector->SetInput(maskFilter->GetOutput());
	//try
	//{
	//	connector->Update();
	//}
	//catch (itk::ExceptionObject& e)
	//{
	//	std::cout<<"connect itk vtk error"<<std::endl;
	//	std::cerr<<e;
	//}
	//WriterType_b::Pointer nii_writer_parse = 
	//	WriterType_b::New();
	//nii_writer_parse->SetInput(connector->GetOutput());
	//std::string name1 = "masktest.nii";
	//nii_writer_parse->SetFileName(name1);
	//nii_writer_parse->Update();
	//nii_writer_parse->Write();
	
	//compute image size
	int dims[3];
	img->GetDimensions(dims);
	int img_size = dims[0]*dims[1]*dims[2];

	//use sef-defined method to compute means
	int region_size = 0;
	float* val;
	double region_accumulate = 0.0;
	for (int i=0;i<dims[0];i++)
	{
		for (int j=0;j<dims[1];j++)
		{
			for (int k=0;k<dims[2];k++)
			{
				val = (float *)maskFilter->GetOutput()->GetScalarPointer(i, j, k);
				if (*val != 0.0)
				{
					region_size++;
				}
				region_accumulate += *val;
			}
		}
	}
	return region_accumulate/region_size;

	////compute means: use vtk image accumulate method
	//vtkSmartPointer<vtkImageAccumulate> accumulate_Filter = 
	//	vtkSmartPointer<vtkImageAccumulate>::New();
	//accumulate_Filter->SetComponentExtent(0, 0, 0, 0, 0, 0);
	//accumulate_Filter->SetComponentOrigin(0.0, 0.0, 0.0);
	//accumulate_Filter->SetComponentSpacing(1.0, 1.0, 1.0);
	//accumulate_Filter->SetInput(maskFilter->GetOutput());
	//accumulate_Filter->Update();


	//use means[0] to get value!!!!
	//double *means = accumulate_Filter->GetMean();
	//return accumulate_Filter->GetMean()[0] * img_size;   

	////use vtkimage histogramstaticstics to compute means
	//vtkSmartPointer<vtkImageHistogramStatistics> hist = 
	//	vtkSmartPointer<vtkImageHistogramStatistics>::New();
	//hist->SetInput(maskFilter->GetOutput());
	//hist->Update();
	//return hist->GetMean();
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
	
	this->time_course.clear();
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

void GenerateTimecourse::SetLabelMap(vtkSmartPointer <vtkImageData> img)
{
	if (img == NULL)
	{
		return;
	}

	vtkSmartPointer<vtkImageCast> caster = 
		vtkSmartPointer<vtkImageCast>::New();
	caster->SetInput(img);
	caster->SetOutputScalarTypeToUnsignedChar();
	caster->Update();

	this->label_map = vtkSmartPointer<vtkImageData>::New();
	this->label_map->DeepCopy(caster->GetOutput());
}


std::vector<float > GenerateTimecourse::GetPointTimecourse(int i,int j,int k)
{
	std::vector<float> return_vector;

	if (data_container.size()==0)
	{
		std::cout<<"there is no data in container"<<std::endl;
		return return_vector;
	}

	// image size
	int dims[3];
	data_container[0]->GetDimensions(dims);
	if ((i<=dims[0])&&(j<=dims[1])&&(k<=dims[2])&&(i>=0)&&(j>=0)&&(k>=0));
	else
	{
		return return_vector;
	}
	int img_size = dims[0]*dims[1]*dims[2];

	//use sef-defined method to compute means
	int region_size = 0;
	float* val;
	for (int ii=0;ii<data_container.size();ii++)
	{
		val = (float *)data_container[ii]->GetScalarPointer(i, j, k);
		return_vector.push_back(*val);
	}
	
	return return_vector; 
}