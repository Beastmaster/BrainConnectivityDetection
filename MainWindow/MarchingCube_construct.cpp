#include "MarchingCube_construct.h"


construct_base::construct_base(vtkSmartPointer<vtkRenderWindow> win)
{
	this->Set_View_Window(win);
	actor = vtkSmartPointer<vtkActor>::New();
	new_render  = vtkSmartPointer<vtkRenderer>::New();
	marchingCubes 	= vtkSmartPointer<vtkMarchingCubes>::New();
	this->view_window->AddRenderer(new_render);  
	this->new_render->AddActor(actor);
	this->opicity = 1;
	this->value_cube =500;
}
construct_base::construct_base()
{
	this->view_window = NULL;
	marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
	new_render  = vtkSmartPointer<vtkRenderer>::New();
	this->opicity = 1;
	if (this->view_window == NULL)
	{
		return;
	}
	this->view_window->AddRenderer(new_render);  
}
construct_base::~construct_base()
{

}

void construct_base::Re_Construct()
{
	if (this->img_to_Construct->GetDataDimension()!=3)
	{
		return;
	}
#if VTK_MAJOR_VERSION <= 5
	marchingCubes->SetInput(this->img_to_Construct);
#else
	marchingCubes->SetInputData(this->img_to_Construct);
#endif
	//marchingCubes->SetNumberOfContours(2);
	marchingCubes->SetValue(0,this->value_cube);
	//marchingCubes update
	marchingCubes->Update();
	//add mapper
	vtkSmartPointer<vtkDataSetMapper> mapper = 
		vtkSmartPointer<vtkDataSetMapper>::New();
#if VTK_MAJOR_VERSION <= 5
	mapper->SetInput(marchingCubes->GetOutput());
#else
	mapper->SetInputData(marchingCubes->GetOutput());
#endif
	mapper->ScalarVisibilityOff();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(1.0,0.0,0.0);
	this->view_window->Render();
}


void construct_base::Set_Opicity(double op)
{
	this->opicity = op;
	actor->GetProperty()->SetOpacity(this->opicity);
}

void construct_base::Set_Cube_Value(double value_in)
{
	this->value_cube = value_in;
}

void construct_base::Set_Input_Img(vtkSmartPointer<vtkImageData> img)
{
	this->img_to_Construct = vtkSmartPointer<vtkImageData> ::New();
	this->img_to_Construct = img;
}



void construct_base::Set_View_Window(vtkSmartPointer<vtkRenderWindow> win)
{
	this->view_window = vtkSmartPointer<vtkRenderWindow>::New();
	this->view_window = win	;
}

