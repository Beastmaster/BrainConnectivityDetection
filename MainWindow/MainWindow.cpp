#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "vtkGetTimecourse.h"
#include "vtkGetTimecourse.cpp"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	this->init_Parameters();
	this->set_data_container(this->data_container);

	//axial_renderer->AddActor(axial_mask_Actor);
	//coronal_renderer->AddActor(coronal_mask_Actor);
	//sagittal_renderer->AddActor(sagittal_mask_Actor);

	connect(this->ui->show_Btn,SIGNAL(clicked()),this,SLOT(on_click_show()));
	connect(this->ui->show_Btn,SIGNAL(clicked()),this,SLOT(on_click_show3d()));
	connect(this->ui->img_load_Btn,SIGNAL(clicked()),this,SLOT(on_click_load()));
	//connect bold function
	connect(this->ui->bold_Btn,SIGNAL(clicked()),this,SLOT(on_click_bold()));
	//slider bar
	connect(this->ui->view_vol_Slider,SIGNAL(valueChanged(int)),this,SLOT(on_slider_volume_move(int)));
	connect(this->ui->set_opicity_Slider,SIGNAL(valueChanged(int)),this,SLOT(on_slider_opicity_move(int)));
	connect(this->ui->strip_val_Slider,SIGNAL(valueChanged(int)),this,SLOT(on_slider_strip_val_move(int)));

	//connect signal to enable atuo-scroll in info panel
	connect(this->ui->info_Panel,SIGNAL(cursorPositionChanged()),this,SLOT(info_Panel_Scroll()));

	//connect mask function Btns
	connect(this->ui->add_mask_Btn,SIGNAL(clicked()),this,SLOT(on_click_add_mask_file()));
	connect(this->ui->clr_mask_Btn,SIGNAL(clicked()),this,SLOT(on_click_del_mask()));
	connect(this->ui->mask_Btn,SIGNAL(clicked()),this,SLOT(on_click_mask()));

}

MainWindow::~MainWindow()
{
	if (this->bold_win!=NULL)
	{
		delete this->bold_win;
	}
	if (this->view_axial!=NULL)
	{
		delete view_axial;
		delete view_cornoal;
		delete view_saggital;
	}
	if (this->view_coronal_reslice!=NULL)
	{
		delete view_axial_reslice;
		delete view_coronal_reslice;
		delete view_saggital_reslice;
	}

	delete ui;
}

void MainWindow::init_Parameters()
{

	view_saggital = NULL;
	view_cornoal  = NULL;
	view_axial    = NULL;
	
	view_axial_reslice = NULL;
	view_coronal_reslice = NULL;
	view_saggital_reslice = NULL;

	
	bold_win      = NULL;
	new_3d_view   = NULL;


	//actor for mask
	axial_mask_Actor   = NULL;
	coronal_mask_Actor = NULL;
	sagittal_mask_Actor= NULL;

	//init opicity slider bar
	this->ui->set_opicity_Slider->setRange(0,100);
	this->ui->set_opicity_Slider->setValue(50);
	//init strip value slider bar
	this->ui->strip_val_Slider->setRange(0,1500);
	this->ui->strip_val_Slider->setValue(1500);
}

void MainWindow::on_click_load()
{
	disconnect(this->ui->view_vol_Slider,SIGNAL(valueChanged(int)),this,SLOT(on_slider_volume_move(int)));
	this->file_name_list = 
		QFileDialog::getOpenFileNames(this,
		tr("open log file"),"./",tr("(*)"));
	if (file_name_list.isEmpty()){return;}

	for (int i=0;i<file_name_list.size();i++)
	{
		this->file_name = file_name_list[i];
		std::string filenamexx = this->file_name.toStdString();
		print_Info("Load Image:  ",file_name);

		Image_Convert_Base* img_reader = new Image_Convert_Base;
		img_reader->SetFileName(filenamexx);

		img_view_base_Type temp_img;
		temp_img.first = filenamexx;
		temp_img.second = img_reader->GetOutput();
		delete img_reader;

		print_Info("Image Scalar Number: ",QString::number(temp_img.second->GetNumberOfScalarComponents()));

		int* dim = new int[3];
		temp_img.second->GetDimensions(dim);
		print_Info("Dimension: ",QString::number(*dim),QString::number(dim[1]),QString::number(dim[2]));

		double* ori = new double[3];
		temp_img.second->GetOrigin(ori);
		print_Info("Origin: ",QString::number(*ori),QString::number(ori[1]),QString::number(ori[2]));
		int* ex = new int [6];
		temp_img.second->GetExtent(ex);
		print_Info("extent: ",QString::number(*ex),QString::number(ex[1]),QString::number(ex[2]));
		print_Info("extent: ",QString::number(ex[3]),QString::number(ex[4]),QString::number(ex[5]));
		
		this->img_to_view = temp_img;
		this->data_container.push_back(temp_img);
	}

	this->set_slider_volume_range(this->data_container.size());

	connect(this->ui->view_vol_Slider,SIGNAL(valueChanged(int)),this,SLOT(on_slider_volume_move(int)));
}



void MainWindow::on_click_show()
{
	if (this->img_to_view.first.empty())
	{
		return;
	}

	if (view_axial != NULL)
	{
		delete view_axial;
		delete view_cornoal;
		delete view_saggital;
	}
	if (view_axial_reslice != NULL)
	{
		delete view_axial_reslice;
		delete view_coronal_reslice;
		delete view_saggital_reslice;
	}
	view_saggital_reslice = new reslice_view_base(this->ui->sagittal_view_widget->GetRenderWindow(),'s');
	view_coronal_reslice  = new reslice_view_base(this->ui->coronal_view_widget->GetRenderWindow(),'c');
	view_axial_reslice    = new reslice_view_base(this->ui->axial_view_widget->GetRenderWindow(),'a');

	////add mask first
	view_coronal_reslice->Set_View_Img(this->img_to_view.second);
	//view_cornoal_reslice->Set_Mask_Img(this->mask_img.second);
	view_coronal_reslice->RenderView();
	view_coronal_reslice->Set_View_Img(this->img_to_view.second);
	//view_cornoal_reslice->Set_Mask_Img(this->mask_img.second);
	view_coronal_reslice->RenderView();
	view_saggital_reslice->Set_View_Img(this->img_to_view.second);
//	view_saggital_reslice->Set_Mask_Img(this->mask_img.second);
	view_saggital_reslice->RenderView();
	view_axial_reslice->Set_View_Img(this->img_to_view.second);
//	view_axial_reslice->Set_Mask_Img(this->mask_img.second);
	view_axial_reslice->RenderView();
}

void MainWindow::on_click_show3d()
{
	if (this->img_to_view.first.empty())
	{
		return;
	}
	if (new_3d_view != NULL)
	{
		delete new_3d_view;
	}
	new_3d_view = new construct_base(this->ui->ster_3d_view_widget->GetRenderWindow());
	new_3d_view->Set_Input_Img(this->img_to_view.second);
	new_3d_view->Re_Construct();
}

void MainWindow::refresh_view()
{
	//refresh 2d views
	if (view_axial !=NULL)
	{
		view_axial->Set_View_Img(this->img_to_view.second);
		view_axial->RenderView(10);
		view_cornoal->Set_View_Img(this->img_to_view.second);
		view_cornoal->RenderView(13);
		view_saggital->Set_View_Img(this->img_to_view.second);
		view_saggital->RenderView(10);
	}
	if (view_axial_reslice!=NULL)
	{
		view_axial_reslice->Set_View_Img(this->img_to_view.second);
		view_axial_reslice->RenderView();
		view_coronal_reslice->Set_View_Img(this->img_to_view.second);
		view_coronal_reslice->RenderView();
		view_saggital_reslice->Set_View_Img(this->img_to_view.second);
		view_saggital_reslice->RenderView();
	}
	
	
	//refresh 3d view
	new_3d_view->Set_Input_Img(this->img_to_view.second);
	new_3d_view->Re_Construct();
}

void MainWindow::on_click_bold()
{
	bold_win = new SubWidgetParadigmInBold;
	bold_win->setWindowModality(Qt::ApplicationModal);
	bold_win->show();
}

void MainWindow::on_click_add_mask_file()
{
	print_Info("add a ","mask file");
	this->file_name = 
		QFileDialog::getOpenFileName(this,
		tr("open log file"),"./",tr("(*)"));
	if (file_name.isEmpty()){return;}

	std::string filenamexx = this->file_name.toStdString();
	print_Info("Load Mask Image:  ",file_name);

	Image_Convert_Base* img_reader = new Image_Convert_Base;
	img_reader->SetFileName(filenamexx);

	img_view_base_Type temp_img;
	temp_img.first = filenamexx;
	temp_img.second = img_reader->GetOutput();

	//reslice mask image to adjust 
	vtkSmartPointer<vtkImageReslice> resample =
		vtkSmartPointer<vtkImageReslice>::New();
	resample->SetInput(temp_img.second);
	resample->SetOutputSpacing(this->img_to_view.second->GetSpacing());
	resample->SetOutputExtent(this->img_to_view.second->GetExtent());
	resample->SetInterpolationModeToLinear();
	resample->Update();

	this->mask_img = temp_img;

	print_Info("Load Mask Image:  ",
		QString::number(*temp_img.second->GetDimensions()),
		QString::number(*temp_img.second->GetDimensions()+1),
		QString::number(*temp_img.second->GetDimensions()+2));
}
void MainWindow::on_click_mask()
{	
	print_Info("show ","mask");

	if (this->mask_img.first.empty())
	{
		print_Info("Please add a ","mask");
		return;
	}

	view_saggital_reslice->Set_Mask_Img(this->mask_img.second);
	view_coronal_reslice->Set_Mask_Img(this->mask_img.second);
	view_axial_reslice->Set_Mask_Img(this->mask_img.second);

	view_saggital_reslice->RenderView();
	view_coronal_reslice->RenderView();
	view_axial_reslice->RenderView();
}
void MainWindow::on_click_del_mask()
{
	print_Info("del a ","mask");
	if (mask_img.first.empty())
	{
		print_Info("No Mask ","to del");
		return;
	}

	//remove from view
	this->view_axial_reslice->RemoveMask();
	this->view_coronal_reslice->RemoveMask();
	this->view_saggital_reslice->RemoveMask();

	//delete mask from data container
	this->mask_img.first.clear();
	this->mask_img.second = NULL;
}

void MainWindow::on_slider_volume_move(int posxx)
{
	int pos = this->ui->view_vol_Slider->value();
	std::cout<<"position is : "<<pos<<std::endl;
	print_Info("Position is : ", QString::number(pos));

	if (this->data_container.size()==0)
	{
		return;
	}
	else
	{
		this->img_to_view = this->data_container[pos-1];
		print_Info("Current image is: ",img_to_view.first.data());
		this->refresh_view();
	}
}


void MainWindow::on_slider_opicity_move(int posxx)
{
	int pos = this->ui->set_opicity_Slider->value();
	
	double opicity = double(pos)/100;
	print_Info("Opicity is : ", QString::number(opicity));

	if (this->new_3d_view == NULL)   {return;}

	this->new_3d_view->Set_Opicity(opicity);
	this->ui->ster_3d_view_widget->GetRenderWindow()->Render();
}
void MainWindow::on_slider_strip_val_move(int)
{
	int pos = this->ui->strip_val_Slider->value();
	double strip_val = pos;
	print_Info("strip value is : ", QString::number(strip_val));

	if (this->new_3d_view == NULL)   {return;}

	this->new_3d_view->Set_Cube_Value(strip_val);
	this->new_3d_view->Re_Construct();
	this->ui->ster_3d_view_widget->GetRenderWindow()->Render();
}

void MainWindow::mouse_Wheel_move(QWheelEvent *e)
{
	int numDegree = e->delta()/8;
	int numSteps = numDegree/15;
	std::cout<<numDegree<<std::endl;
	print_Info("axial slice is: ",QString::number(this->view_axial->Slice_Position));
	print_Info("cornoal slice is: ",QString::number(this->view_cornoal->Slice_Position));
	print_Info("saggital slice is: ",QString::number(this->view_saggital->Slice_Position));
}

//information scroll in info panel
void MainWindow::info_Panel_Scroll()
{
	QTextCursor text_cursor =  ui->info_Panel->textCursor();
	text_cursor.movePosition(QTextCursor::End);
	ui->info_Panel->setTextCursor(text_cursor);
}
void MainWindow::print_Info(QString in,QString x)
{
	QString temp = in;
	temp.append(x);
	temp.append("\n");
	ui->info_Panel->insertPlainText(temp);
}
void MainWindow::print_Info(QString in,QString x,QString y,QString z)
{
	QString temp = in;
	temp.append(x);
	temp.append(y);
	temp.append(z);
	temp.append("\n");
	ui->info_Panel->insertPlainText(temp);
}
//basic method to calculate center of the image
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
void MainWindow::set_slider_volume_range(int num)
{
	this->ui->view_vol_Slider->setMinimum(1);
	this->ui->view_vol_Slider->setMaximum(num);

	//set display number
	this->ui->min_vol_Lab->setText("1");
	this->ui->max_vol_Lab->setText(QString::number(this->data_container.size()));
}
void MainWindow::set_data_container(vector<img_view_base_Type >  da_con)
{
	if (da_con.empty())
	{
		return;
	}
	else
	{
		this->data_container = da_con;
	}
}






