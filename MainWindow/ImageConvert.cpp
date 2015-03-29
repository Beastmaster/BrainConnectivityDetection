#include "ImageConvert.h"



Image_Convert_Base::Image_Convert_Base()
{

}
Image_Convert_Base::~Image_Convert_Base()
{

}

void Image_Convert_Base::SetFileName(std::string in)
{
	this->file_name = in;
}

vtkSmartPointer<vtkImageData> Image_Convert_Base::GetOutput()
{
	this->Get_Name_Suffix();

	if (file_suffix == ".nii")
	{
		//nifti io
		//typedef itk::NiftiImageIO NiftiIOType;
		NiftiIOType::Pointer niftiIO = 
			NiftiIOType::New();	
		//itk read nii file
		//typedef itk::Image<float, 3> ImageType;
		//typedef itk::ImageSeriesReader< ImageType >  ReaderType;
		ReaderType_b::Pointer reader = ReaderType_b::New();
		reader->SetImageIO(niftiIO);
		reader->SetFileName(this->file_name);
		reader->Update();

		//itk-vtk connector
		//typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
		i2vConnectorType::Pointer connector = i2vConnectorType::New();
		connector->SetInput(reader->GetOutput());
		connector->Update();

		vtkSmartPointer<vtkImageCast> img_caster = 
			vtkSmartPointer<vtkImageCast>::New();
		img_caster->SetInput(connector->GetOutput());
		img_caster->SetOutputScalarTypeToFloat();
		img_caster->Update();

		vtkSmartPointer<vtkImageData> return_img = 
			vtkSmartPointer<vtkImageData>::New();
		return_img->DeepCopy(img_caster->GetOutput());
		return return_img;
	}
	else if (file_suffix == ".mha")
	{
		vtkSmartPointer<vtkMetaImageReader> img_reader = 
			vtkSmartPointer<vtkMetaImageReader>::New();
		img_reader->SetFileName(file_name.data());
		img_reader->Update();

		vtkSmartPointer<vtkImageCast> img_caster = 
			vtkSmartPointer<vtkImageCast>::New();
		img_caster->SetInput(img_reader->GetOutput());
		img_caster->SetOutputScalarTypeToFloat();
		img_caster->Update();

		vtkSmartPointer<vtkImageData> return_img = 
			vtkSmartPointer<vtkImageData>::New();
		return_img->DeepCopy(img_caster->GetOutput());

		return return_img;
	}

}
void Image_Convert_Base::Get_Name_Suffix()
{
	if (this->file_name.empty())
	{
		return;
	}
	else
	{
		std::size_t pos = file_name.find(".");
		this->file_suffix = file_name.substr(pos);
	}
}