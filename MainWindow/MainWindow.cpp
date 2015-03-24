#include "MainWindow.h"
#include "ui_MainWindow.h"


//head for extract contour
#include "vtkContourFilter.h"
#include "vtkPolyDataNormals.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkOutlineFilter.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	//visiulization init
	this->axial_vtk_viewer = vtkSmartPointer<vtkImageViewer2>::New();
	this->axial_vtk_renderer = vtkSmartPointer<vtkRenderer>::New();
	ui->axial_view_widget->GetRenderWindow()->AddRenderer(axial_vtk_renderer);

	connect(this->ui->show_Btn,SIGNAL(clicked()),this,SLOT(on_click_show()));
	connect(this->ui->img_load_Btn,SIGNAL(clicked()),this,SLOT(on_click_load()));
}

MainWindow::~MainWindow()
{
}





void MainWindow::on_click_load()
{
	this->file_name = 
		QFileDialog::getOpenFileName(this,
		tr("open log file"),"./",tr("(*)"));
	if (file_name.isEmpty()){return;}

	QByteArray ba = file_name.toLocal8Bit();
	const char* filenamexx = ba.data();

	vtkSmartPointer<vtkMetaImageReader> img_reader = 
		vtkSmartPointer<vtkMetaImageReader>::New();
	img_reader->SetFileName(filenamexx);
	std::cout<<filenamexx<<std::endl;
	img_reader->Update();

	this->img_to_view = img_reader->GetOutput();
	
}



void MainWindow::on_click_show()
{
	vtkSmartPointer<vtkContourFilter> skin_extract = 
		vtkSmartPointer<vtkContourFilter>::New();
	skin_extract->SetInput(this->img_to_view);
	skin_extract->SetValue(100,500);

	vtkSmartPointer<vtkPolyDataNormals> skin_norm = 
		vtkSmartPointer<vtkPolyDataNormals>::New();
	skin_norm->SetInput(skin_extract->GetOutput());
	skin_norm->SetFeatureAngle(60.0);
	vtkSmartPointer<vtkPolyDataMapper> skin_mapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	skin_mapper->SetInput(skin_norm->GetOutput());
	skin_mapper->ScalarVisibilityOff();

	vtkSmartPointer<vtkActor> skin_act = vtkSmartPointer<vtkActor>::New();
	skin_act->SetMapper(skin_mapper);

	//add a frame
	vtkOutlineFilter *outline = vtkOutlineFilter::New();
	outline->SetInput(this->img_to_view);
	vtkPolyDataMapper *mapoutline = vtkPolyDataMapper::New();
	mapoutline->SetInput(outline->GetOutput());
	vtkActor* outline_actor = vtkActor::New();
	outline_actor->SetMapper(mapoutline);

	//set camera
	vtkSmartPointer<vtkCamera> a_camera = vtkSmartPointer<vtkCamera>::New();
	a_camera->SetViewUp(0,0,-1);
	a_camera->SetPosition(0,1,0);
	a_camera->SetFocalPoint(0,0,0);
	a_camera->ComputeViewPlaneNormal();

	this->axial_vtk_renderer->AddActor(skin_act);
	this->axial_vtk_renderer->AddActor(outline_actor);
	this->axial_vtk_renderer->SetActiveCamera(a_camera);
	a_camera->Dolly(1.5);

	this->axial_vtk_renderer->SetBackground(1,1,0);
	
	this->axial_vtk_renderer->ResetCameraClippingRange();

	vtkSmartPointer<vtkRenderWindowInteractor> ren_intac = vtkSmartPointer<vtkRenderWindowInteractor>::New();

	ren_intac->SetRenderWindow(this->ui->axial_view_widget->GetRenderWindow());

	ren_intac->Initialize();
	ren_intac->Start();
	return;
}


