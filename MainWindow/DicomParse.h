/******************************************
*Function: seek dicom image folder to read dicom headers
*
*Copyright: Mr Luo Jiaxi
*Changed by: Qin Shuo
*Date: 2015.3.30
******************************************/
#ifndef _DICOMPARSE_H_
#define _DICOMPARSE_H_


#include "itkImageSeriesReader.h"
#include "itkGDCMImageIO.h"
//read dcm file
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
//read .nii and .hdr/img file
#include "itkNiftiImageIO.h"
//itk image file reader
#include <itkImageFileReader.h>
//itk image file writer
#include "itkImageFileWriter.h"
#include "itkImageSeriesReader.h"
//convert vtk data format to itk data format
#include "itkVTKImageToImageFilter.h"
//convert itk data format to vtk data format
#include "itkImageToVTKImageFilter.h"

#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QList>
#include <QStringList>
#include <QFileInfoList>
#include <QDirIterator>
#include <QMessageBox>
#include <QFile>
#include <Qtextstream>
#include <QDialog>
#include <QProgressDialog>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>

#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkImageChangeInformation.h"


typedef itk::GDCMSeriesFileNames					NamesGeneratorType;
typedef float										PixelType;//typedef short      PixelType;
typedef itk::Image< PixelType, 3 >					ImageType;
typedef itk::ImageSeriesReader< ImageType >			ReaderType;
typedef itk::GDCMImageIO							ImageIOType;
//reader type (defined in DialogLoadImage.h)
typedef itk::ImageSeriesReader< ImageType >			ReaderType_b;
//writer type
typedef itk::ImageFileWriter  < ImageType >			WriterType_b;
//define IOs
typedef itk::NiftiImageIO							NiftiIOType;
typedef itk::GDCMImageIO							DicomIOType;
//connector type
typedef itk::ImageToVTKImageFilter<ImageType>		i2vConnectorType;
typedef itk::VTKImageToImageFilter<ImageType>		v2iConnectorType;
//dicom tags container type
typedef itk::MetaDataDictionary						DictionaryType;
//dicom tags are represented as strings
typedef itk::MetaDataObject<std::string>			MetaDataStringType;

struct File_info_in_DicomParse
{
	QStringList name;
	QList<qint64> path_position;
	QStringList description;
	QStringList slice_number;
	QStringList temperal_number;
};


//select dir
void Seek_Dicom_Folder(QString m_strDir);

//sub function
void ParseParseParse(QString path, QString m_strDir);

//parse log file
void Load_File_from_log(QString log_name,
						std::vector<vtkSmartPointer<vtkImageData> >& container,
						std::vector<std::string>&);

//read the log and transform context into data
void Log2Container_inDicomParse(QString log_name, 
				   File_info_in_DicomParse* info, 
				   std::vector<vtkSmartPointer<vtkImageData> >& container,
				   int index,
				   std::vector<std::string>&);

//call dcm2nii function to convert dicom images to nii
void Call_dcm2nii_func();


////--------------ui___dialog for tree widget----------------//
//class Tree_Dialog_in_DicomParse : public QDialog
//{
//	Q_OBJECT
//public:
//	Tree_Dialog_in_DicomParse(QWidget *parent	= 0);
//	~Tree_Dialog_in_DicomParse();
//
//	void SetInfo(File_info_in_DicomParse*, int *);
//	QTreeWidget* index_tree;
//	private slots:
//		void on_click_item(QTreeWidgetItem* item,int column);
//private:
//	int* index;
//	File_info_in_DicomParse* infox;
//};


#endif