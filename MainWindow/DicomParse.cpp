
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

	for (int i=0;i<strlstAllDir.size();i++)
	{
		ParseParseParse(strlstAllDir.at(i),m_strDir);
	}

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

		//Sequence Name
		std::string seriesDescriptionTag = "0008|103e";
		std::string seriesDescription = "/";

		DictionaryType::ConstIterator seriesDescriptionTagItr = dictionary.Find(seriesDescriptionTag);

		if(seriesDescriptionTagItr!=end)
		{
			MetaDataStringType::ConstPointer seriesDescriptionEntryvalue =
				dynamic_cast<const MetaDataStringType *>( seriesDescriptionTagItr->second.GetPointer() );

			seriesDescription = seriesDescriptionEntryvalue->GetMetaDataObjectValue();
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
				out	<<QString::fromStdString(fileNames.at(i))<<endl;
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
		//parse from log and add data to container
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
			QStringList f_name_list;
			if(f_name.contains("\\") || f_name.contains("/"))
			{
				f_name_list= f_name.split("\\");
				f_name_list= f_name_list.last().split("/");
			}
			else
			{
				name_to_sort_all[cnt] = f_name.toStdString();
				cnt++;
				return;
			}
			QString f_file_name = f_name_list.last().split(".").first();
			QString f_file_order;
			if (f_file_name.split("_").size()>2)
			{
				f_file_order = f_file_name.split("_").at(1);
			}
			else
			{
				f_file_order = f_file_name.split("_").first();
			}

			if (f_file_order.toInt())
			{
				int order_f = f_file_order.toInt();
				name_to_sort_all[order_f] = f_name.toStdString();
			}
			else
			{
				name_to_sort_all[cnt] = f_name.toStdString();
			}
			cnt++;
		}
	}
	else
	{
		while(cnt<slice_number-(slice_number%volume_number))
		{
			//example:
			//C:\Users\USER\Documents\Medical_Images\CHOW,^CHUN^. P8801456_fMRI 10Jun2013\MR_1071.dcm
			//C:\Users\USER\Documents\Medical_Images\CHOW,^CHUN^. P8801456_fMRI 10Jun2013\MR_1072.dcm
			//C:/Users/USER/Documents/Medical_Images/Brainpla_281/BOLDacti_3578/MR_10_292357.dcm
			//C:/Users/USER/Documents/Medical_Images/Brainpla_281/BOLDacti_3578/MR_11_292358.dcm
			//1. split by "\\", get file name 
			//2. split file name by ".", get file name without suffix
			//3. split file name by "_", get order number
			QString f_name = in.readLine();
			QStringList f_name_list;
			if(f_name.contains("\\") || f_name.contains("/"))
			{
				f_name_list= f_name.split("\\");
				f_name_list= f_name_list.last().split("/");
			}
			else
			{
				name_to_sort_all[cnt] = f_name.toStdString();
				cnt++;
				return;
			}
			QString f_file_name = f_name_list.last().split(".").first();
			QString f_file_order;
			if (f_file_name.split("_").size()>2)
			{
				f_file_order = f_file_name.split("_").at(1);
			}
			else
			{
				f_file_order = f_file_name.split("_").first();
			}

			if (f_file_order.toInt())
			{
				int order_f = f_file_order.toInt();
				name_to_sort_all[order_f] = f_name.toStdString();
			}
			else
			{
				name_to_sort_all[cnt] = f_name.toStdString();
			}
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
		if(1)
		{		
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
			file_names.push_back(info->name.at(index).toStdString());
		}
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




void Call_dcm2nii_func()
{

}





/***************
//----------tree widget-------------//
Tree_Dialog_in_DicomParse::Tree_Dialog_in_DicomParse(QWidget *parent): QDialog(parent)
{
	setWindowTitle("Select Index");
	index_tree = new QTreeWidget(this);
	index_tree->setObjectName(QString::fromUtf8("index_tree"));
	index_tree->setGeometry(QRect(20, 10, 351, 271));

	QStringList head;
	head<<"Name"<<"Volume";
	index_tree->setHeaderLabels(head);
}

Tree_Dialog_in_DicomParse::~Tree_Dialog_in_DicomParse()
{
}

void Tree_Dialog_in_DicomParse::on_click_item(QTreeWidgetItem* item,int column)
{

	int col = this->index_tree->indexOfTopLevelItem(item);

	std::cout<<"selected col:"<<col;

	*this->index = col;
	this->done(100);
	this->hide();
}

void Tree_Dialog_in_DicomParse::SetInfo(File_info_in_DicomParse* in_info, int * select_index)
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
**********/

