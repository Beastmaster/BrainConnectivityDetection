#ifndef SUBWIDGETPARADIGMINBOLD_H
#define SUBWIDGETPARADIGMINBOLD_H
//test flag
/*
* change flag position:
*1. QS_DEBUG_MODE
*2. en_Read_Register_flag
*3. en_Brain_Strip_flag
*4. en_Visualization
*5. register all fmri image flag #PreProcess()
*6. threshold-- label map or t-map
*/
#define QS_DEBUG_MODE
#define en_Read_Register_flag  1        //enable = 1
#define en_Brain_Strip_flag    0		//enable = 1
#define en_Visiualization_flag 0        //enable = 1
#define out_Lable_Map_flag     1        //enable = 1
	
#ifdef QS_DEBUG_MODE
#define BRAINSTRIP_SRC	"D:/WorkPlace/NeuroAres/brain_strip_atlas/atlasImage.mha"
#define BRAINSTRIP_MASK "D:/WorkPlace/NeuroAres/brain_strip_atlas/atlasMask.mha"
#else
#define BRAINSTRIP_SRC "C:/Users/rc-mic/Desktop/NeuroAres/resources/data/atlasImage.mha"
#define BRAINSTRIP_MASK "C:/Users/rc-mic/Desktop/NeuroAres/resources/data/atlasMask.mha"
#endif

#include <QWidget>
#include "fMRIEngine_include.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug> 
#include <QString> 
//tree widget
#include <QTreeWidget>
#include <QTreeWidgetItem>
//scroll area
#include <QScrollArea>
//for file parser
//choose file
#include "QFileDialog"
//qt file
#include "QFile"
//qlist
#include <QList>
//qlist widget
#include <QListWidget>
//file
#include <qdir.h>
#include <qfiledialog.h>
#include <QProgressDialog>

#include <stdio.h>
#include <string.h>
//use vector and map
#include <vector>
#include <map>

//use itk to convert 2D slices to 3D volume
#include <itkImage.h>
//itk smart pointer
#include <itkSmartPointer.h>
//itk image file reader
#include <itkImageFileReader.h>
//itk image file writer
#include "itkImageFileWriter.h"
#include "itkImageSeriesReader.h"
//convert vtk data format to itk data format
#include "itkVTKImageToImageFilter.h"
//convert itk data format to vtk data format
#include "itkImageToVTKImageFilter.h"
//read dcm file
#include "itkGDCMImageIO.h"
//read .nii and .hdr/img file
#include "itkNiftiImageIO.h"
//itk cast image to different type
#include "itkCastImageFilter.h"
//deep copy
#include "itkImageRegionIterator.h"

//vtk reader
#include <vtkImageMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
//cast INT to FLOAT
#include <vtkImageCast.h>
//vtk change origin information
#include <vtkImageChangeInformation.h>
//dicom reader
#include <vtkDICOMImageReader.h>
//vti writer, save beta
#include <vtkXMLImageDataWriter.h>
//mha read/write
#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
//data processing
#include "vtkImageThreshold.h"
#include "vtkImageGaussianSmooth.h"
//image flip (coordinate transform)
#include "vtkImageFlip.h"
//image threshold
#include <vtkImageThreshold.h>
#include "vtkObjectFactory.h"

//plot design matrix
#include "qcustomplot.h"
//for registration
#include "Register.h"
//for skull striping
#include "itkStripTsImageFilter.h"
//itk mask
#include "itkMaskImageFilter.h"
//time count
#include <time.h>
//add new advance dialog 
#include "Dialog_Paradigm_Advance.h"


//----------typedefs for itk IOs-------------//
//pointer type : float
//typedef itk::Point< double,3 >				PointType;//cooridnate and spacing
//image type: float, 3 dimensions (defined in DialogLoadImage.h)
typedef itk::Image< float , 3 >             ImageTypex; //image type pixel:float;dimension:3
//image type for atlas 
typedef itk::Image<short, 3>					AtlasImageType;
typedef itk::Image<unsigned char, 3>						AtlasLabelType;
//reader type: atlas
typedef itk::ImageFileReader<AtlasImageType> AtlasReaderType;
typedef itk::ImageFileReader<AtlasLabelType> LabelReaderType;
//reader type (defined in DialogLoadImage.h)
typedef itk::ImageSeriesReader< ImageTypex > ReaderType_b;
//writer type
typedef itk::ImageFileWriter  < ImageTypex > WriterType_b;
//define IOs
typedef itk::NiftiImageIO					 NiftiIOType;
typedef itk::GDCMImageIO					 DicomIOType;
//connector type
typedef itk::ImageToVTKImageFilter<ImageTypex> i2vConnectorType;
typedef itk::VTKImageToImageFilter<ImageTypex> v2iConnectorType;
//dicom tags container type
typedef itk::MetaDataDictionary				 DictionaryType;
//dicom tags are represented as strings
typedef itk::MetaDataObject<std::string>	 MetaDataStringType;

struct File_info
{
	QStringList name;
	QList<qint64> path_position;
	QStringList description;
	QStringList slice_number;
	QStringList temperal_number;
};

namespace Ui {
class SubWidgetParadigmInBold;
}

class SubWidgetParadigmInBold : public QWidget
{
    Q_OBJECT
    
public:
    explicit SubWidgetParadigmInBold(QWidget *parent = 0);
    ~SubWidgetParadigmInBold();

public slots:
	void on_click_addcondition();
	void on_select_condition();
	void on_click_viewdesign();
	void on_click_fitmode();
	void on_click_savebeta();
	void on_click_open();
	void on_click_loadimage();

	void on_click_addcontrast();
	void on_select_contrast();
	void on_click_activate();
	void on_click_threshold();
	void on_threshold_changed();
	void on_click_plot();

	//one_click_test function
	void on_click_test();
	void on_click_advance();
	void on_click_use_default();
	void on_click_user_input();

private:
    Ui::SubWidgetParadigmInBold *ui;
	/****************///----by qinshuo-----begin
	void init_para(); //should be clear when new volume specified
	//filename list
	QStringList filenames_list;

	//paradigm
	//counter for times of "add" clicked
	int num_Condition;
	//TR
	float TR;
	//vtkFloatArray* onset;
	vector< vector<int> > onset;
	//vtkFloatArray* duration;
	vector< vector<int> > duration;
	//parameters
	int numVolume;
	SignalModeling* siglm;      //model paradigm
	vtkFloatArray* designMat;   //design matrix
	vtkIntArray* contrast;		//contrast vector
	float t,p;

	//-----get from outside-----//
	itk::Image<float,3>::Pointer          T2_image_reference;
	//itk::Image<char, 3>::Pointer				Brain_Mask;
	AtlasLabelType :: Pointer                             Brain_Mask;
	//transform matrix
	itk::VersorRigid3DTransform< double >::Pointer T2_fmri_register_matrix;

	//-----get from outside-----//

	//GLMDector
	vtkGLMDetector* GLMDetector;
	//GLMEstimator
	vtkGLMEstimator* GLMEstimator;
	//GLMVolumeGenerator
	//vtkGLMVolumeGenerator* GLMVolmeGenerator;
	//CDF:compute t
	vtkCDF* CDF ;
	//threshold
	//vtkIsingActivationThreshold* IsingActivationThreshold;
	//beta map
	vtkImageData* BetaMap;
	//t map
	vtkImageData* tMap;
	//activated area
	vtkImageData* ActArea;
	Dialog_Paradigm_Advance* advance_dialog ;
protected:
	//push enter to cal t-threshold
	void keyPressEvent(QKeyEvent *event);
	int pip_flag;
	//image data container
	//std::vector<vtkSmartPointer<vtkImageData> > data_container;
	std::map<int,vtkSmartPointer<vtkImageData> > data_container;
	//file handle
	QFile file;
	//file information
	File_info* info;
	//index in log container
	int select_index;
	//fixex image in registration
	itk::Image<float,3>::Pointer reg_refer_container;
	//parse log file and add data to container
	void Log2Container(File_info*, 
		std::map<int,vtkSmartPointer<vtkImageData> >&,int);
	//data pre-processing
	void PreProcess();
	//analyze writer
	//no implement
	//function to plot design
	void Design_plot();
	//registration
	void Register_process(vtkImageData* &);
	int Bold_Visualize(const vtkImageData* &);

	void Get_Register_Image_process();
	void Brain_Striping_process();

    /****************///----by qinshuo-----end
};








//--------------ui___dialog for tree widget----------------//
class Tree_Dialog : public QDialog
{
	Q_OBJECT
public:
	Tree_Dialog(QWidget *parent	= 0);
	~Tree_Dialog();

	void SetInfo(File_info*, int *);
	QTreeWidget* index_tree;
private slots:
		void on_click_item(QTreeWidgetItem* item,int column);
private:
	int* index;
	File_info* infox;
};



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
	ActivationThreshold();
	~ActivationThreshold();
	void SimpleExecute(vtkImageData *input, vtkImageData *output);
	void ExecuteInformation(vtkImageData *input, vtkImageData *output);
};

#endif // SUBWIDGETPARADIGMINBOLD_H