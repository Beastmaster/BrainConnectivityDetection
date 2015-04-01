#include "RegistrationProcess.h"



Reg_Image_Type Registration_Process(Reg_Image_Type fix, Reg_Image_Type src) 
{
	//1. connect fixed image to ITK data type
	vtkSmartPointer<vtkImageData> temp_fixed_image = 
		vtkSmartPointer<vtkImageData>::New();
	temp_fixed_image->DeepCopy(fix);
	v2iConnectorType::Pointer v2iconnector_pre_temp = v2iConnectorType::New();
	v2iconnector_pre_temp->SetInput(temp_fixed_image);
	try
	{
		v2iconnector_pre_temp->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "**Error converting vtk type to itk type" << std::endl;
		std::cerr << err << std::endl;
		return NULL;
	}
	ImageTypex::Pointer itk_temp_fixed_image = v2iconnector_pre_temp->GetOutput();


	//2. connect source image to ITK data type
	vtkSmartPointer<vtkImageData> temp_src_image = 
		vtkSmartPointer<vtkImageData>::New();
	temp_src_image->DeepCopy(src);
	v2iConnectorType::Pointer v2iconnector_pre_temp1 = v2iConnectorType::New();
	v2iconnector_pre_temp1->SetInput(temp_src_image);
	try
	{
		v2iconnector_pre_temp1->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "**Error converting vtk type to itk type" << std::endl;
		std::cerr << err << std::endl;
		return NULL;
	}
	ImageTypex::Pointer itk_temp_src_image = v2iconnector_pre_temp1->GetOutput();


	//3. resample images 
	typedef itk::ResampleImageFilter<ImageTypex,ImageTypex> ResampleFilterType;
	ResampleFilterType::Pointer resample_filter = ResampleFilterType::New();
	typedef itk::AffineTransform<double,3> TransformType;
	TransformType::Pointer transform = TransformType::New();
	resample_filter->SetTransform(transform);
	typedef itk::NearestNeighborInterpolateImageFunction<ImageTypex,double> InterpolatorType;
	InterpolatorType::Pointer interpolator = InterpolatorType::New();
	resample_filter->SetInterpolator(interpolator);
	resample_filter->SetDefaultPixelValue(0);

	ImageTypex::SpacingType src_spacing = 	itk_temp_src_image->GetSpacing();
	ImageTypex::PointType   src_origin  =  itk_temp_src_image->GetOrigin();
	ImageTypex::SizeType    src_size    =  itk_temp_src_image->GetLargestPossibleRegion().GetSize();
	ImageTypex::DirectionType direction;
	direction.SetIdentity();

	resample_filter->SetOutputDirection(direction);
	resample_filter->SetOutputSpacing(src_spacing);
	resample_filter->SetOutputOrigin(src_origin);
	resample_filter->SetInput(itk_temp_fixed_image);
	try
	{
		resample_filter->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "**Error Resampling!!**" << std::endl;
		std::cerr << err << std::endl;
		return NULL;
	}
	ImageTypex::Pointer resampled_itk_temp_fixed_image = resample_filter->GetOutput();

	//4. run register process
	ImageTypex::Pointer  registered_img= ImageTypex::New();
	typedef Register< float, float, float > RegisterType_process;
	RegisterType_process * reg1 = new RegisterType_process;    
	reg1->SetFixedImage( resampled_itk_temp_fixed_image );
	reg1->SetMovingImage( itk_temp_src_image );
	reg1->GenerateTranformMatrix();    
	reg1->GetRegisteredMovingImage( registered_img );
	delete reg1;


	//5. convert itk image to vtk image
	i2vConnectorType::Pointer i2vconnector_lag_temp = i2vConnectorType::New();
	i2vconnector_lag_temp->SetInput(registered_img);
	try
	{
		i2vconnector_lag_temp->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "**Error converting ITK type to VTK type" << std::endl;
		std::cerr << err << std::endl;
		return NULL;
	}

	//5. get output
	return i2vconnector_lag_temp->GetOutput();
}
