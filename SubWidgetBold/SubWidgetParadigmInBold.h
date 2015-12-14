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
//#define QS_DEBUG_MODE
#define en_Read_Register_flag  0        //enable = 1
#define en_Brain_Strip_flag    1		//enable = 1
#define en_Visiualization_flag 1        //enable = 1
#define out_Lable_Map_flag     0        //enable = 1
	
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
#include <QVBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QThread>


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
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
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
#include "MyProgressDialog.h"

#ifndef QS_DEBUG_MODE
#include "MainWindow.h"
//#include "Volume.h"
//#include "LayerCollection.h"
//#include "LayerBold.h"
//#include "LayerVolume.h"
//#include "RenderView.h"
//#include "RenderView3D.h"
//#include "RenderView2D.h"
#endif


//----------typedefs for itk IOs-------------//
//pointer type : float
typedef itk::Point< double,3 >				PointTypex;//cooridnate and spacing
//image type: float, 3 dimensions (defined in DialogLoadImage.h)
#ifndef DIALOGLOADIMAGE_H
typedef itk::Image< unsigned char, 3 >             ImageType;
#endif
typedef itk::Image< float , 3 >             ImageTypex; //image type pixel:float;dimension:3
//image type for atlas 
typedef itk::Image<short, 3>					AtlasImageType;
typedef itk::Image<unsigned char, 3>			AtlasLabelType;
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

class SubWidgetParadigmInBold;
struct File_info
{
	QStringList name;
	QList<qint64> path_position;
	QStringList description;
	QStringList slice_number;
	QStringList temperal_number;
};


//Function: Parse log file and re-construction
class Data_ReConstruction
{
public:
	Data_ReConstruction()
	{
		signle_name = "/DicomSortList.txt";
		info = new File_info;
	};
	~Data_ReConstruction()
	{
		delete info;
	};

	int SetLogFile(std::string in){log_filename = in;return 0;};
	int SetLogDir (std::string in){log_filename = in.append(signle_name);log_dir = in;return 0;}; //generate full name auto..ly
	std::map<int,vtkSmartPointer<vtkImageData> > GetDataContainer(){return dataContainer; };
	int SetIndex  (int in){this->select_index = in;return 0;};
	File_info* GetInfo() {return this->info;};
	int ParseLogFile();
	int ConsturctData();

private:
	std::string log_filename; //log file full name
	std::string log_dir;      //project directory (hold dicomsortlist.txt file)
	std::string signle_name;  //DicomSortList.txt with out path
	File_info*  info;
	QStringList filenames_list;
	int         select_index;
	std::map<int,vtkSmartPointer<vtkImageData> > dataContainer;

	int PreProcess();
	int Log2Container(File_info* info, std::map<int,vtkSmartPointer<vtkImageData> > &container,int index);
	int GetDicomTag_InstanceNumber(std::string slice_name);
};


//Function: basic functions: generate design paradigm
//			hold all design parameters
class Fmri_paradigm
{
public:
	explicit Fmri_paradigm()
	{
		siglm = NULL;
		TR    = 0.0;
		numCondition = 0;
		numVolume    = 0;
		designMat = vtkFloatArray::New();
	};
	~Fmri_paradigm()
	{
		if (siglm != NULL)
			delete siglm;

		designMat->Delete();
	};

	//design para
	std::vector< std::vector<int> > onset;   //vtkFloatArray* onset;
	std::vector< std::vector<int> > duration;//vtkFloatArray* duration;

	int SetTR(float in)						 {TR=in; return 0;};
	int SetNumCondition(int in)				 {numCondition = in;return 0;};
	int SetNumVolume(int in)				 {numVolume = in; return 0;};
	int SetOnset(vector< vector<int> > in)   {onset = in;return 0;};
	int SetDuration(vector< vector<int> > in){duration = in;return 0;};
	//optional
	int SetPattern(std::string in)           {design_Pattern = in;return 0;};
	int SetContrast(std::vector<int> in)	 {contrast = in;return 0;	};

	//Return Value:
	//0: update success
	//1: size of condition does not match
	//2: onset and duration in-valid
	//3: number of volume and design length not match
	int Update()
	{
		if (siglm!=NULL)
			delete siglm;
		this->siglm = new SignalModeling;
		//this->numVolume = 60;
		this->siglm->SetLen(this->numVolume);
		this->siglm->SetTR(this->TR);
		this->siglm->SetnumComponent(this->numCondition);
		this->siglm->SetOnset(this->onset);
		this->siglm->SetDuration(this->duration);
		if ((numCondition!=onset.size())||(numCondition!=duration.size()))
		{
			std::cout<<"size of condition does not match"<<std::endl;
			delete siglm;
			siglm = NULL;
			return 1;
		}
		if ((onset.size() == 0)||(duration.size()==0))
		{
			std::cout<<"input valid onset and duration"<<std::endl;
			delete siglm;
			siglm = NULL;
			return 2;
		}
		if ((onset.back().back()+duration.back().back()) != numVolume)
		{
			std::cout<<"input valid number of volume"<<std::endl;
			delete siglm;
			siglm = NULL;
			return 3;
		}

		this->designMat = GenerateDesignMatrix(siglm);
		return 0;
	};

	vtkFloatArray* GetOutput(){return designMat;};
	vtkIntArray*   GetContrast()
	{
		if (siglm == NULL)
		{
			std::cout<<"Design Matrix not created ... Run Update first"<<std::endl;
			vtkIntArray* con_return = vtkIntArray::New();
			con_return->SetNumberOfTuples(0);
			return con_return;
		}

		int num_Component = this->designMat->GetNumberOfComponents();
		vtkIntArray* con_return = vtkIntArray::New();
		con_return->SetNumberOfComponents(1);
		con_return->SetNumberOfTuples(num_Component);
		this->contrast.resize(num_Component);
		for (int i=0;i<num_Component;i++)
			con_return->SetComponent(i,0,this->contrast[i]);

		return con_return;
	};

private:
	int   numCondition;  //number of conditions
	float TR;             //TR
	int   numVolume;      //number of volumes
	std::string design_Pattern;

	SignalModeling*       siglm;         //model paradigm
	vtkFloatArray*        designMat;     //design matrix
	std::vector<int>      contrast;	   //contrast vector

protected:
	int VectorToArray(std::vector< std::vector<int> > onset,std::vector< std::vector<int> > duration,SignalModeling* siglm);
};

//Function:  Main function to run fmri analysis
class Fmri_Module
{
public:
	explicit Fmri_Module()
	{
		designMat = vtkFloatArray::New();
		contrast  = vtkIntArray::New();
		CDF       = vtkSmartPointer<vtkCDF>::New();
		p         = 0.05;
		dof       = 0;

		BetaMap = vtkImageData::New();	//beta map
		tMap    = vtkImageData::New();	//t map
		ActArea = vtkImageData::New();	//activated area
	};
	~Fmri_Module()
	{
		designMat->Delete();
		contrast->Delete();
	};

	//input functions
	static float P2T(float p,int dof){auto CDF= vtkSmartPointer<vtkCDF>::New();return CDF->p2t(p,dof);};
	static float T2P(float t,int dof){auto CDF= vtkSmartPointer<vtkCDF>::New();return CDF->t2p(t,dof);};
	int Get_dof(){return this->dof;};
	int Set_p_Value(float in){this->p=in; int dof = data_container.size()-1;	this->t=this->CDF->p2t(this->p,dof);return 0;};
	int Set_t_Value(float in){this->t=in; return 0;};

	//ATTENTION: functions below must be run!!!!
	int SetDataset(std::map<int,vtkSmartPointer<vtkImageData> > in) {data_container = in; dof = data_container.size() ;return 0;};
	int SetContrast(vtkIntArray* in) {contrast  = in;return 0;};
	int SetDesignMatrix(vtkFloatArray* in) {designMat = in;return 0;};
	int SetTransformMatrix(itk::VersorRigid3DTransform< double >::Pointer in){transfrom_matrix = in ;return 0;};
	int SetReference(itk::Image<float,3>::Pointer in){T2_image_reference = in;return 0;};
	int SetBrainMask(AtlasLabelType :: Pointer    in){Brain_Mask         = in;return 0;};

	int Update();
	int UpDateParameters();
	
	vtkImageData* GetOutput(){return ActArea;};

private:
	float t,p;
	int dof;
	vtkFloatArray*  designMat;
	vtkIntArray*    contrast;
	std::map<int,vtkSmartPointer<vtkImageData> >   data_container;
	itk::VersorRigid3DTransform< double >::Pointer transfrom_matrix;
	itk::Image<float,3>::Pointer          T2_image_reference;
	AtlasLabelType :: Pointer             Brain_Mask;						

	vtkImageData* BetaMap;	//beta map
	vtkImageData* tMap;	//t map
	vtkImageData* ActArea;	//activated area
	vtkSmartPointer<vtkCDF> CDF;

protected:
	int CheckParameter(){};
	int ApplyTransform(vtkImageData* &input_im);
	int FitModel(){return 0;};
	int Activate(){return 0;};

	//progress report
	static void GLMVGenerator_Progress_Func(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);
	static void GLMEstimator_Progress_Func (vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);
};


namespace Ui {
class SubWidgetParadigmInBold;
}
class thread_base: public QThread
{
	Q_OBJECT
public:
	thread_base(){};
	~thread_base(){};
	void SetParent(SubWidgetParadigmInBold* in) {parent = in;};
	SubWidgetParadigmInBold* parent;
};
class SubWidgetParadigmInBold : public QWidget
{

    Q_OBJECT
    
public:
    explicit SubWidgetParadigmInBold(QWidget *parent = 0);
    ~SubWidgetParadigmInBold();

	//----Public function: These function must run first before all the process----//
	int SetMainWindow(MainWindow* in)
	{
		this->mainwnd = in;
		if (this->mainwnd == NULL)
			return 1;
		return 0;
	};
	//Return value: 0, success; 1, prj dir name empty
	int SetPrjDir()
	{
		this->img_prj_dir = this->mainwnd->GetProjectDirectory().toStdString();
		if (this->img_prj_dir.empty())
			return 1;
		return 0;
	};
	//Return value: 0, success; 1, src dir name empty
	int SetSrcDir()
	{
		this->img_src_dir = this->mainwnd->GetDirectory().toStdString();
		if (this->img_src_dir.empty())
			return 1;
		return 0;
	};
	//Return value: 0, success; 1,volume list is empty
	int SetReference()
	{
		//T2 reference image
		LayerCollection* lc_volume = this->mainwnd->GetLayerCollection( "Volume" );
		QList<Layer*> volum_list = lc_volume->GetLayers();
		if (volum_list.isEmpty())
			return 1;
		this->T2_image_reference =  ((LayerVolume*)volum_list[0])->GetITKImage();
		return 0;
	};
	//Ruturn value: 0, success; 1, brain mask is invalid; 2, brainITKImage not exist; 3, brain strip process failed
	int SetBrainMask()
	{
		//Brain_Mask;
		LayerCollection* lc_volume = this->mainwnd->GetLayerCollection( "Volume" );
		LayerVolume* brain = NULL;
		ImageType::Pointer brainITKImage = NULL;
		brain = (LayerVolume*)lc_volume->GetLayerByName("Brain");
		if (brain)
		{
			brainITKImage = brain->GetITKImage();
			if (brainITKImage)
			{
				std::cout<<"Get ITK Image";
				this->Brain_Mask = AtlasLabelType::New();
				//convert striped brain to float
				typedef itk::CastImageFilter<ImageType,AtlasLabelType> CasterType;
				CasterType::Pointer caster_temp = CasterType::New();
				caster_temp->SetInput(brainITKImage);
				try 
				{
					caster_temp->Update();
				}
				catch (itk::ExceptionObject &ex)
				{
					std::cout << ex << std::endl;
					return 1;
				}
				this->Brain_Mask = caster_temp->GetOutput();
		
			}
			else
			{
				std::cout<<"brainITKImage does not exist.."<<std::endl;
				return 2;
			}
		}
		else //run brain strip process
		{
			if (BrainStrip(this->T2_image_reference,this->Brain_Mask) != 0)
			{
				std::cout<<"Brain Strip process failed"<<std::endl;
				return 3;
			}
		}
		return 0;
	};
	//disable all button or input widget
	void DisableAllWidget()
	{

	};
	void EnableAllWidget()
	{

	};

public slots:
	//load function
	void on_click_loadimage();
	void on_click_loadimage_m() {load_thread[0]->start();}; // multi-thread version
	void loadimage_stage2();
	void loadiamge_stage3();
	void loadiamge_stage3_m() {load_thread3[0]->start();}; // multi-thread version
	//parameters input
	void on_click_use_default();
	void on_click_use_default_m() {model_thread[0]->start();}; // multi-thread version
	void on_click_user_input();
	void on_click_open();
	void on_click_advance();
	//run algorithm
	void on_click_threshold_stage1();
	void on_click_threshold();
	void on_click_threshold_m(){threshold_thread[0]->start();};   // multi-thread version
	void on_threshold_changed();
	void on_threshold_changed_m(){change_thread[0]->start();}; // multi-thread version
	void on_click_addcondition();
	void on_select_condition();
	void on_click_addcontrast();
	void on_select_contrast();
	void set_contrast();
	void visualize_act(){this->Bold_Visualize();};
	void err_information(QString in)
	{
		QMessageBox::warning(NULL,tr("Error!"),in,QMessageBox::Ok);
	};

signals:
	void errInfo(QString);
	void proceed_stage2();
	void proceed_stage3();
	void update_Contrast();
	void refresh_view();
	void emit_sel_contrast();
	void emit_threshold();

public:
    Ui::SubWidgetParadigmInBold *ui;
	/****************///----by qinshuo-----begin

	//All directories' names
	std::string img_src_dir;	//Corresponding to : m_strDir;
	std::string img_prj_dir;    //Corresponding to : m_strProjectDir;
	
	Fmri_Module*		 fmri_run;
	Fmri_paradigm*		 paradigm;
	Data_ReConstruction* data_constructer;

	MainWindow* mainwnd;

	itk::Image<float,3>::Pointer          T2_image_reference;				//Reference image to register ( T2 Image get from main window )
	AtlasLabelType :: Pointer             Brain_Mask;						//itk::Image<char, 3>::Pointer
	itk::VersorRigid3DTransform< double >::Pointer T2_fmri_register_matrix;	//transform matrix

	//image data container
	std::map<int,vtkSmartPointer<vtkImageData> > data_container;


	vtkFloatArray* designMat;       //design matrix
	vtkIntArray*   contrast;		//contrast vector
	//activated area
	vtkImageData* ActArea;

	//advance option for paradigm
	Dialog_Paradigm_Advance* advance_dialog ;

protected:
	//file information
	File_info* info;
	//index in log container
	int select_index;
	//filter flag
	int load_flag,model_flag;  // if 0 not initialized

	void init_para(); //should be clear when new volume specified
	//push enter to cal t-threshold
	void keyPressEvent(QKeyEvent *event);
	int Bold_Visualize();

	//function to plot design
	void Design_plot();

	//brain strip function
	int BrainStrip(itk::Image<float,3>::Pointer init_image,itk::Image<unsigned char, 3>::Pointer brain);
	int Read_Fmri_Configure(std::string filename, Fmri_paradigm* paradigm);
 

	//-----multi-thread class ----//

	class thread_load: public thread_base
	{
	public:
		thread_load(){};
		~thread_load(){};
	protected:
		void run(){parent->on_click_loadimage();};
	};
	class thread_load3: public thread_base
	{
	public:
		thread_load3(){};
		~thread_load3(){};
	protected:
		void run(){parent->loadiamge_stage3();};
	};

	class thread_model: public thread_base
	{
	public:
		thread_model(){};
		~thread_model(){};
	protected:
		void run(){parent->on_click_use_default();};
	};

	class thread_threshold: public thread_base
	{
	public:
		thread_threshold(){};
		~thread_threshold(){};
	protected:
		void run(){parent->on_click_threshold_stage1();};
	};

	class thread_th_change: public thread_base
	{
	public:
		thread_th_change(){};
		~thread_th_change(){};
	protected:
		void run(){parent->on_threshold_changed();};
	};

	thread_load*       load_thread[1];
	thread_load3*      load_thread3[1];
	thread_model*      model_thread[1];
	thread_threshold*  threshold_thread[1];
	thread_th_change*  change_thread[1];
};








//--------------ui_dialog for tree widget----------------//
class Tree_Dialog : public QDialog
{
	Q_OBJECT
public:
	Tree_Dialog(QWidget *parent	= 0);
	~Tree_Dialog();
	
	QVBoxLayout* verticalLayout;
	QLabel* label;
	QGridLayout* gridLayout;
	QPushButton* loadBtn;
	QPushButton* cancelBtn;
	QTreeWidget* index_tree;

	void SetInfo(File_info*, int *);

private slots:
	void on_click_item(QTreeWidgetItem* item,int column);
	void on_click_load();
	void on_click_cancel();
private:
	int* index;
	File_info* infox;
};





#endif // SUBWIDGETPARADIGMINBOLD_H