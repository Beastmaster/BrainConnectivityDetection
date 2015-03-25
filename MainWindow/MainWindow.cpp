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

	std::string filenamexx = this->file_name.toStdString();
	const char* cccc = filenamexx.data();
	std::cout<<cccc<<std::endl;

	vtkSmartPointer<vtkMetaImageReader> img_reader = 
		vtkSmartPointer<vtkMetaImageReader>::New();
	img_reader->SetFileName(cccc);
	img_reader->Update();

	this->img_to_view = img_reader->GetOutput();
}



void MainWindow::on_click_show()
{
	slice_view_base* view_axial = new slice_view_base(this->ui->axial_view_widget->GetRenderWindow(),'a');
	view_axial->Set_View_Img(this->img_to_view);
	view_axial->RenderView(10);
	slice_view_base* view_cornoal = new slice_view_base(this->ui->coronal_view_widget->GetRenderWindow(),'c');
	view_cornoal->Set_View_Img(this->img_to_view);
	view_cornoal->RenderView(13);
	slice_view_base* view_saggital = new slice_view_base(this->ui->sagittal_view_widget->GetRenderWindow(),'s');
	view_saggital->Set_View_Img(this->img_to_view);
	view_saggital->RenderView(10);
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




slice_view_base::slice_view_base(vtkRenderWindow* winx,char a)
{
	this->Set_Direction(a);
	this->Set_Window(winx);
	this->slice_n = 0;

	//renderer init
	new_render = vtkSmartPointer<vtkRenderer>::New();
	this->view_window->AddRenderer(this->new_render);

	//view2 init
	img_viewer2 = vtkSmartPointer<vtkImageViewer2>::New();
	img_viewer2->SetRenderWindow(this->view_window);
	img_viewer2->SetRenderer(new_render);
	switch(this->direction)
	{
	case 'a':
		{
			img_viewer2->SetSliceOrientationToXY();
			break;
		}
	case 'c':
		{
			img_viewer2->SetSliceOrientationToYZ();
			break;
		}
	case 's':
		{
			img_viewer2->SetSliceOrientationToXZ();
			break;
		}
	default:
		{
			img_viewer2->SetSliceOrientationToXY();
			break;
		}
	}
}

slice_view_base::~slice_view_base()
{}

void slice_view_base::RenderView(int x)
{
	this->slice_n = x;
	img_viewer2->SetSlice(this->slice_n);
	this->img_viewer2->Render();
}

void slice_view_base::Set_Direction(char x)
{
	this->direction = x ;
	switch (x)
	{
	case 'a':
		{
			double axialX[3] = {1,0,0};
			double axialY[3] = {0,1,0};
			double axialZ[3] = {0,0,1};
			this->view_dirX = axialX;
			this->view_dirY = axialY;
			this->view_dirZ = axialZ;
			break;
		}
	case 'c':
		{
			double coronalX[3] = {1,0,0};
			double coronalY[3] = {0,0,-1};
			double coronalZ[3] = {0,1,0};
			this->view_dirX = coronalX;
			this->view_dirY = coronalY;
			this->view_dirZ = coronalZ;
			break;
		}
	case 's':
		{
			double sagittalX[3] = {0,1,0};
			double sagittalY[3] = {0,0,-1};
			double sagittalZ[3] = {-1,0,0};
			this->view_dirX = sagittalX;
			this->view_dirY = sagittalY;
			this->view_dirZ = sagittalZ;
			break;
		}
	default:
		{
			double xxx[3] = {0,1,0};
			double yyy[3] = {0,0,-1};
			double zzz[3] = {-1,0,0};
			this->view_dirX = xxx;
			this->view_dirY = yyy;
			this->view_dirZ = zzz;
			break;
		}
	}
}

void slice_view_base::Set_View_Img(vtkSmartPointer<vtkImageData> img)
{
	this->img_to_view = img;
	this->img_viewer2->SetInput(img_to_view);
}

void slice_view_base::Set_Window(vtkRenderWindow* win)
{
	this->view_window = vtkSmartPointer<vtkRenderWindow>::New();
	this->view_window = win;
}

double* slice_view_base::calculate_img_center(vtkSmartPointer<vtkImageData> img)
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






