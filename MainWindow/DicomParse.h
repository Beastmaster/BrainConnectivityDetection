/******************************************
*Function: seek dicom image folder to read dicom headers
*
*Copyright: Mr Luo Jiaxi
*Changed by: Qin Shuo
*Date: 2015.3.30
******************************************/


#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkGDCMImageIO.h"

#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QList>
#include <QStringList>
#include <QFileInfoList>
#include <QDirIterator>
#include <QMessageBox>
#include <QFile>
#include <Qtextstream>

typedef itk::GDCMSeriesFileNames NamesGeneratorType;
typedef float      PixelType;//typedef short      PixelType;
//const unsigned int         Dimension = 3;
typedef itk::Image< PixelType, 3 >      ImageType;
typedef itk::ImageSeriesReader< ImageType >     ReaderType;
typedef itk::GDCMImageIO       ImageIOType;


//select dir
void Seek_Dicom_Folder(QString m_strDir);

//sub function
void ParseParseParse(QString path, QString m_strDir);

