#include "ImageConvert.h"
#include "itkOrientImageFilter.h"


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

	if (file_suffix == ".nii"||(file_suffix == ".hdr"))
	{
		//nifti io
		//typedef itk::NiftiImageIO NiftiIOType;
		NiftiIOType::Pointer niftiIO = 
			NiftiIOType::New();	
		//itk read nii file
		//typedef itk::Image<float, 3> ImageType;
		//typedef itk::ImageSeriesReader< ImageType >  ReaderType;
		ReaderType_Convert::Pointer reader = ReaderType_Convert::New();
		reader->SetImageIO(niftiIO);
		reader->SetFileName(this->file_name);
		try
		{
			reader->Update();
		}
		catch (itk::ExceptionObject& e)
		{
			std::cout<<"read nii error"<<std::endl;
			std::cerr<<e;
		}

		//-------add --------//
		//directed by zhangteng
		itk::OrientImageFilter<ImageTypex,ImageTypex>::Pointer orientor = 
			itk::OrientImageFilter<ImageTypex,ImageTypex>::New();
		orientor->UseImageDirectionOn();
		orientor->SetDesiredCoordinateOrientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);
		orientor->SetInput(reader->GetOutput());
		try
		{
			orientor->Update();
		}
		catch (itk::ExceptionObject& e)
		{
			std::cout<<"orientai ror"<<std::endl;
			std::cerr<<e;
		}
		 
		          

		//itk-vtk connector
		//typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
		i2vConnectorType::Pointer connector = i2vConnectorType::New();
		connector->SetInput(orientor->GetOutput());//(reader->GetOutput());
		try
		{
		connector->Update();
		}
		catch (itk::ExceptionObject& e)
		{
			std::cout<<"connect itk vtk error"<<std::endl;
			std::cerr<<e;
		}

		vtkSmartPointer<vtkImageCast> img_caster = 
			vtkSmartPointer<vtkImageCast>::New();
#if VTK_MAJOR_VERSION <= 5
		img_caster->SetInput(connector->GetOutput());
#else
		img_caster->SetInputData(connector->GetOutput());
#endif
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
		try
		{
			img_reader->Update();
		}
		catch(...)
		{
			return NULL;
		}
		

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
	else if (file_suffix == ".dcm")
	{
		vtkSmartPointer<vtkDICOMImageReader> reader =
			vtkSmartPointer<vtkDICOMImageReader>::New();
		reader->SetFileName(file_name.data());
		reader->Update();

		vtkSmartPointer<vtkImageCast> img_caster = 
			vtkSmartPointer<vtkImageCast>::New();
#if VTK_MAJOR_VERSION <= 5
		img_caster->SetInput(reader->GetOutput());
#else
		img_caster->SetInputData(reader->GetOutput());
#endif
		img_caster->SetOutputScalarTypeToFloat();
		img_caster->Update();

		vtkSmartPointer<vtkImageData> return_img = 
			vtkSmartPointer<vtkImageData>::New();
		return_img->DeepCopy(img_caster->GetOutput());

		return return_img;
	}

	else
	{
		return NULL;
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
		std::size_t pos = file_name.find_last_of(".");
		this->file_suffix = file_name.substr(pos);
	}
}

ImageTypex::Pointer Image_Convert_Base::GetITKPointer(std::string file_name)
{
	std::string file_suffix;
	//get file name suffix
	if (file_name.empty())
	{
		std::cout<<"input a file name first!"<<std::endl;
		return NULL;
	}
	else
	{
		std::size_t pos = file_name.find_last_of(".");
		file_suffix = file_name.substr(pos);
	}

	if (file_suffix == ".nii")
	{
		//nifti io
		//typedef itk::NiftiImageIO NiftiIOType;
		NiftiIOType::Pointer niftiIO = 
			NiftiIOType::New();	
		//itk read nii file
		//typedef itk::Image<float, 3> ImageType;
		//typedef itk::ImageSeriesReader< ImageType >  ReaderType;
		ReaderType_Convert::Pointer reader = ReaderType_Convert::New();
		reader->SetImageIO(niftiIO);
		reader->SetFileName(file_name);
		try
		{
			reader->Update();
		}
		catch (itk::ExceptionObject& e)
		{
			std::cout<<"read nii error"<<std::endl;
			std::cerr<<e;
		}

		//-------add --------//
		itk::OrientImageFilter<ImageTypex,ImageTypex>::Pointer orientor = 
			itk::OrientImageFilter<ImageTypex,ImageTypex>::New();
		orientor->UseImageDirectionOn();
		orientor->SetDesiredCoordinateOrientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);
		orientor->SetInput(reader->GetOutput());
		try
		{
			orientor->Update();
		}
		catch (itk::ExceptionObject& e)
		{
			std::cout<<"orientai ror"<<std::endl;
			std::cerr<<e;
			return NULL;
		}

		return orientor->GetOutput();
	}
	else
	{
		return NULL;
	}
}

void Image_Convert_Base::WriteTonii(vtkSmartPointer<vtkImageData> image,std::string name)
{

	v2iConnectorType::Pointer connector = v2iConnectorType::New();
	connector->SetInput(image);
	try
	{
		connector->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cout<<"connect itk vtk error"<<std::endl;
		std::cerr<<e;
	}

	WriterType_Convert::Pointer nii_writer_parse = 
		WriterType_Convert::New();
	nii_writer_parse->SetInput(connector->GetOutput());
	std::string name1 = name;
	if (name1.compare(name1.size()-4,4,".nii") == 0)
	{}//do nothing
	else
	{
		name1.append(".nii");
	}
	nii_writer_parse->SetFileName(name1);
	nii_writer_parse->Write();
}
