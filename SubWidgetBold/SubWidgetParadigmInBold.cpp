/****************
*Project:   NeuroArea (CUHK)
*Author:    Qin Shuo
*Reference: 3D Slicer
*Date:      2015.9.10
*Version:   2.0
***************/
#include "SubWidgetParadigmInBold.h"
#include "ui_SubWidgetParadigmInBold.h"



//Public Use Class
//Description:  Private defined threshold function class
//Input:        Multi-image input 
class VTK_FMRIENGINE_EXPORT  ActivationThreshold : public vtkMultipleInputsImageFilter
{
public:
	static ActivationThreshold *New();
	vtkTypeMacro(ActivationThreshold, vtkMultipleInputsImageFilter);

	vtkSetMacro(pos_threshold, float);
	vtkSetMacro(neg_threshold, float);
private:
	float pos_threshold;
	float neg_threshold;
	int dims[3];               // array of dimensions
	int x;                     // x dimension of activation volume 
	int y;                     // y dimension of activation volume 
	int z;                     // z dimension of activation volume 
	unsigned long int size;    // size of the image inputs
protected:
	ActivationThreshold()
	{
		this->pos_threshold = 0;
		this->neg_threshold = 0;
	};
	~ActivationThreshold() {};
	//Implement virtual function to run filter
	void SimpleExecute(vtkImageData *input, vtkImageData *output)
	{ 
		this->GetInput(0)->GetDimensions(dims);
		x = dims[0];
		y = dims[1];
		z = dims[2];
		std::cout<<dims[0]<<dims[1]<<dims[2]<<std::endl;
		size = x*y*z;
		int numberOfInputs;
	#if (VTK_MAJOR_VERSION >= 5)
		numberOfInputs = this->GetNumberOfInputConnections(0);
	#else  
		numberOfInputs = this->NumberOfInputs;
	#endif
		output->SetDimensions(dims);
		output->SetScalarType(VTK_FLOAT);
		output->SetSpacing(input->GetSpacing());
		output->SetOrigin(input->GetOrigin());
		output->AllocateScalars();

		// get the data array from input image 
		vtkFloatArray *inputArray = (vtkFloatArray *)this->GetInput(0)->GetPointData()->GetScalars();
		vtkFloatArray *activation = vtkFloatArray::New();
		for (unsigned long int i=0; i<size; i++)
		{
			if (inputArray->GetValue(i) >= pos_threshold)
			{
				activation->InsertNextValue(inputArray->GetValue(i));
			}
			else if (inputArray->GetValue(i) <= neg_threshold)
			{
				activation->InsertNextValue(inputArray->GetValue(i));
			}
			else
			{
				activation->InsertNextValue(0.0);
			}
		} 
		output->GetPointData()->SetScalars(activation);
		activation->Delete();
	};
	// If the output image of a filter has different properties from the input image
	// we need to explicitly define the ExecuteInformation() method
	void ActivationThreshold::ExecuteInformation(vtkImageData *input, vtkImageData *output)
	{
		output->SetDimensions(dims);
		output->SetScalarType(VTK_INT);
		output->SetSpacing(1.0,1.0,1.0);
		output->SetOrigin(0.0,0.0,0.0);
		output->AllocateScalars();
	}
};vtkStandardNewMacro(ActivationThreshold);//Badly required when creating new filter



//Function: Parse log file and put data to container
//Return Value:
//				0: Success
//				1: Log file cannot open
//				2: Log information invalid
//				3: Volume selection exit
//				4: Load single file
int Data_ReConstruction::ParseLogFile()
{
	//clear data_container
	if(!this->dataContainer.empty())
		this->dataContainer.clear();

	//select read mod, judging by filename_list null or not
	if(this->filenames_list.isEmpty())
	{
		//-----single .dcm files load from log file------//
		qDebug()<<"begin parse log file !";
		//open log file
#ifdef QS_DEBUG_MODE
		QString log = 
			QFileDialog::getOpenFileName(this,
			tr("open log file"),"./",tr("(*)"));
		if (log.isEmpty()){return 10;}

#else	//get file name from main window
		QString log = QString(log_filename.c_str()) ;
#endif

		qDebug()<<log;
		//create file handle
		QFile file;
		file.setFileName(log);
		//read file line by line
		if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			std::cout<<"File cannot open"<<std::endl;
			return 1;
		}

		//use text stream to read line
		QTextStream in(&file);	
		//get col number
		QString line1;
		qint64 line_number=0;

		//clear File_info
		if (this->info != NULL)
		{
			delete this->info;
		}
		this->info = new File_info;

		while(!in.atEnd())
		{
			line1 = in.readLine();

			//find if there is "<",">"
			//if not, skip to next line
			if(!line1.contains("<",Qt::CaseInsensitive))
				continue;
			//split by "<" ">"
			QString name = line1.split("<").at(1);
			QString head = name.split(">").at(0);
			QString name_2 = name.split(">").at(1);

			if(head == "Patient_Name")
			{
				//qDebug()<<"Patient_Name";
				this->info->name.append(name_2);
			}
			if(head == "Series_Description")
			{
				//qDebug()<<"Series_Description";
				this->info->description.append(name_2);
			}
			if(head == "Slice_Number")
			{
				//qDebug()<<"Slice_Number";
				this->info->slice_number.append(name_2);
			}
			if(head == "NumberOfTemporalPositions")
			{
				//qDebug()<<"NumberOfTemporalPositions";
				this->info->temperal_number.append(name_2);
			}		
			if(head == "Path")
			{
				//qDebug()<<"path position";
				this->info->path_position<<in.pos();
			}
		}
		qDebug()<<"parse log done !";
		//close file
		file.close();

		//valid log file
		if(this->info->description.empty())
		{
			std::cout<<"The log file is invalid!"<<std::endl;//QMessageBox::information(NULL,"ERROR","The log file is invalid!",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
			return 2;
		}
	}
	else
	{
		std::cout<<"Please load single file"<<std::endl;
		return 4;
	}

	return 0;
}

//Function: Run construct process
//Return Value:
//				0: Success
//				1: Log file cannot open
//				2: Slice number and volume number do not match 
int Data_ReConstruction::ConsturctData()
{
	if (this->filenames_list.size() == 0)
	{
		//to see if there are enough volume for fmri analysis
		if (info->temperal_number.at(select_index).toInt()<30)
		{
			std::cout<<"No enough volumes(60 at least)"<<std::endl;//QMessageBox::warning(NULL,"Warning","No enough volumes(60 at least)",QMessageBox::Ok,QMessageBox::Ok);
			return 1;
		}

		//parse from log and add data to container
		this->Log2Container(this->info, this->dataContainer,this->select_index);	
	}	
	else
	{
		//-----connected multi slice volume------//
		QFileInfo fileinfo(this->filenames_list.at(0));
		QString directory=fileinfo.absolutePath();
		qDebug()<<directory;
		//connected volume
		QString suffix_dcm = "dcm";
		QString suffix_mha = "mha";
		QString suffix_nii = "nii";
		QString suffix_hdr = "hdr";
		for(int i=0;i<filenames_list.size();i++)
		{
			QFileInfo fileinfox(this->filenames_list.at(i));
			QString directory_x = directory;
			QString full_name = directory_x.append(fileinfox.fileName());
			const char* filename = this->filenames_list.at(i).toStdString().data();//full_name.toStdString().data();
			qDebug()<<this->filenames_list.at(i).toStdString().data();

			if(fileinfox.suffix() == suffix_dcm)
			{//read dicom
				vtkSmartPointer<vtkDICOMImageReader> reader=
					vtkSmartPointer<vtkDICOMImageReader>::New();
				reader->SetFileName(this->filenames_list.at(i).toStdString().data());//(filename);
				reader->Update();
				qDebug()<<"reader "<<filename<<" successfully";
				//add file into data_container
				this->dataContainer[i] = reader->GetOutput();
			}
			else if(fileinfox.suffix() == suffix_mha)
			{
				//vtkSmartPointer<vtkMetaImageReader> reader=
				//	vtkSmartPointer<vtkMetaImageReader>::New();
				//reader->SetFileName(this->filenames_list.at(i).toStdString().data());//(filename);
				//qDebug()<<"reader "<<filename<<" successfully";
				//reader->Update();
				////add file into data_container
				//this->data_container[i] = reader->GetOutput();
			}
			else if(fileinfox.suffix() == suffix_nii || fileinfox.suffix() == suffix_hdr)
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
				reader->SetFileName(this->filenames_list.at(i).toStdString().data());
				reader->Update();

				//itk-vtk connector
				//typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
				i2vConnectorType::Pointer connector = i2vConnectorType::New();
				connector->SetInput(reader->GetOutput());
				connector->Update();

				this->dataContainer[i] = vtkSmartPointer<vtkImageData>::New();
				this->dataContainer[i]->DeepCopy(connector->GetOutput());
				qDebug()<<"(nii/hdr)reader "<<this->filenames_list.at(i).toStdString().data()<<" successfully";
			}

			else
			{
				std::cout<<"File format error"<<std::endl;//QMessageBox::information(NULL,"File format error","Should be dcm/nii/mha");
				return 5;
			}
		}
	}

	this->filenames_list.clear();
	qDebug()<<"load files successfully!!";
	return this->PreProcess();;
}

//Function: Select certain slices to reconstruct data and put them into the data container
//Return Value:
//				0: Success
//				1: Log file cannot open
//				2: Slice number and volume number do not match 
//				3: Cannot found in the DICOM header
//				4: Entry was not of string type
int Data_ReConstruction::Log2Container(File_info* info,std::map<int,vtkSmartPointer<vtkImageData> > &container,int index)
{
	int size = 0;
	int slice_number = 0;
	int volume_number= 0;
	//find number of item in QStringList
	size = info->description.size();
	slice_number = info->slice_number.at(index).toInt();
	volume_number= info->temperal_number.at(index).toInt();

	qDebug()<<"volume info:"<<info->description.at(index)
		<<info->slice_number.at(index)<<info->temperal_number.at(index);

	//--read log again to find file path--//
	QFile file;
	QString log = QString(log_filename.c_str());	
	file.setFileName(log);
	//read file line by line
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return 1;
	//use text stream to read line
	QTextStream in(&file);
	//seek to path
	in.seek(info->path_position.at(index));
	//1. put name into a vector
	std::map<int,std::string> name_to_sort_all;

	//progress bar
	MyProgressDialog* dialogLoading_re_parse = new MyProgressDialog();
	dialogLoading_re_parse->setLabelText("Reading fMRI images");
	dialogLoading_re_parse->setMaximum(slice_number-(slice_number%volume_number));
	dialogLoading_re_parse->setValue(0);

	int cnt = 0;
	int total = slice_number-(slice_number%volume_number);
	while(cnt<total)
	{
		QString f_name = in.readLine();
		name_to_sort_all[GetDicomTag_InstanceNumber(f_name.toStdString())] = f_name.toStdString();
		dialogLoading_re_parse->setValue(cnt);
		cnt++;
	}
	file.close();//read DicomSortLists.txt done
	//delete dialogLoading_re_parse;

	//2. put name_to_sort by order
	std::vector< std::vector<std::string> > name_holder(volume_number);//QString to hold file names
	int cnt2 = 0;
	for (std::map<int,std::string>::iterator it = name_to_sort_all.begin();
		it!=name_to_sort_all.end();++it)
	{
		if (cnt2<slice_number-(slice_number%volume_number))
		{
			int xx = cnt2%volume_number;
			name_holder[xx].push_back((*it).second);
			cnt2++;
		}
		else
			continue;
	}

	//to find slice number and volume number match or not
	if(name_holder.begin()->size()!=name_holder.back().size())
	{
		std::cout<< "slice and volume do not match"<<std::endl;//QMessageBox::about(NULL, "ERROR", "slice and volume do not match");
		return 2;
	}

	//define itk reader
	//typedef itk::Image< float , 3 >             ImageType; //image type pixel:float;dimension:3
	//typedef itk::ImageSeriesReader< ImageType > ReaderType;
	ReaderType_b::Pointer itk_reader = ReaderType_b::New();
	DicomIOType::Pointer  dicomIO = DicomIOType::New();
	itk_reader->SetImageIO(dicomIO);

	//typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
	i2vConnectorType::Pointer connector = i2vConnectorType::New();

	//clear data container if not NULL
	if(!container.empty())
		container.clear();


	//progress bar
	//MyProgressDialog* dialogLoading_File = new MyProgressDialog();
	dialogLoading_re_parse->setLabelText("Reslicing FMRI files");
	dialogLoading_re_parse->setMaximum(volume_number);
	dialogLoading_re_parse->setValue(0);

	for(int i = 0;i<volume_number;i++)
	{
		//change process bar
		dialogLoading_re_parse->setValue(i);
		
		//begin
		itk_reader->SetFileNames(name_holder[i]);
		itk_reader->Update();
		//get information from dicom tags( thickness among slices)
		const  DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();
		DictionaryType::ConstIterator itr = dictionary.Begin();
		DictionaryType::ConstIterator end = dictionary.End();

		std::string sliceThicknessID = "0018|0050";
		std::string sliceOriginID    = "0020|0032";
		DictionaryType::ConstIterator Thickness_tagItr = dictionary.Find( sliceThicknessID );
		DictionaryType::ConstIterator Origin_tagItr    = dictionary.Find( sliceOriginID );

		if( Thickness_tagItr == end | Origin_tagItr == end)
		{
			std::cerr << "Tag " << sliceThicknessID<<"or"<<"sliceOriginID";
			std::cerr << " not found in the DICOM header" << std::endl;
			return 3;
		}
		MetaDataStringType::ConstPointer Thickness_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Thickness_tagItr->second.GetPointer() );
		MetaDataStringType::ConstPointer Origin_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Origin_tagItr->second.GetPointer() );

		std::string Thickness_tagvalue;
		std::string Origin_tagvalue;

		if( Thickness_entryvalue || Origin_entryvalue)
		{
			Thickness_tagvalue = Thickness_entryvalue->GetMetaDataObjectValue();
			Origin_tagvalue    = Origin_entryvalue   ->GetMetaDataObjectValue();
			std::cout << "slice thickness is (" << sliceThicknessID <<  ") ";
			std::cout << " is: " << Thickness_tagvalue << std::endl;
			std::cout << "origin is (" << sliceOriginID <<  ") ";
			std::cout << " is: " << Origin_tagvalue << std::endl;
		}
		else
		{
			std::cerr << "Entry was not of string type" << std::endl;
			return 4;
		}
		double slice_thickness = std::stod(Thickness_tagvalue);

		//connect
		connector->SetInput(itk_reader->GetOutput());
		connector->Update();
		//output vtkimagedata to data_container
		vtkSmartPointer<vtkImageData> buff =
			vtkSmartPointer<vtkImageData>::New();
		buff = connector->GetOutput();
		qDebug()<<"converter: "<<i<<" done!";
		//write to .mha for test
		if(1)
		{		
			//-----------change information(origin and spacing)--------------//
			vtkSmartPointer<vtkImageChangeInformation> changer = 
				vtkSmartPointer<vtkImageChangeInformation>::New();
			//get information :origin
			//it seems that there is no need to change origin
			double origin[3]={0,0,0};
			buff->GetOrigin(origin);

			qDebug()<<"origin:"<<origin[0]<<origin[1]<<origin[2];

			double spacing[3]={0,0,0};
			buff->GetSpacing(spacing);
			spacing[2] = slice_thickness;
			changer->SetOutputSpacing(spacing);
			//change 
			changer->SetInput(buff);
			//changer->SetOutputOrigin(origin);
			changer->Update();

			//vtkSmartPointer<vtkImageData> buff_c =
			//	vtkSmartPointer<vtkImageData>::New();
			//buff_c = changer->GetOutput();
			container[i] = vtkSmartPointer<vtkImageData>::New();
			container[i]->DeepCopy(changer->GetOutput());
		}
		// if cancel read image, stop and clear all
		if (dialogLoading_re_parse->wasCanceled())
		{
			//close file first
			file.close();
			//clear data container 
			this->dataContainer.clear();
			//delete 
			break;
		}
	}
	delete dialogLoading_re_parse;
	if (file.isOpen())
	{
		file.close();
	}
	return 0;
}


//Public Use Function
//Description: Get Instance Number from Dicom tag
//Input:       single slice image dicom file name, generate from DicomSortLists.txt
//Problem:     It read slices one more time just to get instance number. Cost read-write time.
int Data_ReConstruction::GetDicomTag_InstanceNumber(std::string slice_name)
{
	typedef float			   PixelType;
	const unsigned int         Dimension = 2;

	typedef itk::Image< PixelType, Dimension >      ImageType;
	typedef itk::ImageFileReader< ImageType >     ReaderType;

	ReaderType::Pointer reader = ReaderType::New();

	typedef itk::GDCMImageIO       ImageIOType;
	ImageIOType::Pointer dicomIO = ImageIOType::New();

	reader->SetFileName( slice_name );
	reader->SetImageIO( dicomIO );

	try
	{
		reader->Update();
	}
	catch (itk::ExceptionObject &ex)
	{
		std::cout << ex << std::endl;
		return EXIT_FAILURE;
	}

	typedef itk::MetaDataDictionary   DictionaryType;

	const  DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();

	typedef itk::MetaDataObject< std::string > MetaDataStringType;

	DictionaryType::ConstIterator itr = dictionary.Begin();
	DictionaryType::ConstIterator end = dictionary.End();

	//Instance Number
	std::string InstanceNumberTag = "0020|0013";
	DictionaryType::ConstIterator tagItr = dictionary.Find( InstanceNumberTag );
	int InstanceNumber = 0;
	if( tagItr != end )
	{
		MetaDataStringType::ConstPointer entryvalue =
			dynamic_cast<const MetaDataStringType *>(
			tagItr->second.GetPointer() );

		if( entryvalue )
		{
			std::string tagvalue = entryvalue->GetMetaDataObjectValue();
			//std::cout << "Patient's Name (" << InstanceNumberTag <<  ") ";
			//std::cout << " is: " << tagvalue.c_str() << std::endl;
			////put value to instancenumber
			InstanceNumber = std::stoi(tagvalue);
		}
	}
	return InstanceNumber;
}



//Return Value:
//				0: Update process success
//				1: contrast vector invalid
//				2: vtk image to itk image failed
int Data_ReConstruction::PreProcess()
{	
	qDebug()<<"pre-processing ! !";
	//progress bar
	MyProgressDialog* dialogLoading_File_reg = new MyProgressDialog();
	dialogLoading_File_reg->setLabelText("Pre-processing...");
	dialogLoading_File_reg->setMaximum(this->dataContainer.size());
	dialogLoading_File_reg->setValue(0);

	typedef Register< float, float, float > fMRI2T2RegisterType_pre;
	vtkSmartPointer<vtkImageData> temp_fixed_fmri_image = vtkSmartPointer<vtkImageData>::New();
	temp_fixed_fmri_image->DeepCopy(this->dataContainer[0]);

	v2iConnectorType::Pointer v2iconnector_pre_temp = v2iConnectorType::New();
	v2iconnector_pre_temp->SetInput(temp_fixed_fmri_image);
	try
	{
		v2iconnector_pre_temp->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "**Error converting vtk type to itk type" << std::endl;
		std::cerr << err << std::endl;
		return 2;
	}
	ImageTypex::Pointer itk_temp_fixed_fmri_image = v2iconnector_pre_temp->GetOutput();

	//iterate through the data container
	int tt=0;
	for(std::map<int,vtkSmartPointer<vtkImageData> >::iterator it = (this->dataContainer.begin())++;
#ifndef QS_DEBUG_MODE
		it!=this->dataContainer.end(); 
#else
		it==this->data_container.end();//if deine QS_DEBUG_MODE flag, skip register across all data
#endif
	++it)////////////////////////////////
	{
		//progress change
		tt++;
		dialogLoading_File_reg->setValue(tt);

		//vtk image data to itk image data 
		v2iConnectorType::Pointer v2iconnector_pre_temp = v2iConnectorType::New();
		v2iconnector_pre_temp->SetInput((*it).second);
		try
		{
			v2iconnector_pre_temp->Update();
		}
		catch( itk::ExceptionObject &err )
		{
			std::cerr << "**Error converting vtk type to itk type" << std::endl;
			std::cerr << err << std::endl;
			return 3;
		}

		ImageTypex::Pointer registered_fmri_img     = ImageTypex::New(); 
		fMRI2T2RegisterType_pre * reg1 = new fMRI2T2RegisterType_pre;    
		reg1->SetFixedImage( itk_temp_fixed_fmri_image );
		reg1->SetMovingImage( v2iconnector_pre_temp->GetOutput() );
		reg1->GenerateTranformMatrix();    
		reg1->GetRegisteredMovingImage( registered_fmri_img );
		delete reg1;

		//convert back to vtk image
		i2vConnectorType::Pointer i2vconnector_pre_temp = i2vConnectorType::New();
		i2vconnector_pre_temp->SetInput(registered_fmri_img);
		try
		{
			i2vconnector_pre_temp->Update();
		}
		catch( itk::ExceptionObject &err )
		{
			std::cerr << "**Error converting vtk type to itk type" << std::endl;
			std::cerr << err << std::endl;
			return 4;
		}
		(*it).second->DeepCopy(i2vconnector_pre_temp->GetOutput());

		std::cout<<".\n register "<<(*it).first<<" th image done \n \n"<<std::endl;
	}
	delete dialogLoading_File_reg;

	//progress bar for gassian smoothing
	MyProgressDialog* dialogLoading_File_smooth = new MyProgressDialog();
	dialogLoading_File_smooth->setMaximum(dataContainer.size());
	dialogLoading_File_smooth->setLabelText("Running Gassian Smoothing..");
	dialogLoading_File_smooth->setValue(0);
	//1.  Gassian smoothing
	int ii=0;
	for(std::map<int,vtkSmartPointer<vtkImageData> >::iterator it = this->dataContainer.begin();
		it!=this->dataContainer.end();++it)
	{
		//progress change
		dialogLoading_File_smooth->setValue(ii);

		vtkSmartPointer<vtkImageGaussianSmooth> smooth_h=
			vtkSmartPointer<vtkImageGaussianSmooth>::New();
		//smooth parameter
		//smooth_h->SetRadiusFactor(3);
		//smooth_h->SetStandardDeviation();
		vtkSmartPointer<vtkImageData> buff_h=
			vtkSmartPointer<vtkImageData>::New();
		smooth_h->SetInput((*it).second);

		smooth_h->Update();

		buff_h = smooth_h->GetOutput();
		(*it).second = buff_h;
		qDebug()<<"smoothing "<<ii++<<"th image";
	}
	delete dialogLoading_File_smooth;


	//progress bar for thresholding
	MyProgressDialog* dialogLoading_File_thrd = new MyProgressDialog();
	dialogLoading_File_thrd->setMaximum(dataContainer.size());
	dialogLoading_File_thrd->setLabelText("Thresholding..");
	dialogLoading_File_thrd->setValue(0);
	//2.   thresholding
	int jj=0;
	for(std::map<int,vtkSmartPointer<vtkImageData> >::iterator it = this->dataContainer.begin();
		it!=this->dataContainer.end();++it)
	{
		//progress change
		dialogLoading_File_thrd->setValue(jj);

		vtkSmartPointer<vtkImageThreshold> threshold_h=
			vtkSmartPointer<vtkImageThreshold>::New();
		//threshold parameter
		threshold_h->ThresholdBetween(10,9000);
		threshold_h->ReplaceOutOn();
		threshold_h->SetOutValue(0.0);
		threshold_h->SetInput((*it).second);
		threshold_h->Update();
		vtkSmartPointer<vtkImageData> buff_h=
			vtkSmartPointer<vtkImageData>::New();
		buff_h = threshold_h->GetOutput();
		(*it).second = buff_h;
		qDebug()<<"thresholding "<<jj++<<"th image";
	}
	delete dialogLoading_File_thrd;

	//3.  write to file for test
	int kk=0;
	for(std::map<int,vtkSmartPointer<vtkImageData> >::iterator it = this->dataContainer.begin();
		it==this->dataContainer.end();++it)//"!=" do /"==" undo
	{
		std::stringstream ssx;
		ssx<<"th-smod"<<kk++<<".nii";

		std::string name = ssx.str();

		//the following code is vtk image data to itk image data 
		//write to .nii file!
		v2iConnectorType::Pointer v2iconnector = 
			v2iConnectorType::New();
		v2iconnector->SetInput((*it).second);
		v2iconnector->Update();

		WriterType_b::Pointer nii_writer = 
			WriterType_b::New();
		nii_writer->SetInput(v2iconnector->GetOutput());
		nii_writer->SetFileName(name);
		nii_writer->Update();
		qDebug()<<"writing "<<kk<<"th image";
	}
	qDebug()<<"pre-processing done!";
	return 0;
}


//Description:   Put data in vector into vtkfloat array
//Function:      Set parameters to siglm
//Return value:  
//				0  :  function is done successfully
//				1  :  siglm is null
//				2  :  size of onset and duration is not matched
int Fmri_paradigm::VectorToArray(vector< vector<int> > onset,vector< vector<int> > duration,SignalModeling* siglm)
{
	//valid siglm
	if (siglm == NULL)
	{
		std::cout<<"siglm is NULL"<<std::endl;
		return 1;
	}

	//simple check size of onsets and duration
	if (onset.size() != duration.size())
	{
		std::cout<<"size of onset and duration not match"<<std::endl;
		return 2;
	}

	//put data in vector into vtkfloat array
	vtkFloatArrayType tem_onset    = vtkFloatArrayType::New();
	vtkFloatArrayType tem_duration = vtkFloatArrayType::New();

	tem_onset->SetNumberOfComponents(onset.size());
	//tem_onset->SetNumberOfTuples(this->onset.front().size());
	tem_duration->SetNumberOfComponents(duration.size());
	//tem_duration->SetNumberOfTuples(this->duration.front().size());
	int j_onset = 0;//component
	for (vector< vector<int> >::iterator it = onset.begin();it!=onset.end();++it)
	{
		int i=0;//tuple
		for(vector<int>::iterator ti = it->begin();ti!=it->end();++ti)
		{
			tem_onset->InsertComponent(i++,j_onset,*ti);
		}
		j_onset++;
	}
	int j_dur = 0;//component
	for (vector< vector<int> >::iterator it = duration.begin();it!=duration.end();++it)
	{
		int i=0;//tuple
		for(vector < int >::iterator ti = it->begin();ti!=it->end();++ti)
		{
			tem_duration->InsertComponent(i++,j_dur,*ti);
		}
		j_dur++;
	}
	return 0;
}


//Return Value:
//				0: Update process success
//				1: contrast vector invalid
//				2: The experiment parameters does not match with number of volumes
int Fmri_Module::Update()
{	
	//check designMat first
	if (data_container.size()!=designMat->GetNumberOfTuples())
	{
		std::cout<<"The experiment parameters does not match with number of volumes"<<std::endl;
		return 2;
	}

	//Generate design matrix
	typedef Register< float, float, float > fMRI2T2RegisterType_pre;
	vtkSmartPointer<vtkImageData> temp_fixed_fmri_image = vtkSmartPointer<vtkImageData>::New();
	temp_fixed_fmri_image->DeepCopy(this->data_container[0]);

	v2iConnectorType::Pointer v2iconnector_pre_temp = v2iConnectorType::New();
	v2iconnector_pre_temp->SetInput(temp_fixed_fmri_image);
	try
	{
		v2iconnector_pre_temp->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "**Error converting vtk type to itk type" << std::endl;
		std::cerr << err << std::endl;
		return 2;
	}
	ImageTypex::Pointer itk_temp_fixed_fmri_image = v2iconnector_pre_temp->GetOutput();

	//generate transform matrix
	fMRI2T2RegisterType_pre * reg_first_fmri = new fMRI2T2RegisterType_pre;    
	reg_first_fmri->SetFixedImage( this->T2_image_reference );
	reg_first_fmri->SetMovingImage( itk_temp_fixed_fmri_image );
	reg_first_fmri->GenerateTranformMatrix();    
	this->transfrom_matrix = reg_first_fmri->GetTranformMatrix();
	delete reg_first_fmri;
	std::cout<<"register first with T2 image done"<<std::endl;


	vtkSmartPointer<vtkGLMDetector> GLMDetector=vtkGLMDetector::New();
	// pass design matrix to GLMDetector
	GLMDetector->SetDesignMatrix(this->designMat);
	// Gets/Sets the activation detection method (GLM = 1; MI = 2).
	GLMDetector->SetDetectionMethod(ACTIVATION_DETECTION_METHOD_GLM);//default parameter

	//-----setup parameters for estimator
	vtkSmartPointer<vtkCallbackCommand> progressCallback2 = 
		vtkSmartPointer<vtkCallbackCommand>::New();
	progressCallback2->SetCallback(&Fmri_Module::GLMEstimator_Progress_Func);

	//Description: the estimation must occur before any activation volumes can be generated
	vtkSmartPointer<vtkGLMEstimator> GLMEstimator=vtkGLMEstimator::New();
	GLMEstimator->AddObserver(vtkCommand::ProgressEvent, progressCallback2);
	//set detector first
	GLMEstimator->SetDetector(GLMDetector);
	//add detector first to estimator
	GLMEstimator->SetDetector(GLMDetector);
	//Whether prewhiten or not(1/0)
	GLMEstimator->SetPreWhitening(0);//default
	// Enables or disables high-pass filtering.0:disable
	GLMEstimator->EnableHighPassFiltering(0);//default
	//Sets/Gets global effect.1:grand mean;2:global mean,3:pre-whiten data
	GLMEstimator->SetGlobalEffect(1);//default parameter
	//add data to GLMEstimator
	GLMEstimator->RemoveAllInputs();
	for(int i = 0;i<this->data_container.size();i++)
		GLMEstimator->AddInput(this->data_container[i]);

	GLMEstimator->Update();
	this->BetaMap = GLMEstimator->GetOutput();

	//check contrast vector
	if(this->contrast->GetNumberOfTuples()==0)
	{
		std::cout<<"No Contrast specified"<<std::endl;//QMessageBox::information(NULL,"Error","No contrast specified",QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
		return 1;
	}

	vtkSmartPointer<vtkCallbackCommand> progressCallback = 
		vtkSmartPointer<vtkCallbackCommand>::New();
	progressCallback->SetCallback(&Fmri_Module::GLMVGenerator_Progress_Func);

	//clear input to GLMVolumeGenerator
	vtkSmartPointer<vtkGLMVolumeGenerator> GLMVolmeGenerator = vtkGLMVolumeGenerator::New();
	GLMVolmeGenerator->AddObserver(vtkCommand::ProgressEvent, progressCallback);
	//Determine whether to prewhiten 0/1
	GLMVolmeGenerator->SetPreWhitening(0);//default
	//generating tMap
	GLMVolmeGenerator->SetContrastVector(this->contrast);
	GLMVolmeGenerator->SetDesignMatrix(this->designMat);
	GLMVolmeGenerator->SetInput(this->BetaMap);
	GLMVolmeGenerator->Update();
	this->tMap = GLMVolmeGenerator->GetOutput();
	
	qDebug()<<"activate done";
	UpDateParameters();
	return 0;
}

//Return value:
//				0: update success
int Fmri_Module::UpDateParameters()
{
	vtkSmartPointer<ActivationThreshold> act_Threshold_hd = vtkSmartPointer<ActivationThreshold>::New();
	float neg_threshold = -this->t;
	float pos_threshold =  this->t;
	act_Threshold_hd->Setneg_threshold(neg_threshold);
	act_Threshold_hd->Setpos_threshold(pos_threshold);
	act_Threshold_hd->SetInput(this->tMap);
	act_Threshold_hd->Update();
	this->ActArea->DeepCopy(act_Threshold_hd->GetOutput());

	this->ApplyTransform(this->ActArea);

	return 0;
}


//Return Value:
//				0: Register success
//				1: vtk to itk image connector failed
//				2: resampling failed
//				3: mask brain profile failed
//				4: itk image convert to vtk image failed
//				5: write image failed
int Fmri_Module::ApplyTransform(vtkImageData* &input_im)
{
	//progress dialog
	MyProgressDialog* dialog_register = new MyProgressDialog();
	dialog_register->setLabelText("Running Registration..");
	dialog_register->setMaximum(10);
	dialog_register->setValue(0);

	//convert act_area to itk image
	v2iConnectorType::Pointer v2iconnector_a = 
		v2iConnectorType::New();
	v2iconnector_a->SetInput(input_im);
	try
	{
		v2iconnector_a->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "error convert act_image to itk";
		std::cerr << err;
		delete dialog_register;
		return 1;
	}

	dialog_register->setMaximum(2);

	//-----apply matrix----//
	std::cout<<"applying transform matrix"<<std::endl;
	typedef itk::ResampleImageFilter< ImageTypex, ImageTypex > ResampleFilterType;
	ResampleFilterType::Pointer resampler_in_register = ResampleFilterType::New();
	resampler_in_register->SetTransform( this->transfrom_matrix );
	resampler_in_register->SetInput( v2iconnector_a->GetOutput() ); 
	resampler_in_register->SetSize( this->T2_image_reference->GetLargestPossibleRegion().GetSize() );
	resampler_in_register->SetOutputOrigin(  this->T2_image_reference->GetOrigin() );
	resampler_in_register->SetOutputSpacing( this->T2_image_reference->GetSpacing() );
	resampler_in_register->SetOutputDirection( this->T2_image_reference->GetDirection() );
	resampler_in_register->SetDefaultPixelValue( 0 );
	try
	{
		resampler_in_register->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "**Error resampling image" << std::endl;
		std::cerr << err << std::endl;
		delete dialog_register;
		return 2;
	}
	std::cout<<"apply transform matrix done"<<std::endl;
	dialog_register->setMaximum(3);

	//-----------mask regd_activate_img-------//
	ImageTypex::Pointer masked_activate_img = ImageTypex::New();
	if (en_Brain_Strip_flag)
	{
		std::cout<<"putting mask"<<std::endl;
		typedef itk::MaskImageFilter<ImageTypex, AtlasLabelType, ImageTypex> MaskFilterType;
		MaskFilterType::Pointer maskFilter = MaskFilterType::New();
		maskFilter->SetInput1(  resampler_in_register->GetOutput() );
		maskFilter->SetInput2( this->Brain_Mask );
		try
		{
			maskFilter->Update();
		}
		catch ( itk::ExceptionObject &exception )
		{
			std::cerr << "mask error!!" << std::endl;
			std::cerr << "Exception caught ! " << std::endl;
			std::cerr << exception << std::endl;
			delete dialog_register;
			return 3;
		}
		masked_activate_img = maskFilter->GetOutput();
		std::cout<<"mask process done"<<std::endl;
	}
	else
	{	
		masked_activate_img = resampler_in_register->GetOutput();
	}
	dialog_register->setMaximum(4);
	//--------------mask regd_activate_img-----------//


	//put registered image to container
	i2vConnectorType::Pointer i2vconnector_im2con = 
		i2vConnectorType::New();
	i2vconnector_im2con->SetInput(masked_activate_img);
	try
	{
		i2vconnector_im2con->Update();
	}
	catch ( itk::ExceptionObject &exception )
	{
		std::cerr << "itk to vtk error!!" << std::endl;
		std::cerr << "Exception caught ! " << std::endl;
		std::cerr << exception << std::endl;
		delete dialog_register;
		return 4;
	}

	input_im->DeepCopy(i2vconnector_im2con->GetOutput());
	dialog_register->setMaximum(5);

	//write activated area to nii
	WriterType_b::Pointer nii_writer_itkrgd_image = 
		WriterType_b::New();
	nii_writer_itkrgd_image->SetInput(masked_activate_img);
	nii_writer_itkrgd_image->SetFileName("act_area.nii");
	try
	{
		nii_writer_itkrgd_image->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "error write act_image";
		std::cerr << err;
		delete dialog_register;
		return 5;
	}
	dialog_register->setValue(10);
	delete dialog_register;
	return 0;
}
void  Fmri_Module::GLMVGenerator_Progress_Func(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
	vtkGLMVolumeGenerator* testFilter = static_cast<vtkGLMVolumeGenerator*>(caller);
	std::cout << "Progress: " << testFilter->GetProgress() << std::endl;
}
void  Fmri_Module::GLMEstimator_Progress_Func(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
	vtkGLMEstimator* testFilter = static_cast<vtkGLMEstimator*>(caller);
	std::cout << "Progress: " << testFilter->GetProgress() << std::endl;
}








SubWidgetParadigmInBold::SubWidgetParadigmInBold(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubWidgetParadigmInBold)
{
    ui->setupUi(this);
	this->init_para();
	connect(ui->conditionNameBox,SIGNAL(activated(const QString &)),
		this,SLOT( on_select_condition()),Qt::UniqueConnection);
	connect(ui->file_open_btn,SIGNAL(clicked()),this,SLOT(on_click_open()));
	connect(ui->bold_load_btn,SIGNAL(clicked()),this,SLOT(on_click_loadimage_m()));
	connect(ui->contrastNameBox,SIGNAL(activated(const QString &)),
		this,SLOT( on_select_contrast()),Qt::UniqueConnection);

	connect(ui->user_input_Btn,SIGNAL(clicked()),this,SLOT(on_click_user_input()));
	//connect test button
	connect(ui->one_click_test_Btn,SIGNAL(clicked()),this,SLOT(on_click_test()));
	connect(ui->advance_Btn,SIGNAL(clicked()),this,SLOT(on_click_advance()));
	connect(ui->use_default_Btn,SIGNAL(clicked()),this,SLOT(on_click_use_default_m())); 
	connect(ui->threshold_Btn,SIGNAL(clicked()),this,SLOT(on_click_threshold_m()));  
	connect(this,SIGNAL(emit_threshold()),this,SLOT(on_click_threshold()));
	connect(this,SIGNAL(errInfo(QString)),this,SLOT(err_information(QString)));
	//disable useless widget
	ui->enable_test_Btn->setVisible(false);
	ui->enable_test_Btn->setDisabled(true);
	ui->one_click_test_Btn->setVisible(false);
	ui->one_click_test_Btn->setDisabled(true);
}

SubWidgetParadigmInBold::~SubWidgetParadigmInBold()
{
	if (paradigm!=NULL)
	{
		delete paradigm;
		paradigm = NULL;
	}
	if (fmri_run!=NULL)
	{
		delete fmri_run;
		fmri_run = NULL;
	}
	if (data_constructer == NULL)
	{
		delete data_constructer;
		data_constructer = NULL;
	}

    delete ui;
}


//read input numbers and generate design matrix
void SubWidgetParadigmInBold::on_click_addcondition()
{
	if (ui->onsetLineEdit->text().isEmpty()||ui->durationLineEdit->text().isEmpty())
	{
		QMessageBox::warning(NULL,tr("Error!"),tr("Input is empty"),QMessageBox::Ok);
		return;
	}

	QStringList onset_list = ui->onsetLineEdit->text().split(",");
	QStringList duration_list = ui->durationLineEdit->text().split(",");

	if((onset_list.count() != duration_list.count())||onset_list.empty())
	{
		QMessageBox::warning(NULL,tr("Error!"),tr("length of duration and onset is different!"),QMessageBox::Ok);
		return;
	}
	vector<int> onset_tem;
	vector<int> duration_tem;
	for(int i = 0;i<onset_list.size();i++)
	{
		onset_tem.push_back(onset_list.at(i).toInt());
		duration_tem.push_back(duration_list.at(i).toInt());
	}
	paradigm->onset.push_back(onset_tem);
	paradigm->duration.push_back(duration_tem);
}

void SubWidgetParadigmInBold::on_select_condition()
{
	QString str;
	str = this->ui->conditionNameBox->currentText();
	
	if (str == "default")
	{
		this->ui->onsetLineEdit->setText(tr("10,30,50"));
		this->ui->durationLineEdit->setText(tr("10,10,10"));
	}

	this->on_click_addcondition();
}

void SubWidgetParadigmInBold::on_click_addcontrast()
{	
	QString contrast_text = this->ui->Vector_in->text();
	std::vector< int > con;  //temp vector to hold contrast vector
	if (contrast_text.isEmpty())
	{
		QMessageBox::warning(NULL,tr("Error!"),tr("Contrast is empty"),QMessageBox::Ok);
		return;
	}

	QStringList con_list = contrast_text.split(",");
	for (int i=0;i<con_list.size();i++)
		con.push_back(con_list[i].toInt());

	paradigm->SetContrast(con);
	this->contrast->DeepCopy(paradigm->GetContrast());

	model_flag = 1;
}
void SubWidgetParadigmInBold::on_select_contrast()
{
	QString str;
	str = this->ui->contrastNameBox->currentText();

	if (str == "default")
	{
		this->ui->Vector_in->setText(tr("1,0,0"));
	}
	this->on_click_addcontrast();
}

void SubWidgetParadigmInBold::set_contrast()
{
	this->ui->contrastNameBox->setCurrentIndex(0);
}

void SubWidgetParadigmInBold::on_click_use_default()
{
	paradigm = new Fmri_paradigm;
	//use default condition
	this->ui->conditionNameBox->setCurrentIndex(0);
	//trigger
	ui->onsetLineEdit->setText(tr("10,30,50"));
	ui->durationLineEdit->setText(tr("10,10,10"));
	this->on_select_condition();
	paradigm->SetTR(3);
	//paradigm->SetOnset(x_onset);
	//paradigm->SetDuration(x_duration);
	paradigm->SetNumCondition(1);
	paradigm->SetNumVolume(60);

	paradigm->Update();
	this->designMat = paradigm->GetOutput();

	int tuples=this->designMat->GetNumberOfTuples();
	int comp  =this->designMat->GetNumberOfComponents();

	std::ofstream file ("designmatrix.txt");
	if(file.is_open())
	{
		file<<"point data"<<std::endl;
		file<<"number of tuples:"<<tuples<<endl;
		file<<"number of component:"<<comp<<endl;

		for(int j=0;j<tuples;j++)
		{
			//file<<"\ncomponent:"<<j<<"\n";
			for(int i=0;i<comp;i++)
				file<<designMat->GetComponent(j,i)<<" ";

			file<<";";
		}
	}
	file.close();

	connect(this,SIGNAL(update_Contrast()),this,SLOT(set_contrast()));
	connect(this,SIGNAL(emit_sel_contrast()),this,SLOT(on_select_contrast()));
	emit update_Contrast();//this->ui->contrastNameBox->setCurrentIndex(0);
	emit emit_sel_contrast();;//this->on_select_contrast();
}

//tobe continued
void SubWidgetParadigmInBold::on_click_open()
{
	QMessageBox::information(NULL,"No Access","Please choose Load Imge from dataset",
		QMessageBox::Yes);
}
void SubWidgetParadigmInBold::on_click_loadimage()
{
	if(!( !SetMainWindow( MainWindow::GetMainWindow()) && //Set main window 
		  !SetPrjDir() &&      //Set project dir
		  !SetSrcDir() &&      //Set image source dir
		  !SetReference() &&   //Set reference image
		  !SetBrainMask()))     //Set brain mask
	{
		std::cout<<"There is something wrong ... "<<std::endl;
		emit errInfo("Please valid the DataSet");//QMessageBox::warning(NULL,tr("Error!"),tr("Please valid the DataSet "),QMessageBox::Ok);
		return;
	}

	if (this->data_constructer != NULL)
		delete this->data_constructer;
	data_constructer = new Data_ReConstruction;
	data_constructer->SetLogDir(this->img_prj_dir);
	if (data_constructer->ParseLogFile()!=0)
	{
		std::cout<<"Log Parse failed"<<std::endl;
		return;
	}
	this->info = data_constructer->GetInfo();
	connect(this,SIGNAL(proceed_stage2()),this,SLOT(loadimage_stage2()));
	emit proceed_stage2();
}

void SubWidgetParadigmInBold::loadimage_stage2()
{
	//Tree display of series
	Tree_Dialog* tree_index = new Tree_Dialog;
	tree_index->SetInfo(this->info,&this->select_index);
	if (tree_index->exec() != 100)
	{
		delete tree_index;
		return ;
	}
	delete tree_index;

	data_constructer->SetIndex(this->select_index);
	if (data_constructer->ConsturctData()!=0)
		return;
	connect(this,SIGNAL(proceed_stage3()),this,SLOT(loadiamge_stage3()));
	emit proceed_stage3();
}

void SubWidgetParadigmInBold::loadiamge_stage3()
{
	this->data_container = data_constructer->GetDataContainer();
	load_flag = 1;
}

void SubWidgetParadigmInBold::on_click_threshold_stage1()
{
	emit emit_threshold();
}

void SubWidgetParadigmInBold::on_click_threshold()
{
	//check flag first
	if ((load_flag*model_flag) == 0)
	{
		emit errInfo("Please Load file or Generate model first!");//QMessageBox::warning(NULL,tr("Error!"),tr("Please Load file or Generate model first!"),QMessageBox::Ok);
		return;
	}

	if (fmri_run!=NULL)
		delete fmri_run;
	
	fmri_run = new Fmri_Module;
	fmri_run->SetDataset(this->data_container);
	fmri_run->SetDesignMatrix(this->designMat);
	fmri_run->SetContrast(this->contrast);
	fmri_run->SetTransformMatrix(this->T2_fmri_register_matrix);
	fmri_run->SetBrainMask(this->Brain_Mask);
	fmri_run->SetReference(this->T2_image_reference);

	if (fmri_run->Update() != 0)
	{
		emit errInfo("Something WRONG with the process.. Please re-start .. ");//QMessageBox::warning(NULL,tr(//"Error!"),
		return;
	}

	this->ActArea = this->fmri_run->GetOutput();
	//this->Bold_Visualize();
	connect(this,SIGNAL(refresh_view()),this,SLOT(visualize_act()));
	connect(ui->pValue_in,SIGNAL(editingFinished()),this,SLOT(on_threshold_changed()));
	emit refresh_view();
}
void SubWidgetParadigmInBold::on_threshold_changed()
{
	//set t to display
	
	float p_value = ui->pValue_in->value();

	float t_value = Fmri_Module::P2T(p_value,fmri_run->Get_dof());
	ui->tStat_in->setText(QString::number(t_value));

	this->fmri_run->Set_t_Value(t_value);
	this->fmri_run->UpDateParameters();
	this->ActArea = this->fmri_run->GetOutput();

	qDebug()<<"threshold done";
	emit refresh_view();//this->Bold_Visualize();
}

void SubWidgetParadigmInBold::keyPressEvent(QKeyEvent *event)
{
	if(event->key() == Qt::Key_0)
	{
		float p_value=0.0;
		p_value = ui->pValue_in->text().toFloat();

		//set t to display
		ui->tStat_in->setText(QString::number(Fmri_Module::P2T(p_value,fmri_run->Get_dof())));
	}
}


void SubWidgetParadigmInBold::init_para()	
{
	this->designMat = vtkFloatArray::New();   //design matrix
	this->contrast  = vtkIntArray::New();	//contrast vector
	info     = NULL;
	paradigm = NULL;
	fmri_run = NULL;
	data_constructer = NULL;

	load_flag  = 0;
	model_flag = 0;

	//init multi thread
	load_thread[0]      = new thread_load();
	load_thread[0]->SetParent(this);
	load_thread3[0]     = new thread_load3();
	load_thread3[0]->SetParent(this);
	model_thread[0]     = new thread_model();
	model_thread[0]->SetParent(this);
	threshold_thread[0] = new thread_threshold();
	threshold_thread[0]->SetParent(this);
	change_thread[0]    = new thread_th_change();
	change_thread[0]->SetParent(this);

	//--------p value input----//
	//set p value input spinbox parameters
	this->ui->pValue_in->setValue(0.01);//p value default=0.01
	this->ui->pValue_in->setRange(0.00,0.10);//set p value vary range
	this->ui->pValue_in->setSingleStep(0.001); //set inc/dec step
	this->ui->pValue_in->setDecimals(3); //set precision

	//--------init condition select-----//
	QListWidget* list_widget_condition_item = new QListWidget;
	this->ui->conditionNameBox->setModel(list_widget_condition_item->model());
	this->ui->conditionNameBox->setView(list_widget_condition_item);
	this->ui->conditionNameBox->setEditable(true);
	this->ui->conditionNameBox->addItem((QWidget::tr("default")));
	this->ui->conditionNameBox->setEditText(tr("please select"));

	//--------init contrast vector select-------//
	QListWidget* list_widget_contrast_item = new QListWidget;
	this->ui->contrastNameBox->setModel(list_widget_contrast_item->model());
	this->ui->contrastNameBox->setView(list_widget_contrast_item);
	this->ui->contrastNameBox->setEditable(true);
	this->ui->contrastNameBox->addItem((QWidget::tr("default")));
	this->ui->contrastNameBox->setEditText(tr("please select"));

	this->advance_dialog = new Dialog_Paradigm_Advance;
}


void SubWidgetParadigmInBold::Design_plot()
{
	//--------put design matrix into QVector to display-------//
	QVector<double> X_axis(this->designMat->GetNumberOfTuples());
	vector< QVector<double> > Y_axis(this->designMat->GetNumberOfComponents());
	vector< QVector<double> > Y_axis_block(this->designMat->GetNumberOfTuples());
	for (int i=0;i<this->designMat->GetNumberOfTuples();i++)
	{
		X_axis[i]=double(i);
		for(int j=0;j<this->designMat->GetNumberOfComponents();j++)
		{
			Y_axis[j].push_back(double(this->designMat->GetComponent(i,j)));
			Y_axis_block[j].push_back(0);
		}
	}
	/*
	for(int i=0;i<this->onset.size();i++)
	{
		for (int j=0;j<this->onset[i].size();j++)
		{
			for (int k=this->onset[i][j];k<this->onset[i][j]+this->duration[i][j];k++)
			{
				Y_axis_block[i][k]=1;
			}
		}
	}*/
	//set up dialog
	QDialog* plot_dialog = new QDialog;
	plot_dialog->resize(600, 400);
	QScrollArea* scrollArea = new QScrollArea(plot_dialog);
	QCustomPlot* plot_graph = new QCustomPlot(scrollArea);
	//setup scroll area
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(plot_graph);
	scrollArea->setGeometry(QRect(20, 20, 580, 380));
	plot_graph->setObjectName(QString::fromUtf8("plot"));
	plot_dialog->setModal(false);

	// add two new graphs and set their look:
	plot_graph->addGraph();
	plot_graph->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
	plot_graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
	plot_graph->addGraph();
	plot_graph->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
	// configure right and top axis to show ticks but no labels:
	// (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
	plot_graph->xAxis2->setVisible(false);//	upper axis visible
	plot_graph->xAxis2->setTickLabels(false);//
	plot_graph->yAxis2->setVisible(false);//	right axis visible
	plot_graph->yAxis2->setTickLabels(false);//
	//set axis end arrow;
	plot_graph->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
	plot_graph->xAxis->setLabel("volume");
	plot_graph->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
	plot_graph->yAxis->setLabel("density");
	//plot_graph->xAxis->
	// make left and bottom axes always transfer their ranges to right and top axes:
	connect(plot_graph->xAxis, SIGNAL(rangeChanged(QCPRange)), plot_graph->xAxis2, SLOT(setRange(QCPRange)));
	connect(plot_graph->yAxis, SIGNAL(rangeChanged(QCPRange)), plot_graph->yAxis2, SLOT(setRange(QCPRange)));
	
	//----- pass data points to graphs: ----//
	//input QVector
	plot_graph->graph(0)->setData(X_axis, Y_axis_block[0]);
	plot_graph->graph(0)->setName("DESIGN");
	plot_graph->graph(1)->setData(X_axis, Y_axis[0]);
	plot_graph->graph(1)->setName("BOLD Model");

	// let the ranges scale themselves so graph 0 fits perfectly in the visible area:
	plot_graph->graph(0)->rescaleAxes();
	// same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
	plot_graph->graph(1)->rescaleAxes(true);
	// Note: we could have also just called customPlot->rescaleAxes(); instead
	// Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
	plot_graph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);


	plot_dialog->show();
}




int SubWidgetParadigmInBold::Bold_Visualize()
{
#ifndef QS_DEBUG_MODE
	/***************render here*************/
	//Visualize
	MainWindow* mainwnd = MainWindow::GetMainWindow();

	LayerBold* m_layerBold = new LayerBold();
	m_layerBold->SetUniqueName("activation area");
	m_layerBold->Create(this->ActArea);
	m_layerBold->SetColorRange(this->ActArea->GetScalarRange()[0],this->ActArea->GetScalarRange()[1]);
	m_layerBold->SetLookUpTable();

	LayerCollection* col_bold = mainwnd->GetLayerCollection( "Bold" );
	col_bold->AddLayer(m_layerBold);

	//Refresh Fiducial Actor
	RenderView* view_s = mainwnd->GetRenderView(0);
	RenderView* view_c = mainwnd->GetRenderView(1);
	RenderView* view_a = mainwnd->GetRenderView(2);
	RenderView* view_3d = mainwnd->GetRenderView(3);

	((RenderView2D*) view_s)->AppendLayerOnTop(m_layerBold);
	((RenderView2D*) view_c)->AppendLayerOnTop(m_layerBold);
	((RenderView2D*) view_a)->AppendLayerOnTop(m_layerBold);
	((RenderView3D*) view_3d)->AppendLayerOnTop(m_layerBold);
	/**************render here****************/
#endif
	return 0;
}


void SubWidgetParadigmInBold::on_click_advance()
{
	if (this->advance_dialog->exec()!=100)
		return;

	this->designMat = this->advance_dialog->designMat;
	this->contrast  = this->advance_dialog->contrast_vector;
}

void SubWidgetParadigmInBold::on_click_user_input()
{
	this->on_click_addcondition();
	this->on_click_addcontrast();

}



//Description: Read fmri design configure file
//
//Input:    Struct of paradigm parameters
//Return value: 
//				0:   Run function successfully
//				1:   paradigm instance is NULL
//				2:   File name invalid
//				3:   Configure file syntax error
int SubWidgetParadigmInBold::Read_Fmri_Configure(std::string filename, Fmri_paradigm* paradigm)
{
	//section head
	std::string Section_TR         = "TR";
	std::string Section_Conditions = "Conditions";
	std::string Section_Points     = "Time Points";
	std::string Section_Onset      = "Onset";
	std::string Section_Duration   = "Duration";
	std::string Section_Contrast   = "Contrast Vector"; 
	std::string Section_Pattern    = "Design Pattern";
	//section end
	std::string Section_TR_e         = "/TR";
	std::string Section_Conditions_e = "/Conditions";
	std::string Section_Points_e     = "/Time Points";
	std::string Section_Onset_e      = "/Onset";
	std::string Section_Duration_e   = "/Duration";
	std::string Section_Contrast_e   = "/Contrast Vector"; 
	std::string Section_Pattern_e    = "/Design Pattern";

	//valid paradigm instance
	if (paradigm == NULL)
	{
		std::cout<<"paradigm is NULL"<<std::endl;
		return 1;
	}

	//valid file name
	std::ifstream file(filename);
	if (!file)
	{
		std::cout<<"fmri configure file name is not invalid"<<std::endl;
		return 2;
	}
	else
	{
		std::string temp_line;
		int cnt_row = 0;

		while (std::getline(file,temp_line,'\n'))
		{
			//exclude comments first
			std::size_t comment_pos = temp_line.find_first_of("#");
			temp_line.resize(comment_pos);

			//invalid line
			if (temp_line.size()==0)
				continue;

			if ((temp_line.find(Section_TR)!=std::string::npos) &&
				(temp_line.find(Section_TR_e)==std::string::npos))
			{
begin_parse_section_TR:
				//read a new line
				std::getline(file,temp_line,'\n');
				//check end of file
				if (!file)
				{
					std::cout<<"Configure file format error"<<std::endl;
					return 3;
				}
				//reach end of a section?
				if(temp_line.find(Section_TR_e)!=std::string::npos)
					continue;

				//exclude empty characters
				int s = temp_line.find_first_not_of(" ");
				int e = temp_line.find_last_not_of(" ");
				temp_line = temp_line.substr(s,e-s+1);
				if (temp_line.empty())
					goto begin_parse_section_TR;

				//transform character to number
				std::string::size_type sz;
				paradigm->SetTR (std::stof(temp_line,&sz));
				//skip to next line in the section
				goto begin_parse_section_TR;
			}

			else if ((temp_line.find(Section_Points)!=std::string::npos) &&
				(temp_line.find(Section_Points_e)==std::string::npos))
			{
begin_parse_section_TP:
				//read a new line
				std::getline(file,temp_line,'\n');
				//check end of file
				if (!file)
				{
					std::cout<<"Configure file format error"<<std::endl;
					return 3;
				}
				//reach end of a section?
				if(temp_line.find(Section_Points_e)!=std::string::npos)
					continue;

				//exclude empty characters
				int s = temp_line.find_first_not_of(" ");
				int e = temp_line.find_last_not_of(" ");
				temp_line = temp_line.substr(s,e-s+1);
				if (temp_line.empty())
					goto begin_parse_section_TP;

				//transform character to number
				std::string::size_type sz;
				paradigm->SetNumVolume(std::stoi(temp_line,&sz));
				//skip to next line in the section
				goto begin_parse_section_TP;
			}

			else if ((temp_line.find(Section_Pattern)!=std::string::npos) &&
				(temp_line.find(Section_Pattern_e)==std::string::npos))
			{
begin_parse_section_Pattern:
				//read a new line
				std::getline(file,temp_line,'\n');
				//check end of file
				if (!file)
				{
					std::cout<<"Configure file format error"<<std::endl;
					return 3;
				}
				//reach end of a section?
				if(temp_line.find(Section_Pattern_e)!=std::string::npos)
					continue;

				//exclude empty characters
				int s = temp_line.find_first_not_of(" ");
				int e = temp_line.find_last_not_of(" ");
				temp_line = temp_line.substr(s,e-s+1);
				if (temp_line.empty())
					goto begin_parse_section_Pattern;

				//character
				paradigm->SetPattern(temp_line);

				//skip to next line in the section
				goto begin_parse_section_Pattern;
			}

			else if ((temp_line.find(Section_Conditions)!=std::string::npos) &&
				(temp_line.find(Section_Conditions_e)==std::string::npos))
			{
begin_parse_section_Con:
				//read a new line
				std::getline(file,temp_line,'\n');
				//check end of file
				if (!file)
				{
					std::cout<<"Configure file format error"<<std::endl;
					return 3;
				}
				//reach end of a section?
				if(temp_line.find(Section_Conditions_e)!=std::string::npos)
					continue;

				//exclude empty characters
				int s = temp_line.find_first_not_of(" ");
				int e = temp_line.find_last_not_of(" ");
				temp_line = temp_line.substr(s,e-s+1);
				if (temp_line.empty())
					goto begin_parse_section_Con;

				//transform character to number
				std::string::size_type sz;
				paradigm->SetNumCondition (std::stoi(temp_line,&sz));
				//Skip to next line in the section
				goto begin_parse_section_Con;
			}
			else if ((temp_line.find(Section_Onset)!=std::string::npos) &&
				(temp_line.find(Section_Onset_e)==std::string::npos))
			{
begin_parse_section_Onset:
				//read a new line
				std::getline(file,temp_line,'\n');
				//check end of file
				if (!file)
				{
					std::cout<<"Configure file format error"<<std::endl;
					return 3;
				}
				//reach end of a section?
				if(temp_line.find(Section_Onset_e)!=std::string::npos)
					continue;

				//exclude empty characters
				int s = temp_line.find_first_not_of(" ");
				int e = temp_line.find_last_not_of(" ");
				temp_line = temp_line.substr(s,e-s+1);
				if (temp_line.empty())
					goto begin_parse_section_Onset;

				//convert string to std vector


				//Skip to next section in the 
				goto begin_parse_section_Onset;
			}

			else if ((temp_line.find(Section_Duration)!=std::string::npos) &&
				(temp_line.find(Section_Duration_e)==std::string::npos))
			{
begin_parse_section_Duration:
				//read a new line
				std::getline(file,temp_line,'\n');
				//check end of file
				if (!file)
				{
					std::cout<<"Configure file format error"<<std::endl;
					return 3;
				}
				//reach end of a section?
				if(temp_line.find(Section_Duration_e)!=std::string::npos)
					continue;

				//exclude empty characters
				int s = temp_line.find_first_not_of(" ");
				int e = temp_line.find_last_not_of(" ");
				temp_line = temp_line.substr(s,e-s+1);
				if (temp_line.empty())
					goto begin_parse_section_Duration;

				//convert string to std vector


				//Skip to next section in the 
				goto begin_parse_section_Duration;
			}
			else if ((temp_line.find(Section_Contrast)!=std::string::npos) &&
				(temp_line.find(Section_Contrast_e)==std::string::npos))
			{
begin_parse_section_Contrast:
				//read a new line
				std::getline(file,temp_line,'\n');
				//check end of file
				if (!file)
				{
					std::cout<<"Configure file format error"<<std::endl;
					return 3;
				}
				//reach end of a section?
				if(temp_line.find(Section_Contrast_e)!=std::string::npos)
					continue;

				//exclude empty characters
				int s = temp_line.find_first_not_of(" ");
				int e = temp_line.find_last_not_of(" ");
				temp_line = temp_line.substr(s,e-s+1);
				if (temp_line.empty())
					goto begin_parse_section_Contrast;

				//convert string to std vector


				//Skip to next section in the 
				goto begin_parse_section_Contrast;
			}
		}
		return 0;
	}
}


//Description: Run brain strip process
//Input:       
//  			init_image: the original T2 image
//              brain:      itk pointer pointing to striped brain profile
//Return value:
//				0: brain strip process done successfully
//				1: atlas image read failed
//				2: strip filter failed
//				3: write mask to file failed
int SubWidgetParadigmInBold::BrainStrip(itk::Image<float,3>::Pointer init_image,itk::Image<unsigned char, 3>::Pointer brain)
{
	//image type for atlas 
	typedef itk::Image<short, 3>					AtlasImageType;
	typedef itk::Image<unsigned char, 3>			AtlasLabelType;
	typedef itk::Image< float , 3 >                 ImageTypex;
	//reader type: atlas
	typedef itk::ImageFileReader<AtlasImageType> AtlasReaderType;
	typedef itk::ImageFileReader<AtlasLabelType> LabelReaderType;


	std::cout<<"begin brain strip"<<std::endl;
	double startTime = time(NULL);
	AtlasReaderType::Pointer atlasReader = AtlasReaderType::New();
	LabelReaderType::Pointer labelReader = LabelReaderType::New();	
	atlasReader->SetFileName(BRAINSTRIP_SRC);
	labelReader->SetFileName(BRAINSTRIP_MASK);
	try
	{
		atlasReader->Update();
		labelReader->Update();
	}
	catch ( itk::ExceptionObject &exception )
	{
		std::cerr << "Brain Strip error: Cannot read template " << std::endl;
		std::cerr << exception << std::endl;
		return 1;
	}
	// set up skull-stripping filter
	typedef itk::StripTsImageFilter<ImageTypex, AtlasImageType, AtlasLabelType> StripTsFilterType;
	StripTsFilterType::Pointer stripTsFilter = StripTsFilterType::New();

	// set the required inputs for the stripTsImageFilter
	stripTsFilter->SetInput(init_image);          //input image
	stripTsFilter->SetAtlasImage( atlasReader->GetOutput() );
	stripTsFilter->SetAtlasBrainMask( labelReader->GetOutput() );
	try
	{
		std::cerr << "Try brain strip!! " << std::endl;
		stripTsFilter->Update();
	}
	catch ( itk::ExceptionObject &exception )
	{
		std::cerr << "brain strip error !! " << std::endl;
		std::cerr << "Exception caught ! " << std::endl;
		std::cerr << exception << std::endl;
		return 2;
	}
	//write mask for test
	typedef itk::ImageFileWriter  < AtlasLabelType > WriterType_mask;
	WriterType_mask::Pointer nii_writer_mask = WriterType_mask::New();
	brain = AtlasLabelType::New();
	brain = stripTsFilter->GetOutput();
	nii_writer_mask->SetInput( brain);
	nii_writer_mask->SetFileName("mask.nii");
	try
	{
		nii_writer_mask->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "error write mask";
		std::cerr << err;
		return 3;
	}
	double endTime = time(NULL);
	qDebug()<<"brain strip done";
	std::cout << "Total computation time: " << endTime-startTime << "seconds " << std::endl;
	return 0;
}




//----------tree widget-------------//
//OK = 100
//cancel = 101
Tree_Dialog::Tree_Dialog(QWidget *parent): QDialog(parent)
{
	if (this->objectName().isEmpty())
		this->setObjectName(QString::fromUtf8("Tree_Dialog"));
	this->resize(800, 400);
	this->setStyleSheet(QString::fromUtf8("background-color: rgb(37, 37, 38);"));
	verticalLayout = new QVBoxLayout(this);
	verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
	label = new QLabel(this);
	label->setObjectName(QString::fromUtf8("label"));
	QFont font;
	font.setFamily(QString::fromUtf8("Arial"));
	font.setPointSize(20);
	label->setFont(font);
	label->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
	label->setText("Please Select FMRI Series");

	verticalLayout->addWidget(label);

	index_tree = new QTreeWidget(this);
	index_tree->setObjectName(QString::fromUtf8("imageSeriesTreeWidget"));
	QFont font1;
	font1.setFamily(QString::fromUtf8("Arial"));
	font1.setStrikeOut(false);
	index_tree->setFont(font1);
	index_tree->setStyleSheet(QString::fromUtf8("QTreeWidget::item::text{color:white;}\n"
		"\n"
		"QTreeWidget::item {\n"
		"      border: 0.5px solid rgba(38,38,40,255);\n"
		"	  color::white;\n"
		" }\n"
		"\n"
		"QTreeWidget::item:selected {\n"
		"      background-color:rgb(0,102,224);\n"
		"	  color:white;\n"
		" }\n"
		"\n"
		"QTreeWidget::item:!selected {\n"
		"      background-color:rgb(38,38,40,255);\n"
		"	  color:white;\n"
		" }\n"
		"\n"
		"QHeaderView::section{ background-color:rgb(51,51,55);color:white;border-style:flat;padding:2px;font-family: Arial;font-style: normal;font-size: 9pt;}"));
	index_tree->setAlternatingRowColors(false);
	index_tree->setIndentation(20);
	index_tree->setSortingEnabled(false);
	index_tree->header()->setDefaultSectionSize(180);

	verticalLayout->addWidget(index_tree);

	gridLayout = new QGridLayout(this);
	gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
	loadBtn = new QPushButton(this);
	loadBtn->setObjectName(QString::fromUtf8("loadBtn"));
	QFont font2;
	font2.setFamily(QString::fromUtf8("Arial"));
	font2.setPointSize(16);
	loadBtn->setFont(font2);
	loadBtn->setCursor(QCursor(Qt::PointingHandCursor));
	loadBtn->setFocusPolicy(Qt::NoFocus);
	loadBtn->setLayoutDirection(Qt::LeftToRight);
	loadBtn->setText("Select");
	loadBtn->setStyleSheet(QString::fromUtf8("QPushButton { color: rgb(255, 255, 255);}\n"
		" \n"
		"QPushButton:pressed { color: rgb(255, 255, 255);border: none;margin: 0px;padding: 0px;background-color:rgb(0,102,224);}\n"
		"\n"
		"QPushButton:hover:!pressed { color: rgb(255, 255, 255);border: none;margin: 0px;padding: 0px;background-color:rgb(63,63,70);}\n"
		"\n"
		" QPushButton:hover { color:rgb(255, 255, 255);}"));
	loadBtn->setFlat(true);

	gridLayout->addWidget(loadBtn, 0, 0, 1, 1);

	cancelBtn = new QPushButton();
	cancelBtn->setObjectName(QString::fromUtf8("cancelBtn"));
	cancelBtn->setFont(font2);
	cancelBtn->setCursor(QCursor(Qt::PointingHandCursor));
	cancelBtn->setFocusPolicy(Qt::NoFocus);
	cancelBtn->setLayoutDirection(Qt::LeftToRight);
	cancelBtn->setText("Cancel");
	cancelBtn->setStyleSheet(QString::fromUtf8("QPushButton { color: rgb(255, 255, 255);}\n"
		" \n"
		"QPushButton:pressed { color: rgb(255, 255, 255);border: none;margin: 0px;padding: 0px;background-color:rgb(0,0,0);}\n"
		"\n"
		"QPushButton:hover:!pressed { color: rgb(255, 255, 255);border: none;margin: 0px;padding: 0px;background-color:rgb(63,63,70);}\n"
		"\n"
		" QPushButton:hover { color:rgb(255, 255, 255);}"));
	cancelBtn->setFlat(true);

	gridLayout->addWidget(cancelBtn, 0, 1, 1, 1);
	verticalLayout->addLayout(gridLayout);


	QStringList head;
	head<<"Name"<<"Volume";
	index_tree->setHeaderLabels(head);

	connect(this->loadBtn,SIGNAL(clicked()),this,SLOT(on_click_load()));
	connect(this->cancelBtn,SIGNAL(clicked()),this,SLOT(on_click_cancel()));
	connect(this->index_tree,SIGNAL(itemClicked(QTreeWidgetItem *, int)),this,SLOT(on_click_item(QTreeWidgetItem*,int)));
}

Tree_Dialog::~Tree_Dialog()
{
}

void Tree_Dialog::on_click_item(QTreeWidgetItem* item,int column)
{
	int col = this->index_tree->indexOfTopLevelItem(item);
	qDebug()<<"selected col:"<<col;
	*this->index = col;
}

void Tree_Dialog::on_click_load()
{
	this->done(100);
	this->hide();
}
void Tree_Dialog::on_click_cancel()
{
	this->done(101);
	this->hide();
}

void Tree_Dialog::SetInfo(File_info* in_info, int * select_index)
{
	infox = in_info;
	index = select_index;
	if(infox->description.size() == 0)
	{
		QStringList item_list;
		item_list<<"nothing"<<"nothing";
		QTreeWidgetItem * child = new QTreeWidgetItem(index_tree,item_list);
		QMessageBox::about(NULL, "ERROR", "read log failed!");
		this->close();
	}
	else
	{
		for (int i = 0;i<infox->name.size();i++)
		{
			QStringList item_list;
			item_list<<infox->description.at(i)<<infox->temperal_number.at(i);
			QTreeWidgetItem* child = new QTreeWidgetItem(index_tree,item_list);
			index_tree->addTopLevelItem(child);
		}
	}

}








