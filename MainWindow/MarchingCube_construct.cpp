#include "MarchingCube_construct.h"


construct_base::construct_base(vtkSmartPointer<vtkRenderWindow> win)
{
	this->Set_View_Window(win);
	actor = vtkSmartPointer<vtkActor>::New();
	marchingCubes = vtkSmartPointer<vtkMarchingCubes>::New();
	new_render  = vtkSmartPointer<vtkRenderer>::New();
	this->view_window->AddRenderer(new_render);  
	this->new_render->AddActor(actor);
	this->opicity = 1;
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
	//add mapper
	vtkSmartPointer<vtkDataSetMapper> mapper = 
		vtkSmartPointer<vtkDataSetMapper>::New();
	mapper->SetInput(marchingCubes->GetOutput());
	actor->SetMapper(mapper);
	this->view_window->Render();
}


void construct_base::Set_Opicity(float op)
{
	this->opicity = op;
	actor->GetProperty()->SetOpacity(this->opicity);
}


void construct_base::Set_Input_Img(vtkSmartPointer<vtkImageData> img)
{
	this->img_to_Construct = vtkSmartPointer<vtkImageData> ::New();
	this->img_to_Construct = img;

	marchingCubes->SetInput(this->img_to_Construct);
	marchingCubes->SetValue(0,500);
	//marchingCubes->
	marchingCubes->Update();
}


void construct_base::Set_View_Window(vtkSmartPointer<vtkRenderWindow> win)
{
	this->view_window = vtkSmartPointer<vtkRenderWindow>::New();
	this->view_window = win	;
}

