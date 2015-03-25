#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	connect(this->ui->show_Btn,SIGNAL(clicked()),this,SLOT(on_click_show()));
	connect(this->ui->img_load_Btn,SIGNAL(clicked()),this,SLOT(on_click_load()));
}

MainWindow::~MainWindow()
{
	delete ui;
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
	this->display_in_axial(this->img_to_view);
	this->display_in_coronal(this->img_to_view);
	this->display_in_sagittal(this->img_to_view);
}


void MainWindow::display_in_axial(vtkSmartPointer<vtkImageData> img_to_dis)
{
	static double axialX[3] = {1,0,0};
	static double axialY[3] = {0,1,0};
	static double axialZ[3] = {0,0,1};

	//assign reslice direction
	vtkSmartPointer<vtkImageReslice> axi_reslice= 
		vtkSmartPointer<vtkImageReslice>::New();
	axi_reslice->SetInput(img_to_dis);
	axi_reslice->SetOutputDimensionality(2);
	axi_reslice->SetResliceAxesDirectionCosines(axialX,axialY,axialZ);
	axi_reslice->SetResliceAxesOrigin(this->calculate_img_center(img_to_dis));
	axi_reslice->SetInterpolationModeToLinear();

	//look up table
	vtkSmartPointer<vtkWindowLevelLookupTable> table = 
		vtkSmartPointer<vtkWindowLevelLookupTable>::New();
	table->SetWindow(1000);
	table->SetLevel(100);

	vtkSmartPointer<vtkImageMapToColors> color = 
		vtkSmartPointer<vtkImageMapToColors>::New();
	color->SetLookupTable(table);
	color->SetInput(axi_reslice->GetOutput());

	vtkSmartPointer<vtkImageActor> actor = 
		vtkSmartPointer<vtkImageActor>::New();
	actor->SetInput(color->GetOutput());

	//add a renderer
	vtkSmartPointer<vtkRenderer> new_render = 
		vtkSmartPointer<vtkRenderer>::New();
	new_render->AddActor(actor);
	this->ui->axial_view_widget->GetRenderWindow()->AddRenderer(new_render);
	//render 
	this->ui->axial_view_widget->GetRenderWindow()->Render();
}

void MainWindow::display_in_coronal(vtkSmartPointer<vtkImageData> img_to_dis)
{
	static double coronalX[3] = {1,0,0};
	static double coronalY[3] = {0,0,-1};
	static double coronalZ[3] = {0,1,0};

	//assign reslice direction
	vtkSmartPointer<vtkImageReslice> cor_reslice= 
		vtkSmartPointer<vtkImageReslice>::New();
	cor_reslice->SetInput(img_to_dis);
	cor_reslice->SetOutputDimensionality(2);
	cor_reslice->SetResliceAxesDirectionCosines(coronalX,coronalY,coronalZ);
	cor_reslice->SetResliceAxesOrigin(this->calculate_img_center(img_to_dis));
	cor_reslice->SetInterpolationModeToLinear();

	//look up table
	vtkSmartPointer<vtkWindowLevelLookupTable> table = 
		vtkSmartPointer<vtkWindowLevelLookupTable>::New();
	table->SetWindow(1000);
	table->SetLevel(100);

	vtkSmartPointer<vtkImageMapToColors> color = 
		vtkSmartPointer<vtkImageMapToColors>::New();
	color->SetLookupTable(table);
	color->SetInput(cor_reslice->GetOutput());

	vtkSmartPointer<vtkImageActor> actor = 
		vtkSmartPointer<vtkImageActor>::New();
	actor->SetInput(color->GetOutput());

	vtkSmartPointer<vtkRenderer> new_render = 
		vtkSmartPointer<vtkRenderer>::New();
	new_render->AddActor(actor);
	this->ui->coronal_view_widget->GetRenderWindow()->AddRenderer(new_render);
	//render 
	this->ui->coronal_view_widget->GetRenderWindow()->Render();
}


void MainWindow::display_in_sagittal(vtkSmartPointer<vtkImageData> img_to_dis)
{
	static double sagittalX[3] = {0,1,0};
	static double sagittalY[3] = {0,0,-1};
	static double sagittalZ[3] = {-1,0,0};

	//assign reslice direction
	vtkSmartPointer<vtkImageReslice> sag_reslice= 
		vtkSmartPointer<vtkImageReslice>::New();
	sag_reslice->SetInput(img_to_dis);
	sag_reslice->SetOutputDimensionality(2);
	sag_reslice->SetResliceAxesDirectionCosines(sagittalX,sagittalY,sagittalZ);
	sag_reslice->SetResliceAxesOrigin(this->calculate_img_center(img_to_dis));
	sag_reslice->SetInterpolationModeToLinear();

	//look up table
	vtkSmartPointer<vtkWindowLevelLookupTable> table = 
		vtkSmartPointer<vtkWindowLevelLookupTable>::New();
	table->SetWindow(1000);
	table->SetLevel(100);

	vtkSmartPointer<vtkImageMapToColors> color = 
		vtkSmartPointer<vtkImageMapToColors>::New();
	color->SetLookupTable(table);
	color->SetInput(sag_reslice->GetOutput());

	vtkSmartPointer<vtkImageActor> actor = 
		vtkSmartPointer<vtkImageActor>::New();
	actor->SetInput(color->GetOutput());

	vtkSmartPointer<vtkRenderer> new_render = 
		vtkSmartPointer<vtkRenderer>::New();
	new_render->AddActor(actor);
	this->ui->sagittal_view_widget->GetRenderWindow()->AddRenderer(new_render);
	//render 
	this->ui->sagittal_view_widget->GetRenderWindow()->Render();
	return;
}

double* MainWindow::calculate_img_center(vtkSmartPointer<vtkImageData> img)
{
	double spacing[3];
	double origin[3];
	int extent[6];

	img->GetWholeExtent(extent);
	img->GetSpacing(spacing);
	img->GetOrigin(origin);

	double center[3];
	for (int i=0;i<3;i++)
	{
		center[i] = origin[i]+spacing[i]*0.5*(extent[i]+extent[i+1]);
	}

	return center;
}