#ifndef  __RESLICEVIEW_H_
#define  __RESLICEVIEW_H_

#include <QApplication>
//qt + vtk include
#include "QVTKWidget.h"
//qt + vtk connect signal and slot
#include "vtkCommand.h"
#include "vtkEventQtSlotConnect.h"
//vtk reslice
#include "vtkImageReslice.h"
#include "vtkWindowLevelLookupTable.h"
#include "vtkImageMapToColors.h"
#include "vtkImageActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
//change vtk interact handle
#include "vtkInteractorStyleImage.h"
#include "vtkObject.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"

//blend two images
#include "vtkImageBlend.h"


class reslice_view_base : QObject
{
	Q_OBJECT
public:
	explicit reslice_view_base(QWidget *parent = 0);
	reslice_view_base(vtkRenderWindow*,char);
	~reslice_view_base();

	void SetSlice(int x) {this->slice_n = x;};
	void Set_View_Img(vtkSmartPointer<vtkImageData>);
	void Set_Mask_Img(vtkSmartPointer<vtkImageData>);
	void RenderView();
	int Slice_Position;

	vtkSmartPointer<vtkRenderer> new_render;
	vtkSmartPointer<vtkRenderWindow> view_window;

	public slots:
		void on_scroll_mouse_back(vtkObject*);
		void on_scroll_mouse_forward(vtkObject*);
private:
	double view_dirX[3];
	double view_dirY[3];
	double view_dirZ[3];
	double center[3];

	double spacing[3];
	double origin[3];
	int    extent_m[6];

	vtkSmartPointer<vtkImageData> img_to_view;
	vtkSmartPointer<vtkImageData> img_to_mask;
	vtkSmartPointer<vtkImageReslice> reslice;
	vtkSmartPointer<vtkImageReslice> mask_reslice;
	vtkSmartPointer<vtkWindowLevelLookupTable> lookup_table;
	vtkSmartPointer<vtkImageMapToColors> color_map;
	vtkSmartPointer<vtkImageActor> actor;
	vtkSmartPointer<vtkImageActor> mask_actor;

	char direction;
	int  slice_n;
	int* dimensions;
	void Set_Direction(char);

	double* calculate_img_center(vtkSmartPointer<vtkImageData>);
	void Set_Window(vtkRenderWindow*);


	//qt slot connect
	vtkSmartPointer<vtkEventQtSlotConnect> m_Connections_mouse_back;
	vtkSmartPointer<vtkEventQtSlotConnect> m_Connections_mouse_forward;
};





class reslice_interactor_style : public vtkInteractorStyleImage
{
public:
	static reslice_interactor_style* New();
	vtkTypeMacro(reslice_interactor_style, vtkInteractorStyleImage);

	void OnMouseWheelBackward() {};
	void OnMouseWheelForward() {};
};



#endif









