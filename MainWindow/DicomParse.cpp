
#include "DicomParse.h"


void Seek_Dicom_Folder(QString m_strDir)
{
	//m_strDir = QFileDialog::getExistingDirectory(this, tr("Open Image Directory"),
	//	"C:/Users/user/Desktop",QFileDialog::ShowDirsOnly);

	if (m_strDir=="")
		return;

	//Clear DICOMSortList
	QFile dicomInfo(m_strDir+ "/DicomSortList.txt");
	dicomInfo.remove();

	QStringList strlstAllDir;
	strlstAllDir.append(m_strDir);

	//Scan all the folder in the selected directory
	QDirIterator it(m_strDir, QDir::Dirs|QDir::NoDotAndDotDot,QDirIterator::Subdirectories);

	while (it.hasNext())
	{
		strlstAllDir.append(it.next());
	}


	//progress bar
	QProgressDialog* dialogLoading_load_file = new QProgressDialog("Parse....", "", 0, strlstAllDir.size());
	dialogLoading_load_file->setWindowModality(Qt::WindowModal);
	dialogLoading_load_file->setCancelButton(0);
	QLabel* label_temp_lf = new QLabel (dialogLoading_load_file);
	label_temp_lf->setStyleSheet("color: rgb(255, 255, 255);");
	dialogLoading_load_file->setLabel(label_temp_lf);
	dialogLoading_load_file->setMinimumDuration(0);
	dialogLoading_load_file->setLabelText("Loading files..");
	dialogLoading_load_file->setValue(1);

	for (int i=0;i<strlstAllDir.size();i++)
	{
		ParseParseParse(strlstAllDir.at(i),m_strDir);
		dialogLoading_load_file->setValue(i);
	}
	delete dialogLoading_load_file;
}

// first para: path      ---- single dicom file full path
// second para: m_strDir ---- dicom file folder path
void ParseParseParse(QString path, QString m_strDir)
{
	//Example:DicomSeriesReadPrintTags, DicomSeriesReadImageWrite2

	//Sort the series in the directory (Reading multiple series in one directory is possible)
	NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
	nameGenerator->SetUseSeriesDetails( true );
	nameGenerator->GlobalWarningDisplayOff();
	//nameGenerator->AddSeriesRestriction("0018|0024" ); //Sequence Name  **important
	//nameGenerator->AddSeriesRestriction("0008|103E" ); //Series Description  **important
	nameGenerator->SetDirectory(path.toStdString());

	typedef std::vector< std::string >    SeriesIdContainer; //should be sequence Itr

	const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();

	SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
	SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();
	while( seriesItr != seriesEnd )
	{
		ReaderType::Pointer reader = ReaderType::New();
		ImageIOType::Pointer dicomIO = ImageIOType::New();
		reader->SetImageIO( dicomIO );

		typedef std::vector<std::string>    FileNamesContainer;
		FileNamesContainer fileNames = nameGenerator->GetFileNames(seriesItr->c_str());

		reader->SetFileNames( fileNames );
		try
		{	
			reader->Update();
		}
		catch (itk::ExceptionObject& e)
		{
			std::cerr<<e;
			std::cerr<<"ITK reader fiaild... Returning!"<<std::endl;
			return;
		}

		typedef itk::MetaDataDictionary   DictionaryType;

		const  DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();

		DictionaryType::ConstIterator itr = dictionary.Begin();
		DictionaryType::ConstIterator end = dictionary.End();

		typedef itk::MetaDataObject< std::string > MetaDataStringType;

		//Patient Name
		std::string patientNameTag = "0010|0010";
		std::string patientName = "/";

		DictionaryType::ConstIterator patientNameTagItr = dictionary.Find(patientNameTag);

		if(patientNameTagItr!=end)
		{
			MetaDataStringType::ConstPointer patientNameEntryvalue =
				dynamic_cast<const MetaDataStringType *>( patientNameTagItr->second.GetPointer() );

			patientName = patientNameEntryvalue->GetMetaDataObjectValue();	
		}

		//Date
		std::string dateTag = "0008|0020";
		std::string date = "/";

		DictionaryType::ConstIterator dateTagItr = dictionary.Find(dateTag);
		if(dateTagItr!=end)
		{
			MetaDataStringType::ConstPointer dateEntryvalue =
				dynamic_cast<const MetaDataStringType *>( dateTagItr->second.GetPointer() );

			date = dateEntryvalue->GetMetaDataObjectValue();
		}

		date = date.insert(4,"/");
		date = date.insert(7,"/");

		//Modality
		std::string modlaityTag = "0008|0060";
		std::string modality = "/";

		DictionaryType::ConstIterator modalityTagItr = dictionary.Find(modlaityTag);
		if(modalityTagItr!=end)
		{
			MetaDataStringType::ConstPointer modalityEntryvalue =
				dynamic_cast<const MetaDataStringType *>( modalityTagItr->second.GetPointer() );

			modality = modalityEntryvalue->GetMetaDataObjectValue();
		}

		//Sequence Description
		std::string seriesDescriptionTag = "0008|103e";
		std::string seriesDescription = "/";

		DictionaryType::ConstIterator seriesDescriptionTagItr = dictionary.Find(seriesDescriptionTag);

		if(seriesDescriptionTagItr!=end)
		{
			MetaDataStringType::ConstPointer seriesDescriptionEntryvalue =
				dynamic_cast<const MetaDataStringType *>( seriesDescriptionTagItr->second.GetPointer() );

			seriesDescription = seriesDescriptionEntryvalue->GetMetaDataObjectValue();
		}


		//Series Number                 0020|0011
		//Acquisition Number            0020|0012
		//Temporal Position Identifier  0020|0100
		//Instance Number
		std::string InstanceNumberTag = "0020|0013";
		std::vector<std::string> InstanceNumber ;
		std::map<int,std::string> fileNamesOrderMap;
		DictionaryType::ConstIterator InstanceNumberTagItr = dictionary.Find(InstanceNumberTag);
		if (InstanceNumberTagItr!=end)
		{
			MetaDataStringType::ConstPointer InstanceNumbervalue =
				dynamic_cast<const MetaDataStringType *>( InstanceNumberTagItr->second.GetPointer() );

			InstanceNumber .push_back( InstanceNumbervalue->GetMetaDataObjectValue() );
		}


		//Sequence Name
		std::string numberOfTemporalPositionsTag = "0020|0105";
		std::string numberOfTemporalPositions = "/";

		DictionaryType::ConstIterator numberOfTemporalPositionsTagItr = dictionary.Find(numberOfTemporalPositionsTag);

		if(numberOfTemporalPositionsTagItr!=end)
		{
			MetaDataStringType::ConstPointer numberOfTemporalPositionsEntryvalue =
				dynamic_cast<const MetaDataStringType *>( numberOfTemporalPositionsTagItr->second.GetPointer() );

			numberOfTemporalPositions = numberOfTemporalPositionsEntryvalue->GetMetaDataObjectValue();
		}

		QFile dicomInfo(m_strDir+ "/DicomSortList.txt");
		if (dicomInfo.open(QIODevice::Append)) 
		{
			QTextStream out(&dicomInfo);
			out << "<Patient_Name>" <<QString::fromStdString(patientName)<<"</Patient_Name>"<<endl;
			out << "<Date>" <<QString::fromStdString(date)<<"</Date>"<<endl;
			out << "<Modailty>" <<QString::fromStdString(modality)<<"</Modality>"<<endl;
			out << "<Slice_Number>" <<QString::number(fileNames.size())<<"</Slice_Number>"<<endl;
			out << "<Series_Description>" <<QString::fromStdString(seriesDescription)<<"</Series_Description>"<<endl;
			out << "<Series_UID>" <<seriesItr->c_str()<<"</Series_UID>"<<endl;
			out << "<Format>" <<"DICOM"<<"</Format>"<<endl;
			out << "<Path>" <<endl;
			for (int i=0;i<fileNames.size();i++)
			{
				out	<<QString::fromStdString(fileNames.at(i))<<endl;//<<" - "<<QString::fromStdString(InstanceNumber.at(i))
			}
			out	<<"</Path>"<<endl;
			out << "<NumberOfTemporalPositions>"<<QString::fromStdString(numberOfTemporalPositions) << "</NumberOfTemporalPositions>"<<endl<<endl;
		}

		++seriesItr;
	}
}

//parse log file
void Load_File_from_log(QString log_name,
						std::vector<vtkSmartPointer<vtkImageData> >& container,
						std::vector<std::string>& img_names)
{

	QString log = log_name;
	QFile log_file;
	File_info_in_DicomParse* info = new File_info_in_DicomParse;
	//progress bar
	QProgressDialog* dialogLoading_load_file = new QProgressDialog("Load Files", "", 0, 3);
	dialogLoading_load_file->setWindowModality(Qt::WindowModal);
	dialogLoading_load_file->setCancelButton(0);
	QLabel* label_temp_lf = new QLabel (dialogLoading_load_file);
	label_temp_lf->setStyleSheet("color: rgb(255, 255, 255);");
	dialogLoading_load_file->setLabel(label_temp_lf);
	dialogLoading_load_file->setMinimumDuration(0);
	dialogLoading_load_file->setLabelText("Loading files..");
	dialogLoading_load_file->setValue(1);

	//clear data_container
	if(!container.empty())
		container.clear();

	//-----single .dcm files load from log file------//
	std::cout<<"begin parse log file !"<<std::endl;

	if (log.isEmpty())
	{
		delete dialogLoading_load_file;
		return;
	}

	std::cout<<log.toStdString()<<std::endl;
	//create file handle
	log_file.setFileName(log);
	//read file line by line
	if(!log_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		delete dialogLoading_load_file;
		return;
	}

	//use text stream to read line
	QTextStream in(&log_file);	
	//get col number
	//int col_num = ui->in_col->text().toInt();
	QString line1;
	qint64 line_number=0;

	dialogLoading_load_file->setValue(2);

	//clear File_info
	if (info != NULL)
	{
		delete info;
	}
	info = new File_info_in_DicomParse;

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
			info->name.append(name_2);
		}
		if(head == "Series_Description")
		{
			info->description.append(name_2);
		}
		if(head == "Slice_Number")
		{
			info->slice_number.append(name_2);
		}
		if(head == "NumberOfTemporalPositions")
		{
			info->temperal_number.append(name_2);
		}		
		if(head == "Path")
		{
			info->path_position<<in.pos();
		}
	}
	std::cout<<"parse log done !"<<std::endl;
	//close file
	log_file.close();

	dialogLoading_load_file->setValue(3);
	delete dialogLoading_load_file;

	//valid log file
	if(info->description.empty())
	{
		QMessageBox::information(NULL,"ERROR","The log file is invalid!",
			QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
		return;
	}

	int total_vol = info->description.size();

	for (int select_index = 0;select_index<total_vol;select_index++)
	{
		////parse from log and add data to container
		//if (info->description[select_index].contains("pd") && info->description[select_index].contains("t2"))
		//{
		//	if (info->description[select_index].contains("Stereo"))
		//	{
		//		Log2Container_inDicomParse_Stereo(log_name,info,container,select_index,img_names);
		//	}
		//	else
		//	{
		//		Log2Container_inDicomParse_Exception(log_name,info,container,select_index,img_names);
		//	}
		//}
		//else if (info->description[select_index].contains("Stereo"))
		//{
		//	Log2Container_inDicomParse_Stereo_Only(log_name,info,container,select_index,img_names);
		//}
		//else
		//{
		//	Log2Container_inDicomParse(log_name,info,container,select_index,img_names);
		//}
		Log2Container_inDicomParse(log_name,info,container,select_index,img_names);
	}

	std::cout<<"load files successfully!!"<<std::endl;
}

void Log2Container_inDicomParse(QString log_name,File_info_in_DicomParse* info, 
								std::vector<vtkSmartPointer<vtkImageData> >& container,
								int index, 
								std::vector<std::string>& file_names)
{
	int size = 0;
	int slice_number = 0;
	int volume_number= 0;
	//find number of item in QStringList
	size = info->description.size();
	slice_number = info->slice_number.at(index).toInt();
	volume_number= info->temperal_number.at(index).toInt();

	std::cout<<"volume info:"
		<<info->description.at(index).toStdString()
		<<info->slice_number.at(index).toInt()
		<<info->temperal_number.at(index).toInt()
		<<std::endl;
	
	QFile log_file;
	log_file.setFileName(log_name);
	//--read log again to find file path--//
	//read file line by line
	if(!log_file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	//use text stream to read line
	QTextStream in(&log_file);
	//seek to path
	in.seek(info->path_position.at(index));
	//1. put name into a vector
	std::map<int,std::string> name_to_sort_all;
	int cnt = 0;
	if (volume_number == 0)
	{
		while(cnt<slice_number)
		{
			QString f_name = in.readLine();
			name_to_sort_all[Parse_GetDicomTag_InstanceNumber(f_name.toStdString())] = f_name.toStdString();
			cnt++;
		}
	}
	else
	{
		while(cnt<slice_number-(slice_number%volume_number))
		{
			QString f_name = in.readLine();
			name_to_sort_all[Parse_GetDicomTag_InstanceNumber(f_name.toStdString())] = f_name.toStdString();
			cnt++;
		}
	}
	log_file.close();//read DicomSortLists.txt done
	if (name_to_sort_all.empty())
	{
		return;
	}
	//2. put name_to_sort by order
	int volume_number_temp;
	if (volume_number == 0)
	{
		volume_number_temp = 1;
	}
	else
	{
		volume_number_temp = volume_number;
	}
	std::vector< std::vector<std::string> > name_holder(volume_number_temp);//(volume_number);//QString to hold file names
	int cnt2 = 0;
	for (std::map<int,std::string>::iterator it = name_to_sort_all.begin();
		it!=name_to_sort_all.end();++it)
	{
		if (volume_number == 0)
		{
			if (cnt2<slice_number)
			{
				//int xx = cnt2;
				std::string temp = (*it).second;
				name_holder[0].push_back(temp);
				cnt2++;
			}
		}
		else if (cnt2<slice_number-(slice_number%volume_number))
		{
			int xx = cnt2%volume_number;
			std::string temp = (*it).second;
			name_holder[xx].push_back(temp);
			cnt2++;
		}
		else
			continue;
	}

	//to find slice number and volume number match or not
	if(name_holder.begin()->size()!=name_holder.back().size())
	{
		QMessageBox::about(NULL, "ERROR", "slice and volume do not match");
		return;
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
	//if(!container.empty());
	//	container.clear();


	//progress bar
	QProgressDialog* dialogLoading_File = new QProgressDialog("Copying files...", "", 0, volume_number);
	dialogLoading_File->setWindowModality(Qt::WindowModal);
	dialogLoading_File->setCancelButton(0);
	QLabel* label_temp = new QLabel (dialogLoading_File);
	label_temp->setStyleSheet("color: rgb(255, 255, 255);");
	dialogLoading_File->setLabel(label_temp);
	dialogLoading_File->setMinimumDuration(0);
	dialogLoading_File->setValue(0);
	dialogLoading_File->setLabelText("Reading fMRI Images!");
	dialogLoading_File->setValue(1);


	for(int i = 0;i<volume_number_temp;i++)
	{
		if (name_holder[i].empty())
		{
			return;
		}
		itk_reader->SetFileNames(name_holder[i]);
		try
		{
			itk_reader->Update();
		}
		catch (itk::ExceptionObject& e)
		{
			std::cerr<<e;
			return;
		}
		////save .nii files for test
		//WriterType_b::Pointer nii_writer_parse = 
		//	WriterType_b::New();
		//nii_writer_parse->SetInput(itk_reader->GetOutput());
		//std::string name1 = info->description.at(index).toStdString();
		//if (name1.compare(name1.size()-4,4,".nii") == 0)
		//{}//do nothing
		//else
		//{
		//	name1.append(".nii");
		//}
		//nii_writer_parse->SetFileName(name1);
		////nii_writer_parse->Update();
		//nii_writer_parse->Write();

		//get information from dicom tags( thickness among slices)
		const  DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();
		DictionaryType::ConstIterator itr = dictionary.Begin();
		DictionaryType::ConstIterator end = dictionary.End();

		std::string sliceThicknessID = "0018|0050";
		std::string sliceOriginID    = "0020|0032";
		std::string pixelSpacingID   = "0028|0030";
		DictionaryType::ConstIterator Thickness_tagItr   = dictionary.Find( sliceThicknessID );
		DictionaryType::ConstIterator Origin_tagItr      = dictionary.Find( sliceOriginID );
		DictionaryType::ConstIterator PixeSpacing_tagItr = dictionary.Find(pixelSpacingID);

		if( Thickness_tagItr == end | Origin_tagItr == end | PixeSpacing_tagItr == end)
		{
			std::cerr << "Tag " << sliceThicknessID<<"or"<<"sliceOriginID";
			std::cerr << " not found in the DICOM header" << std::endl;
			return;
		}
		MetaDataStringType::ConstPointer Thickness_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Thickness_tagItr->second.GetPointer() );
		MetaDataStringType::ConstPointer Origin_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Origin_tagItr->second.GetPointer() );
		MetaDataStringType::ConstPointer Spacing_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( PixeSpacing_tagItr->second.GetPointer() );


		std::string Thickness_tagvalue;
		std::string Origin_tagvalue;
		std::string PixelSpacing_tagvalue;

		if( Thickness_entryvalue || Origin_entryvalue)
		{
			Thickness_tagvalue    = Thickness_entryvalue->GetMetaDataObjectValue();
			Origin_tagvalue       = Origin_entryvalue   ->GetMetaDataObjectValue();
			PixelSpacing_tagvalue = Spacing_entryvalue  ->GetMetaDataObjectValue();
			std::cout << "slice thickness is (" << sliceThicknessID <<  ") ";
			std::cout << " is: " << Thickness_tagvalue << std::endl;
			std::cout << "origin is (" << sliceOriginID <<  ") ";
			std::cout << " is: " << Origin_tagvalue << std::endl;
			std::cout << "spacing is (" << pixelSpacingID <<  ") ";
			std::cout << " is: " << PixelSpacing_tagvalue << std::endl;
		}
		else
		{
			std::cerr << "Entry was not of string type" << std::endl;
			return ;
		}
		double slice_thickness = std::stod(Thickness_tagvalue);
		double pixel_spacing   = std::stod(PixelSpacing_tagvalue);
		double img_origin      = std::stod(Origin_tagvalue);

		//connect
		connector->SetInput(itk_reader->GetOutput());
		connector->Update();
		//output vtkimagedata to data_container
		//vtksmartpointer act as buff
		vtkSmartPointer<vtkImageData> buff =
			vtkSmartPointer<vtkImageData>::New();
		buff = connector->GetOutput();
		std::cout<<"converter: "<<i<<" done!"<<std::endl;

		//-----------change information(origin and spacing)--------------//
		//get information :origin
		//it seems that there is no need to change origin
		double origin[3]={0,0,0};
		buff->GetOrigin(origin);
		std::cout<<"origin:"<<origin[0]<<origin[1]<<origin[2]<<std::endl;

		double spacing[3]={0,0,0};
		buff->GetSpacing(spacing);
		spacing[2] = slice_thickness;

		vtkSmartPointer<vtkImageChangeInformation> changer = 
			vtkSmartPointer<vtkImageChangeInformation>::New();
		changer->SetOutputSpacing(spacing);
		//change 
		changer->SetInput(buff);
		//changer->SetOutputOrigin(origin);
		changer->Update();

		vtkSmartPointer<vtkImageData>temp_con_con = vtkSmartPointer<vtkImageData>::New();
		temp_con_con->DeepCopy(changer->GetOutput());
		container.push_back(temp_con_con);
		
		std::string filename_temp = info->description.at(index).toStdString();
		//exclude '/' from name
		if (filename_temp.find("/")!=std::string::npos)
		{
			filename_temp.replace(filename_temp.find("/"),1,"-");
		}
		file_names.push_back(filename_temp);
		
		//change process bar
		dialogLoading_File->setValue(i);
		// if cancel read image, stop and clear all
		if (dialogLoading_File->wasCanceled())
		{
			//close file first
			log_file.close();
			//clear data container 
			container.clear();
			//delete 
			break;
		}
	}
	if (log_file.isOpen())
	{
		log_file.close();
	}
	//delete progress dialog
	delete dialogLoading_File;
}


//split a volume into 2 volumes
//because they whole volume contain pd+t2 images
//if normal t2+pd image, re-construct them in ordinary order
void Log2Container_inDicomParse_Exception(QString log_name, File_info_in_DicomParse* info, std::vector<vtkSmartPointer<vtkImageData> >& container, int index, std::vector<std::string>& file_names)
{
	int size = 0;
	int slice_number = 0;
	int volume_number= 0;
	//find number of item in QStringList
	size = info->description.size();
	slice_number = info->slice_number.at(index).toInt();
	volume_number= info->temperal_number.at(index).toInt();

	std::cout<<"volume info:"
		<<info->description.at(index).toStdString()
		<<info->slice_number.at(index).toInt()
		<<info->temperal_number.at(index).toInt()
		<<std::endl;

	QFile log_file;
	log_file.setFileName(log_name);
	//--read log again to find file path--//
	//read file line by line
	if(!log_file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	//use text stream to read line
	QTextStream in(&log_file);
	//seek to path
	in.seek(info->path_position.at(index));
	//1. put name into a vector
	//divide these filenames by half
	std::vector< std::vector<std::string> > name_holder(2);
	int cnt = 0;
	while(cnt<int(slice_number/2))
	{
		QString f_name = in.readLine();
		name_holder[0].push_back(f_name.toStdString());
		cnt++;
	}
	while(cnt<slice_number)
	{
		QString f_name = in.readLine();
		name_holder[1].push_back(f_name.toStdString());
		cnt++;
	}

	log_file.close();//read DicomSortLists.txt done
	
	if ( name_holder[1].empty() && name_holder[1].empty() )
	{
		return;
	}


	//define itk reader
	//typedef itk::Image< float , 3 >             ImageType; //image type pixel:float;dimension:3
	//typedef itk::ImageSeriesReader< ImageType > ReaderType;
	ReaderType_b::Pointer itk_reader = ReaderType_b::New();
	DicomIOType::Pointer  dicomIO = DicomIOType::New();
	itk_reader->SetImageIO(dicomIO);


	for(int i = 0;i<2;i++)
	{
		if (name_holder[i].empty())
		{
			return;
		}
		itk_reader->SetFileNames(name_holder[i]);
		try
		{
			itk_reader->Update();
		}
		catch (itk::ExceptionObject& e)
		{
			std::cerr<<e;
			return;
		}

		//get information from dicom tags( thickness among slices)
		const  DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();
		DictionaryType::ConstIterator itr = dictionary.Begin();
		DictionaryType::ConstIterator end = dictionary.End();

		std::string sliceThicknessID = "0018|0050";
		std::string sliceOriginID    = "0020|0032";
		std::string pixelSpacingID   = "0028|0030";
		DictionaryType::ConstIterator Thickness_tagItr   = dictionary.Find( sliceThicknessID );
		DictionaryType::ConstIterator Origin_tagItr      = dictionary.Find( sliceOriginID );
		DictionaryType::ConstIterator PixeSpacing_tagItr = dictionary.Find(pixelSpacingID);

		if( Thickness_tagItr == end | Origin_tagItr == end | PixeSpacing_tagItr == end)
		{
			std::cerr << "Tag " << sliceThicknessID<<"or"<<"sliceOriginID";
			std::cerr << " not found in the DICOM header" << std::endl;
			return;
		}
		MetaDataStringType::ConstPointer Thickness_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Thickness_tagItr->second.GetPointer() );
		MetaDataStringType::ConstPointer Origin_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Origin_tagItr->second.GetPointer() );
		MetaDataStringType::ConstPointer Spacing_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( PixeSpacing_tagItr->second.GetPointer() );


		std::string Thickness_tagvalue;
		std::string Origin_tagvalue;
		std::string PixelSpacing_tagvalue;

		if( Thickness_entryvalue || Origin_entryvalue)
		{
			Thickness_tagvalue    = Thickness_entryvalue->GetMetaDataObjectValue();
			Origin_tagvalue       = Origin_entryvalue   ->GetMetaDataObjectValue();
			PixelSpacing_tagvalue = Spacing_entryvalue  ->GetMetaDataObjectValue();
			std::cout << "slice thickness is (" << sliceThicknessID <<  ") ";
			std::cout << " is: " << Thickness_tagvalue << std::endl;
			std::cout << "origin is (" << sliceOriginID <<  ") ";
			std::cout << " is: " << Origin_tagvalue << std::endl;
			std::cout << "spacing is (" << pixelSpacingID <<  ") ";
			std::cout << " is: " << PixelSpacing_tagvalue << std::endl;
		}
		else
		{
			std::cerr << "Entry was not of string type" << std::endl;
			return ;
		}
		double slice_thickness = std::stod(Thickness_tagvalue);
		double pixel_spacing   = std::stod(PixelSpacing_tagvalue);
		double img_origin      = std::stod(Origin_tagvalue);

		//typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
		i2vConnectorType::Pointer connector = i2vConnectorType::New();
		//connect
		connector->SetInput(itk_reader->GetOutput());
		connector->Update();
		//output vtkimagedata to data_container
		//vtksmartpointer act as buff
		vtkSmartPointer<vtkImageData> buff =
			vtkSmartPointer<vtkImageData>::New();
		buff = connector->GetOutput();
		std::cout<<"converter: "<<i<<" done!"<<std::endl;

		//-----------change information(origin and spacing)--------------//
		//get information :origin
		//it seems that there is no need to change origin
		double origin[3]={0,0,0};
		buff->GetOrigin(origin);
		std::cout<<"origin:"<<origin[0]<<origin[1]<<origin[2]<<std::endl;

		double spacing[3]={0,0,0};
		buff->GetSpacing(spacing);
		spacing[2] = slice_thickness;

		vtkSmartPointer<vtkImageChangeInformation> changer = 
			vtkSmartPointer<vtkImageChangeInformation>::New();
		changer->SetOutputSpacing(spacing);
		//change 
		changer->SetInput(buff);
		//changer->SetOutputOrigin(origin);
		changer->Update();

		vtkSmartPointer<vtkImageData>temp_con_con = vtkSmartPointer<vtkImageData>::New();
		temp_con_con->DeepCopy(changer->GetOutput());
		container.push_back(temp_con_con);
		if (i == 0)
		{
			std::string filename_temp = info->description.at(index).toStdString();
			if (filename_temp.find("/")!=std::string::npos)
			{
				filename_temp.replace(filename_temp.find("/"),1,"-");
			}
			file_names.push_back(filename_temp.append("-pd"));
		}
		else
		{
			std::string filename_temp = info->description.at(index).toStdString();
			if (filename_temp.find("/")!=std::string::npos)
			{
				filename_temp.replace(filename_temp.find("/"),1,"-");
			}
			file_names.push_back(filename_temp.append("-t2"));
		}
	}
	if (log_file.isOpen())
	{
		log_file.close();
	}

}

//split a volume into 2 volumes
//because they whole volume contain pd+t2 images
//if it is Stereo volume!!! Do re-construct slices in reverse order!!! God!!!
void Log2Container_inDicomParse_Stereo(QString log_name, File_info_in_DicomParse* info, std::vector<vtkSmartPointer<vtkImageData> >& container, int index, std::vector<std::string>& file_names)
{
	int size = 0;
	int slice_number = 0;
	int volume_number= 0;
	//find number of item in QStringList
	size = info->description.size();
	slice_number = info->slice_number.at(index).toInt();
	volume_number= info->temperal_number.at(index).toInt();

	std::cout<<"volume info:"
		<<info->description.at(index).toStdString()
		<<info->slice_number.at(index).toInt()
		<<info->temperal_number.at(index).toInt()
		<<std::endl;

	QFile log_file;
	log_file.setFileName(log_name);
	//--read log again to find file path--//
	//read file line by line
	if(!log_file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	//use text stream to read line
	QTextStream in(&log_file);
	//seek to path
	in.seek(info->path_position.at(index));
	//1. put name into a vector
	//divide these filenames by half
	std::vector< std::vector<std::string> > name_holder(2);
	int cnt = 0;
	while(cnt<int(slice_number/2))
	{
		QString f_name = in.readLine();
		name_holder[0].push_back(f_name.toStdString());
		cnt++;
	}
	while(cnt<slice_number)
	{
		QString f_name = in.readLine();
		name_holder[1].push_back(f_name.toStdString());
		cnt++;
	}

	//reverse the name holder
	std::vector< std::vector<std::string> > name_holder2(2);
	int size_name_holder_0 = name_holder[0].size();
	for (int x=0;x<size_name_holder_0;x++)
	{
		name_holder2[0].push_back(name_holder[0].back());
		name_holder[0].pop_back();
	}
	int size_name_holder_1 = name_holder[1].size();
	for (int x=0;x<size_name_holder_1;x++)
	{
		name_holder2[1].push_back(name_holder[1].back());
		name_holder[1].pop_back();
	}

	log_file.close();//read DicomSortLists.txt done

	if ( name_holder2[1].empty() && name_holder2[1].empty() )
	{
		return;
	}


	//define itk reader
	//typedef itk::Image< float , 3 >             ImageType; //image type pixel:float;dimension:3
	//typedef itk::ImageSeriesReader< ImageType > ReaderType;
	ReaderType_b::Pointer itk_reader = ReaderType_b::New();
	DicomIOType::Pointer  dicomIO = DicomIOType::New();
	itk_reader->SetImageIO(dicomIO);


	for(int i = 0;i<2;i++)
	{
		if (name_holder2[i].empty())
		{
			return;
		}
		itk_reader->SetFileNames(name_holder2[i]);
		try
		{
			itk_reader->Update();
		}
		catch (itk::ExceptionObject& e)
		{
			std::cerr<<e;
			return;
		}

		//get information from dicom tags( thickness among slices)
		const  DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();
		DictionaryType::ConstIterator itr = dictionary.Begin();
		DictionaryType::ConstIterator end = dictionary.End();

		std::string sliceThicknessID = "0018|0050";
		std::string sliceOriginID    = "0020|0032";
		std::string pixelSpacingID   = "0028|0030";
		DictionaryType::ConstIterator Thickness_tagItr   = dictionary.Find( sliceThicknessID );
		DictionaryType::ConstIterator Origin_tagItr      = dictionary.Find( sliceOriginID );
		DictionaryType::ConstIterator PixeSpacing_tagItr = dictionary.Find(pixelSpacingID);

		if( Thickness_tagItr == end | Origin_tagItr == end | PixeSpacing_tagItr == end)
		{
			std::cerr << "Tag " << sliceThicknessID<<"or"<<"sliceOriginID";
			std::cerr << " not found in the DICOM header" << std::endl;
			return;
		}
		MetaDataStringType::ConstPointer Thickness_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Thickness_tagItr->second.GetPointer() );
		MetaDataStringType::ConstPointer Origin_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Origin_tagItr->second.GetPointer() );
		MetaDataStringType::ConstPointer Spacing_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( PixeSpacing_tagItr->second.GetPointer() );


		std::string Thickness_tagvalue;
		std::string Origin_tagvalue;
		std::string PixelSpacing_tagvalue;

		if( Thickness_entryvalue || Origin_entryvalue)
		{
			Thickness_tagvalue    = Thickness_entryvalue->GetMetaDataObjectValue();
			Origin_tagvalue       = Origin_entryvalue   ->GetMetaDataObjectValue();
			PixelSpacing_tagvalue = Spacing_entryvalue  ->GetMetaDataObjectValue();
			std::cout << "slice thickness is (" << sliceThicknessID <<  ") ";
			std::cout << " is: " << Thickness_tagvalue << std::endl;
			std::cout << "origin is (" << sliceOriginID <<  ") ";
			std::cout << " is: " << Origin_tagvalue << std::endl;
			std::cout << "spacing is (" << pixelSpacingID <<  ") ";
			std::cout << " is: " << PixelSpacing_tagvalue << std::endl;
		}
		else
		{
			std::cerr << "Entry was not of string type" << std::endl;
			return ;
		}
		double slice_thickness = std::stod(Thickness_tagvalue);
		double pixel_spacing   = std::stod(PixelSpacing_tagvalue);
		double img_origin      = std::stod(Origin_tagvalue);

		//typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
		i2vConnectorType::Pointer connector = i2vConnectorType::New();
		//connect
		connector->SetInput(itk_reader->GetOutput());
		connector->Update();
		//output vtkimagedata to data_container
		//vtksmartpointer act as buff
		vtkSmartPointer<vtkImageData> buff =
			vtkSmartPointer<vtkImageData>::New();
		buff = connector->GetOutput();
		std::cout<<"converter: "<<i<<" done!"<<std::endl;

		//-----------change information(origin and spacing)--------------//
		//get information :origin
		//it seems that there is no need to change origin
		double origin[3]={0,0,0};
		buff->GetOrigin(origin);
		std::cout<<"origin:"<<origin[0]<<origin[1]<<origin[2]<<std::endl;

		double spacing[3]={0,0,0};
		buff->GetSpacing(spacing);
		spacing[2] = slice_thickness;

		vtkSmartPointer<vtkImageChangeInformation> changer = 
			vtkSmartPointer<vtkImageChangeInformation>::New();
		changer->SetOutputSpacing(spacing);
		//change 
		changer->SetInput(buff);
		//changer->SetOutputOrigin(origin);
		changer->Update();

		vtkSmartPointer<vtkImageData>temp_con_con = vtkSmartPointer<vtkImageData>::New();
		temp_con_con->DeepCopy(changer->GetOutput());
		container.push_back(temp_con_con);
		if (i == 0)
		{
			std::string filename_temp = info->description.at(index).toStdString();
			size_t num_slash = filename_temp.find("/");
			if (num_slash == std::string::npos)
			{
				file_names.push_back(filename_temp.append("-pd"));
			}
			else
			{
				filename_temp.replace(filename_temp.find("/"),1,"-");
				file_names.push_back(filename_temp.append("-pd"));
			}
		}
		else
		{
			std::string filename_temp = info->description.at(index).toStdString();
			if (filename_temp.find("/")!=std::string::npos)
			{
				filename_temp.replace(filename_temp.find("/"),1,"-");
			}
			file_names.push_back(filename_temp.append("-t2"));
		}
	}
	if (log_file.isOpen())
	{
		log_file.close();
	}

}


// no need to reverse here
void Log2Container_inDicomParse_Stereo_Only(QString log_name, File_info_in_DicomParse* info, std::vector<vtkSmartPointer<vtkImageData> >& container, int index, std::vector<std::string>& file_names)
{
	int size = 0;
	int slice_number = 0;
	int volume_number= 0;
	//find number of item in QStringList
	size = info->description.size();
	slice_number = info->slice_number.at(index).toInt();
	volume_number= info->temperal_number.at(index).toInt();

	std::cout<<"volume info:"
		<<info->description.at(index).toStdString()
		<<info->slice_number.at(index).toInt()
		<<info->temperal_number.at(index).toInt()
		<<std::endl;

	QFile log_file;
	log_file.setFileName(log_name);
	//--read log again to find file path--//
	//read file line by line
	if(!log_file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	//use text stream to read line
	QTextStream in(&log_file);
	//seek to path
	in.seek(info->path_position.at(index));
	//1. put name into a vector
	//divide these filenames by half
	std::vector< std::vector<std::string> > name_holder(1);
	int cnt = 0;
	while(cnt<slice_number)
	{
		QString f_name = in.readLine();
		name_holder[0].push_back(f_name.toStdString());
		cnt++;
	}

	//reverse the name holder
	//this step is useless !!!!!!!!!!!!!!!!!!!
	//do nothing, for sake of convenience
	std::vector< std::vector<std::string> > name_holder2(1);
	int size_name_holder = name_holder[0].size();
	for (int x=0;x<size_name_holder;x++)
	{
		name_holder2[0].push_back(name_holder[0].back());
		//name_holder[0].pop_back();
	}

	log_file.close();//read DicomSortLists.txt done

	if ( name_holder2[0].empty() )
	{
		return;
	}


	//define itk reader
	//typedef itk::Image< float , 3 >             ImageType; //image type pixel:float;dimension:3
	//typedef itk::ImageSeriesReader< ImageType > ReaderType;
	ReaderType_b::Pointer itk_reader = ReaderType_b::New();
	DicomIOType::Pointer  dicomIO = DicomIOType::New();
	itk_reader->SetImageIO(dicomIO);


	for(int i = 0;i<1;i++)
	{
		if (name_holder[i].empty())
		{
			return;
		}
		itk_reader->SetFileNames(name_holder[i]);
		try
		{
			itk_reader->Update();
		}
		catch (itk::ExceptionObject& e)
		{
			std::cerr<<e;
			return;
		}

		//get information from dicom tags( thickness among slices)
		const  DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();
		DictionaryType::ConstIterator itr = dictionary.Begin();
		DictionaryType::ConstIterator end = dictionary.End();

		std::string sliceThicknessID = "0018|0050";
		std::string sliceOriginID    = "0020|0032";
		std::string pixelSpacingID   = "0028|0030";
		DictionaryType::ConstIterator Thickness_tagItr   = dictionary.Find( sliceThicknessID );
		DictionaryType::ConstIterator Origin_tagItr      = dictionary.Find( sliceOriginID );
		DictionaryType::ConstIterator PixeSpacing_tagItr = dictionary.Find(pixelSpacingID);

		if( Thickness_tagItr == end | Origin_tagItr == end | PixeSpacing_tagItr == end)
		{
			std::cerr << "Tag " << sliceThicknessID<<"or"<<"sliceOriginID";
			std::cerr << " not found in the DICOM header" << std::endl;
			return;
		}
		MetaDataStringType::ConstPointer Thickness_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Thickness_tagItr->second.GetPointer() );
		MetaDataStringType::ConstPointer Origin_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Origin_tagItr->second.GetPointer() );
		MetaDataStringType::ConstPointer Spacing_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( PixeSpacing_tagItr->second.GetPointer() );


		std::string Thickness_tagvalue;
		std::string Origin_tagvalue;
		std::string PixelSpacing_tagvalue;

		if( Thickness_entryvalue || Origin_entryvalue)
		{
			Thickness_tagvalue    = Thickness_entryvalue->GetMetaDataObjectValue();
			Origin_tagvalue       = Origin_entryvalue   ->GetMetaDataObjectValue();
			PixelSpacing_tagvalue = Spacing_entryvalue  ->GetMetaDataObjectValue();
			std::cout << "slice thickness is (" << sliceThicknessID <<  ") ";
			std::cout << " is: " << Thickness_tagvalue << std::endl;
			std::cout << "origin is (" << sliceOriginID <<  ") ";
			std::cout << " is: " << Origin_tagvalue << std::endl;
			std::cout << "spacing is (" << pixelSpacingID <<  ") ";
			std::cout << " is: " << PixelSpacing_tagvalue << std::endl;
		}
		else
		{
			std::cerr << "Entry was not of string type" << std::endl;
			return ;
		}
		double slice_thickness = std::stod(Thickness_tagvalue);
		double pixel_spacing   = std::stod(PixelSpacing_tagvalue);
		double img_origin      = std::stod(Origin_tagvalue);

		//typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
		i2vConnectorType::Pointer connector = i2vConnectorType::New();
		//connect
		connector->SetInput(itk_reader->GetOutput());
		connector->Update();
		//output vtkimagedata to data_container
		//vtksmartpointer act as buff
		vtkSmartPointer<vtkImageData> buff =
			vtkSmartPointer<vtkImageData>::New();
		buff = connector->GetOutput();
		std::cout<<"converter: "<<i<<" done!"<<std::endl;

		//-----------change information(origin and spacing)--------------//
		//get information :origin
		//it seems that there is no need to change origin
		double origin[3]={0,0,0};
		buff->GetOrigin(origin);
		std::cout<<"origin:"<<origin[0]<<origin[1]<<origin[2]<<std::endl;

		double spacing[3]={0,0,0};
		buff->GetSpacing(spacing);
		spacing[2] = slice_thickness;

		vtkSmartPointer<vtkImageChangeInformation> changer = 
			vtkSmartPointer<vtkImageChangeInformation>::New();
		changer->SetOutputSpacing(spacing);
		//change 
		changer->SetInput(buff);
		//changer->SetOutputOrigin(origin);
		changer->Update();

		vtkSmartPointer<vtkImageData>temp_con_con = vtkSmartPointer<vtkImageData>::New();
		temp_con_con->DeepCopy(changer->GetOutput());
		container.push_back(temp_con_con);
		if (i == 0)
		{
			std::string filename_temp = info->description.at(index).toStdString();
			size_t num_slash = filename_temp.find("/");
			if (num_slash == std::string::npos)
			{
				file_names.push_back(filename_temp.append("-pd"));
			}
			else
			{
				filename_temp.replace(filename_temp.find("/"),1,"-");
				file_names.push_back(filename_temp.append("-pd"));
			}
		}
		else
		{
			std::string filename_temp = info->description.at(index).toStdString();
			if (filename_temp.find("/")!=std::string::npos)
			{
				filename_temp.replace(filename_temp.find("/"),1,"-");
			}
			file_names.push_back(filename_temp.append("-t2"));
		}
	}
	if (log_file.isOpen())
	{
		log_file.close();
	}
}


void Call_dcm2nii_func(std::string cmd_path,std::string folder_path)
{
	std::string space_space = " ";
	std::string full_cmd;

	full_cmd.append(cmd_path);
	full_cmd.append(space_space);
	full_cmd.append(folder_path);

	system(full_cmd.c_str());
}





void WiteToDicomSeries(vtkSmartPointer<vtkImageData> in_img)
{
	std::string OutputDir;
	//create output directory
	//itksys::SystemTools::MakeDirectory( OutputDir ); 

	//typdefs for output files. image type: 2D dimension
	typedef itk::Image< float , 2 >      OutputImageType;
	typedef itk::ImageSeriesWriter<ImageType,OutputImageType> 
										 SeriesWriterType;
	typedef itk::NumericSeriesFileNames  NamesGeneratorType;   
	typedef itk::GDCMImageIO             GDCMIOType;

	//connect vtkimage to itk iamge
	v2iConnectorType::Pointer connector = v2iConnectorType::New();
	connector->SetInput(in_img);
	try
	{
		connector->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cout<<"connect vtk to itk error"<<std::endl;
		std::cerr<<e;
	}
	ImageType::Pointer src_itk_img = ImageType::New();
	src_itk_img = connector->GetOutput();


	//generate dicom dict (dicom tags informations)
	GDCMIOType::Pointer gdcmIO = GDCMIOType::New();
	itk::MetaDataDictionary & dict = gdcmIO->GetMetaDataDictionary(); //get gdcmIO
	std::string tagkey, value;

	//tagkey and descriptions
	tagkey = "0008|0060"; // Modality   - this tag is ignored? 
	value = "CT"; 
	itk::EncapsulateMetaData<std::string>(dict, tagkey, value ); 

	tagkey = "0008|0008"; // Image Type 
	value = "DERIVED\\SECONDARY"; 
	itk::EncapsulateMetaData<std::string>(dict, tagkey, value); 

	tagkey = "0010|0010"; // Patient Name 
	value = "Pseudo_CT_B032"; 
	itk::EncapsulateMetaData<std::string>(dict, tagkey, value); 

	tagkey = "0008|0064"; // Conversion Type 
	value = "DV"; 
	itk::EncapsulateMetaData<std::string>(dict, tagkey, value); 
	//another solution from itk manual:
	//seriesWriter->SetMetaDataDictionaryArray(reader->GetMetaDataDictionaryArray() )

	//generate file names
	NamesGeneratorType::Pointer namesGenerator = NamesGeneratorType::New();
	//generate names according to image size
	ImageType::RegionType region = src_itk_img->GetLargestPossibleRegion();
	ImageType::IndexType  start = region.GetIndex();
	ImageType::SizeType   size  = region.GetSize();
	std::string format = OutputDir;
	format += "/%3d.dcm";
	namesGenerator->SetSeriesFormat(format.c_str());
	namesGenerator->SetStartIndex(start[2]);
	namesGenerator->SetEndIndex(start[2]+size[2]-1);
	namesGenerator->SetIncrementIndex(1);

	//write to series
	SeriesWriterType::Pointer seriesWriter = 
		SeriesWriterType::New();
	seriesWriter->SetInput(src_itk_img);
	seriesWriter->SetImageIO(gdcmIO); //add dict here!! 
	seriesWriter->SetFileNames(namesGenerator->GetFileNames());
	try
	{
		seriesWriter->Update();
	}
	catch(itk::ExceptionObject & e)
	{
		std::cerr<<"Exception Caught: series writer error"<<std::endl;
		std::cerr<<e<<std::endl;
		return;
	}

}





int Parse_GetDicomTag_InstanceNumber(std::string slice_name)
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







