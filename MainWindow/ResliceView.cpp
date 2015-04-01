#include "ResliceView.h"



reslice_view_base::reslice_view_base(vtkRenderWindow* winx,char a)
{
	// init para
	this->Set_Direction(a);
	this->Set_Window(winx);
	this->slice_n = 0;

	this->img_to_mask = NULL;
	this->img_to_view = NULL;

	this->dimensions = NULL;
	//this->view_dirX = NULL;
	//this->view_dirY = NULL;
	//this->view_dirZ = NULL;
	this->img_to_mask = NULL;
	this->mask_actor  = NULL;
	this->new_render  = NULL;
	this->Interactor      = NULL;
	this->InteractorStyle = NULL;

	//map vtkaction and qt signal
	m_Connections_mouse_back = vtkSmartPointer<vtkEventQtSlotConnect>::New();
	m_Connections_mouse_back->Connect(this->view_window->GetInteractor(),
		vtkCommand::MouseWheelBackwardEvent,this,SLOT(on_scroll_mouse_back(vtkObject*)));

	m_Connections_mouse_forward = vtkSmartPointer<vtkEventQtSlotConnect>::New();
	m_Connections_mouse_forward->Connect(this->view_window->GetInteractor(),
		vtkCommand::MouseWheelForwardEvent,this,SLOT(on_scroll_mouse_forward(vtkObject*)));

	m_Connections_mouse_lft_click = vtkSmartPointer<vtkEventQtSlotConnect>::New();
	m_Connections_mouse_lft_click->Connect(this->view_window->GetInteractor(),
		vtkCommand::LeftButtonPressEvent, this, SLOT(on_click_mouse_lft(vtkObject*)));

	//renderer init
	new_render	 = vtkSmartPointer<vtkRenderer>::New();
	actor		 = vtkSmartPointer<vtkImageActor>::New();
	mask_actor   = vtkSmartPointer<vtkImageActor>::New();
	WindowLevel1 = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
	WindowLevel2 = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
	this->InteractorStyle = vtkSmartPointer<reslice_interactor_style>::New();
	this->Interactor      = vtkSmartPointer<vtkRenderWindowInteractor>::New();

	//this->view_window->AddRenderer(this->new_render);
	//new_render->AddActor(this->actor);
	////set default interact null
	vtkSmartPointer<reslice_interactor_style> new_act_style = 
		vtkSmartPointer<reslice_interactor_style>::New();
	this->view_window->GetInteractor()->SetInteractorStyle(new_act_style);
	this->InstallPipeline();

	this->actor->SetOpacity(1);
	this->mask_actor->SetOpacity(0.5);
}

reslice_view_base::~reslice_view_base()
{
	m_Connections_mouse_back->Disconnect(this->view_window->GetInteractor(),
		vtkCommand::MouseWheelBackwardEvent,this,SLOT(on_scroll_mouse_back(vtkObject*)));
	m_Connections_mouse_forward->Disconnect(this->view_window->GetInteractor(),
		vtkCommand::MouseWheelForwardEvent,this,SLOT(on_scroll_mouse_forward(vtkObject*)));
	//delete[] this->dimensions;
	//delete[] this->view_dirX;
	//delete[] this->view_dirY;
	//delete[] this->view_dirZ;
}
void reslice_view_base::InstallPipeline()
{
	if (this->view_window && this->new_render)
	{
		this->view_window->AddRenderer(this->new_render);
	}

	if (this->new_render && this->actor)
	{
		this->new_render->AddViewProp(this->actor);
	}

	if (this->actor && this->WindowLevel1)
	{
		this->actor->SetInput(this->WindowLevel1->GetOutput());
	}
}



void reslice_view_base::Set_View_Img(vtkSmartPointer<vtkImageData> img)
{
	this->img_to_view = vtkSmartPointer<vtkImageData>::New();
	this->img_to_view = img;
	this->dimensions = new int[3];
	this->img_to_view->GetDimensions(this->dimensions);
	std::cout<<"dimension is :"<<dimensions[0]<<dimensions[1]<<dimensions[2]<<std::endl;
	this->calculate_img_center(img_to_view);
}

void reslice_view_base::Set_Mask_Img(vtkSmartPointer<vtkImageData> img)
{
	this->img_to_mask = vtkSmartPointer<vtkImageData>::New();
	this->img_to_mask = img;

	//vtkSmartPointer<vtkImageBlend> imageBlend = vtkSmartPointer<vtkImageBlend>::New();
	//imageBlend->SetInput(0,img_to_mask);
	//imageBlend->SetInput(1,img_to_view);
	//imageBlend->SetOpacity(0,0.5);
	//imageBlend->SetOpacity(1,0.5);
	//imageBlend->Update();
	//vtkSmartPointer<vtkImageData> temp = vtkSmartPointer<vtkImageData>::New();
	//img_to_view->DeepCopy(imageBlend->GetOutput());
}

void reslice_view_base::RemoveMask()
{
	if (this->img_to_mask == NULL)
	{
		return;
	}

	//uninsatall pipeline
	if (this->new_render && this->mask_actor)
	{
		this->new_render->RemoveViewProp(this->mask_actor);
	}

	if (this->mask_actor && this->WindowLevel2)
	{
		this->mask_actor->SetInput(NULL);
	}

	//delete this mask
	this->img_to_mask = NULL;
}

void reslice_view_base::Set_Window(vtkRenderWindow* win)
{
	this->view_window = vtkSmartPointer<vtkRenderWindow>::New();
	this->view_window = win;
}

void reslice_view_base::RenderView()
{

	this->reslice = vtkSmartPointer<vtkImageReslice>::New();
	this->reslice->SetInput(this->img_to_view);
	this->reslice->SetOutputDimensionality(2);
	this->reslice->SetResliceAxesDirectionCosines(this->view_dirX,this->view_dirY,this->view_dirZ);
	this->reslice->SetResliceAxesOrigin(center);
	this->reslice->SetInterpolationModeToLinear();
	this->WindowLevel1->SetInput(this->reslice->GetOutput());

	if (this->img_to_mask != NULL)
	{
		//install pipline here
		if (this->new_render && this->mask_actor)
		{
			this->new_render->AddViewProp(this->mask_actor);
		}
		if (this->mask_actor && this->WindowLevel2)
		{
			this->mask_actor->SetInput(this->WindowLevel2->GetOutput());
		}
		this->mask_reslice = vtkSmartPointer<vtkImageReslice>::New();
		this->mask_reslice->SetInput(this->img_to_mask);
		this->mask_reslice->SetOutputDimensionality(2);
		this->mask_reslice->SetResliceAxesDirectionCosines(this->view_dirX,this->view_dirY,this->view_dirZ);
		this->mask_reslice->SetResliceAxesOrigin(center);
		this->mask_reslice->SetInterpolationModeToLinear();
		this->WindowLevel2->SetInput(this->mask_reslice->GetOutput());
	}


	//vtkSmartPointer<vtkImageData> input_temp = 
	//	vtkImageData::SafeDownCast(this->WindowLevel1->GetInput());

	this->new_render->ResetCamera();
	this->view_window->Render();
}


//slots: when mouse wheel scroll back, next slice of image show
void reslice_view_base::on_scroll_mouse_back(vtkObject* obj)
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
			if (this->slice_n>this->extent_m[5])
			{
				this->slice_n = this->extent_m[4];
			}
			else if (this->slice_n<extent_m[4])
			{
				this->slice_n = this->extent_m[5];
			}
			center[2] = origin[2]+spacing[2]*this->slice_n;
			//center[2] = spacing[2]*this->slice_n;
			break;
		}
	case 'c':
		{
			this->slice_n++;
			if (this->slice_n>this->extent_m[3])
			{
				this->slice_n = this->extent_m[2];
			}
			else if (this->slice_n<extent_m[2])
			{
				this->slice_n = this->extent_m[3];
			}
			center[1] = origin[1]+spacing[1]*this->slice_n;
			//center[1] = spacing[1]*this->slice_n;
			break;
		}
	case 's':
		{
			this->slice_n++;
			if (this->slice_n>this->extent_m[1])
			{
				this->slice_n = this->extent_m[0];
			}
			else if (this->slice_n<extent_m[0])
			{
				this->slice_n = this->extent_m[1];
			}
			center[0] = origin[0]+spacing[0]*this->slice_n;
			//center[0] = spacing[0]*this->slice_n;
			break;
		}
	default:
		{
			this->slice_n++;
			if (this->slice_n>this->extent_m[1])
			{
				this->slice_n = this->extent_m[0];
			}
			else if (this->slice_n<extent_m[0])
			{
				this->slice_n = this->extent_m[1];
			}
			center[0] = origin[0]+spacing[0]*this->slice_n;
			//center[0] = spacing[0]*this->slice_n;
			break;
		}
	}
	this->RenderView();
}
void reslice_view_base::on_scroll_mouse_forward(vtkObject* obj)
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
			if (this->slice_n>this->extent_m[5])
			{
				this->slice_n = this->extent_m[4];
			}
			else if (this->slice_n<extent_m[4])
			{
				this->slice_n = this->extent_m[5];
			}
			center[2] = origin[2]+spacing[2]*this->slice_n;
			//center[2] = spacing[2]*this->slice_n;
			break;
		}
	case 'c':
		{
			this->slice_n--;
			if (this->slice_n>this->extent_m[3])
			{
				this->slice_n = this->extent_m[2];
			}
			else if (this->slice_n<extent_m[2])
			{
				this->slice_n = this->extent_m[3];
			}
			center[1] = origin[1]+spacing[1]*this->slice_n;
			//center[1] = spacing[1]*this->slice_n;
			break;
		}
	case 's':
		{
			this->slice_n--;
			if (this->slice_n>this->extent_m[1])
			{
				this->slice_n = this->extent_m[0];
			}
			else if (this->slice_n<extent_m[0])
			{
				this->slice_n = this->extent_m[1];
			}
			center[0] = origin[0]+spacing[0]*this->slice_n;
			//center[0] = spacing[0]*this->slice_n;
			break;
		}
	default:
		{
			this->slice_n--;
			if (this->slice_n>this->extent_m[1])
			{
				this->slice_n = this->extent_m[0];
			}
			else if (this->slice_n<extent_m[0])
			{
				this->slice_n = this->extent_m[1];
			}
			center[0] = origin[0]+spacing[0]*this->slice_n;
			break;
		}
	}
	this->RenderView();
}
void reslice_view_base:: on_click_mouse_lft(vtkObject* obj)
{
	vtkSmartPointer<vtkRenderWindowInteractor> iren = 
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//get interactor first
	iren = vtkRenderWindowInteractor::SafeDownCast(obj);

	int eve_pos[2];
	iren->GetEventPosition(eve_pos);
	
	std::cout<<"click mouse left button "<<eve_pos[0]<<" , "<<eve_pos[1]<<std::endl;
}
// private method: set view direction
void reslice_view_base::Set_Direction(char x)
{
	this->direction = x ;
	switch (x)
	{
	case 'a':
		{
			double axialX[3] = {-1,0,0};
			double axialY[3] = {0,-1,0};
			double axialZ[3] = {0,0,-1};

			for (int i=0;i<3;i++)
			{
				this->view_dirX[i] = axialX[i];
				this->view_dirY[i] = axialY[i];
				this->view_dirZ[i] = axialZ[i];
			}

			break;
		}
	case 'c':
		{
			double coronalX[3] = {-1,0,0};
			double coronalY[3] = {0,0,1};
			double coronalZ[3] = {0,1,0};

			for (int i=0;i<3;i++)
			{
				this->view_dirX[i] = coronalX[i];
				this->view_dirY[i] = coronalY[i];
				this->view_dirZ[i] = coronalZ[i];
			}
			break;
		}
	case 's':
		{
			double sagittalX[3] = {0,1,0};
			double sagittalY[3] = {0,0,1};
			double sagittalZ[3] = {1,0,0};
			for (int i=0;i<3;i++)
			{
				this->view_dirX[i] = sagittalX[i];
				this->view_dirY[i] = sagittalY[i];
				this->view_dirZ[i] = sagittalZ[i];
			}
			break;
		}
	default:
		{
			double axialX[3] = {1,0,0};
			double axialY[3] = {0,1,0};
			double axialZ[3] = {0,0,1};

			for (int i=0;i<3;i++)
			{
				this->view_dirX[i] = axialX[i];
				this->view_dirY[i] = axialY[i];
				this->view_dirZ[i] = axialZ[i];
			}
			break;
		}
	}
}
//private method: calculate imge center of a 3D image
double* reslice_view_base::calculate_img_center(vtkSmartPointer<vtkImageData> img)
{

	img->GetWholeExtent(extent_m);
	img->GetSpacing(spacing);
	img->GetOrigin(origin);

	double center[3];
	for (int i=0;i<3;i++)
	{
		center[i] = origin[i]+spacing[i]*0.5*(extent_m[i]+extent_m[i+1]);//
		//center[i] = origin[i]+spacing[i]*0.5*(dimensions[i]-1);
		this->center[i] = center[i];
	}

	return center;
}


//this line is badly need to inhert a new class
//vtkObjectFactory.h must include!
vtkStandardNewMacro(reslice_interactor_style);



