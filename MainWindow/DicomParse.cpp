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






