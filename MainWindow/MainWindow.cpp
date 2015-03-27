#include "MainWindow.h"
#include "ui_MainWindow.h"


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
	}
	if (this->view_cornoal!=NULL)
	{
		delete view_cornoal;
	}
	if (this->view_saggital!=NULL)
	{
		delete view_saggital;
	}
	delete ui;
}

void MainWindow::init_Parameters()
{
	view_saggital = NULL;
	view_cornoal  = NULL;
	view_axial    = NULL;
	bold_win      = NULL;
	new_3d_view   = NULL;

	////add render to view
	//view_saggital = new slice_view_base(this->ui->sagittal_view_widget->GetRenderWindow(),'s');
	//view_cornoal  = new slice_view_base(this->ui->coronal_view_widget->GetRenderWindow(),'c');
	//view_axial    = new slice_view_base(this->ui->axial_view_widget->GetRenderWindow(),'a');

	//init opicity slider bar
	this->ui->set_opicity_Slider->setRange(0,100);
	this->ui->set_opicity_Slider->setValue(50);
}

void MainWindow::on_click_load()
{
	disconnect(this->ui->view_vol_Slider,SIGNAL(valueChanged(int)),this,SLOT(on_slider_volume_move(int)));
	this->file_name = 
		QFileDialog::getOpenFileName(this,
		tr("open log file"),"./",tr("(*)"));
	if (file_name.isEmpty()){return;}

	std::string filenamexx = this->file_name.toStdString();
	print_Info("Load Image:  ",file_name);

	Image_Convert_Base* img_reader = new Image_Convert_Base;
	img_reader->SetFileName(filenamexx);

	img_view_base_Type temp_img;
	temp_img.first = filenamexx;
	temp_img.second = img_reader->GetOutput();
	delete img_reader;

	print_Info("Image Scalar Number: ",QString::number(temp_img.second->GetNumberOfScalarComponents()));

	this->img_to_view = temp_img;
	this->data_container.push_back(temp_img);
	this->set_slider_volume_range(this->data_container.size());
	this->ui->min_vol_Lab->setText("1");
	this->ui->max_vol_Lab->setText(QString::number(this->data_container.size()));

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
	view_saggital = new slice_view_base(this->ui->sagittal_view_widget->GetRenderWindow(),'s');
	view_cornoal  = new slice_view_base(this->ui->coronal_view_widget->GetRenderWindow(),'c');
	view_axial    = new slice_view_base(this->ui->axial_view_widget->GetRenderWindow(),'a');

	view_axial->Set_View_Img(this->img_to_view.second);
	view_axial->RenderView(0);
	view_cornoal->Set_View_Img(this->img_to_view.second);
	view_cornoal->RenderView(0);
	view_saggital->Set_View_Img(this->img_to_view.second);
	view_saggital->RenderView(0);
	ui->gridLayout->setEnabled(true);
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
	view_axial->Set_View_Img(this->img_to_view.second);
	view_axial->RenderView(10);
	view_cornoal->Set_View_Img(this->img_to_view.second);
	view_cornoal->RenderView(13);
	view_saggital->Set_View_Img(this->img_to_view.second);
	view_saggital->RenderView(10);
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

	this->mask_img = temp_img;
}
void MainWindow::on_click_mask()
{	
	print_Info("show ","mask");

	vtkSmartPointer<vtkWindowLevelLookupTable> lookup_table = 
		vtkSmartPointer<vtkWindowLevelLookupTable>::New();
	vtkSmartPointer<vtkImageMapToColors> color_map = 
		vtkSmartPointer<vtkImageMapToColors>::New();
	
	//build up look up table
	lookup_table->SetNumberOfTableValues(3);
	lookup_table->SetRange(0.0,300.0);
	lookup_table->SetValueRange(0,1);
	lookup_table->SetRampToLinear();
	lookup_table->SetSaturationRange(0.0,0.0);
	//lookup_table->SetTableValue(0,0.0,0.0,0.0,0.0);//label 0 is transparent
	//lookup_table->SetTableValue(1,0.0,1.0,0.0,1.0);//label 1 is opaque and green
	//lookup_table->SetTableValue(2,0.0,1.0,0.0,0.0);//label 2 is opaque and green
	lookup_table->Build();
	//setup color map
	color_map->SetLookupTable(lookup_table);
	color_map->SetInput(this->mask_img.second);
	color_map->Update();

	vtkSmartPointer<vtkImageBlend> imBlender = 
		vtkSmartPointer<vtkImageBlend>::New();
	imBlender->SetOpacity(0,1);
	imBlender->SetOpacity(1,0.3);
	imBlender->AddInput(this->img_to_view.second);
	imBlender->AddInput(color_map->GetOutput());
	imBlender->Update();
	vtkSmartPointer<vtkImageData> temp = 
		vtkSmartPointer<vtkImageData>::New();
	temp = imBlender->GetOutput();

	view_axial->Set_View_Img(temp);
	view_cornoal->Set_View_Img(temp);
	view_saggital->Set_View_Img(temp);
}
void MainWindow::on_click_del_mask()
{
	print_Info("del a ","mask");

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








//--------new slice view class: new view method------//
slice_view_base::slice_view_base(vtkRenderWindow* winx,char a)
{
	// init para
	this->Set_Direction(a);
	this->Set_Window(winx);
	this->slice_n = 0;
	
	this->dimensions = NULL;
	this->view_dirX = NULL;
	this->view_dirY = NULL;
	this->view_dirZ = NULL;

	//map vtkaction and qt signal
	this->m_Connections_mouse_back = vtkSmartPointer<vtkEventQtSlotConnect>::New();
	m_Connections_mouse_back = vtkSmartPointer<vtkEventQtSlotConnect>::New();
	m_Connections_mouse_back->Connect(this->view_window->GetInteractor(),
		vtkCommand::MouseWheelBackwardEvent,this,SLOT(on_scroll_mouse_back(vtkObject*)));
	this->m_Connections_mouse_forward = vtkSmartPointer<vtkEventQtSlotConnect>::New();
	m_Connections_mouse_forward = vtkSmartPointer<vtkEventQtSlotConnect>::New();
	m_Connections_mouse_forward->Connect(this->view_window->GetInteractor(),
		vtkCommand::MouseWheelForwardEvent,this,SLOT(on_scroll_mouse_forward(vtkObject*)));

	//renderer init
	new_render = vtkSmartPointer<vtkRenderer>::New();
	this->view_window->AddRenderer(this->new_render);
	//set default interact null
	vtkSmartPointer<new_interactor_style> new_act_style = 
		vtkSmartPointer<new_interactor_style>::New();
	this->view_window->GetInteractor()->SetInteractorStyle(new_act_style);

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
//destructor method: use vtksmartpointer so no need to delete
slice_view_base::~slice_view_base()
{
	m_Connections_mouse_back->Disconnect(this->view_window->GetInteractor(),
		vtkCommand::MouseWheelBackwardEvent,this,SLOT(on_scroll_mouse_back(vtkObject*)));
	m_Connections_mouse_forward->Disconnect(this->view_window->GetInteractor(),
		vtkCommand::MouseWheelForwardEvent,this,SLOT(on_scroll_mouse_forward(vtkObject*)));
	delete[] this->dimensions;
	delete[] this->view_dirX;
	delete[] this->view_dirY;
	delete[] this->view_dirZ;
}
//render the x th slice in a 3D image
int slice_view_base::RenderView(int x)
{
	this->slice_n = x;
	img_viewer2->SetSlice(this->slice_n);
	this->img_viewer2->Render();
	this->Slice_Position = x;
	return x;
}
// private method: set view direction
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
// default method: add imag to view to view widget
void slice_view_base::Set_View_Img(vtkSmartPointer<vtkImageData> img)
{
	this->img_to_view = img;
	this->img_viewer2->SetInput(img_to_view);
	this->dimensions = new int[3];
	this->img_to_view->GetDimensions(this->dimensions);
	std::cout<<"dimension is :"<<dimensions[0]<<dimensions[1]<<dimensions[2]<<std::endl;
}
//private method: add widget window to render
void slice_view_base::Set_Window(vtkRenderWindow* win)
{
	this->view_window = vtkSmartPointer<vtkRenderWindow>::New();
	this->view_window = win;
}
//private method: calculate imge center of a 3D image
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
//slots: when mouse wheel scroll back, next slice of image show
void slice_view_base::on_scroll_mouse_back(vtkObject* obj)
{
	vtkSmartPointer<vtkRenderWindowInteractor> iren = 
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren = vtkRenderWindowInteractor::SafeDownCast(obj);

	std::cout<<"direct  "<<this->direction<<"  call mouse scroll  "<<this->slice_n<<std::endl;

	switch(this->direction)
	{
	case 'a':
		{
			this->slice_n++;
			if (this->slice_n>this->dimensions[2])
			{
				this->slice_n = 0;
			}
			else if (this->slice_n<0)
			{
				this->slice_n = this->dimensions[2];
			}
			break;
		}
	case 'c':
		{
			this->slice_n++;
			if (this->slice_n>this->dimensions[0])
			{
				this->slice_n = 0;
			}
			else if (this->slice_n<0)
			{
				this->slice_n = this->dimensions[0];
			}
			break;
		}
	case 's':
		{
			this->slice_n++;
			if (this->slice_n>this->dimensions[1])
			{
				this->slice_n = 0;
			}
			else if (this->slice_n<0)
			{
				this->slice_n = this->dimensions[1];
			}
			break;
		}
	default:
		{
			this->slice_n++;
			if (this->slice_n>this->dimensions[0])
			{
				this->slice_n = 0;
			}
			else if (this->slice_n<0)
			{
				this->slice_n = this->dimensions[0];
			}
			break;
		}
	}
	this->RenderView(slice_n);
}
void slice_view_base::on_scroll_mouse_forward(vtkObject* obj)
{
	vtkSmartPointer<vtkRenderWindowInteractor> iren = 
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	iren = vtkRenderWindowInteractor::SafeDownCast(obj);

	std::cout<<"direct  "<<this->direction<<"  call mouse scroll  "<<this->slice_n<<std::endl;

	switch(this->direction)
	{
	case 'a':
		{
			this->slice_n--;
			if (this->slice_n>this->dimensions[2])
			{
				this->slice_n = 0;
			}
			else if (this->slice_n<0)
			{
				this->slice_n = this->dimensions[2];
			}
			break;
		}
	case 'c':
		{
			this->slice_n--;
			if (this->slice_n>this->dimensions[0])
			{
				this->slice_n = 0;
			}
			else if (this->slice_n<0)
			{
				this->slice_n = this->dimensions[0];
			}
			break;
		}
	case 's':
		{
			this->slice_n--;
			if (this->slice_n>this->dimensions[1])
			{
				this->slice_n = 0;
			}
			else if (this->slice_n<0)
			{
				this->slice_n = this->dimensions[1];
			}
			break;
		}
	default:
		{
			this->slice_n--;
			if (this->slice_n>this->dimensions[0])
			{
				this->slice_n = 0;
			}
			else if (this->slice_n<0)
			{
				this->slice_n = this->dimensions[0];
			}
			break;
		}
	}
	this->RenderView(slice_n);
}

//this line is badly need to inhert a new class
//vtkObjectFactory.h must include!
vtkStandardNewMacro(new_interactor_style);
