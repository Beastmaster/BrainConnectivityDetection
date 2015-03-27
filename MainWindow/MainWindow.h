#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

//qt open file_dialog
#include <QFileDialog>
//messagebox
#include <QMessageBox>
#include <QWheelEvent>
//qt string class
#include <QString>
//BOLD head
#include "SubWidgetParadigmInBold.h"

//change vtk interact handle
#include "vtkInteractorStyleImage.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"

//qt + vtk include
#include "QVTKWidget.h"
//qt + vtk connect signal and slot
#include "vtkCommand.h"
#include "vtkEventQtSlotConnect.h"
//vtk include files
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkPointData.h"//cannot use SetInput in color_map if not include
#include "vtkImageProperty.h"
#include <vtkImageMapper3D.h>
#include "vtkMetaImageReader.h"
#include "vtkDICOMImageReader.h"
//for visualize
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkImageViewer2.h"
#include "vtkImageActor.h"

//vtk reslice
#include "vtkImageReslice.h"
#include "vtkWindowLevelLookupTable.h"
#include "vtkImageMapToColors.h"
//image fusion
#include "vtkImageBlend.h"

//include my image convert class
#include "ImageConvert.h"

//include my construct
#include "MarchingCube_construct.h"

class slice_view_base;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();


//define slots
private slots:
	//load images
	void on_click_load();
	void on_click_show();
	void on_click_show3d();
	void on_click_bold();
	void on_click_add_mask_file();
	void on_click_mask();
	void on_click_del_mask();
	void on_slider_volume_move(int);
	void on_slider_opicity_move(int);
	void info_Panel_Scroll();
	void mouse_Wheel_move(QWheelEvent *e);
	void init_Parameters();

private:
	//GUI
	Ui::MainWindow	*ui;

	//file names
	QString file_name;

	//first: name; second: vtkImageData
	typedef std::pair<std::string, vtkSmartPointer<vtkImageData> > img_view_base_Type;
	img_view_base_Type img_to_view;
	img_view_base_Type mask_img;
	vector<img_view_base_Type > data_container;
	//bold widget
	SubWidgetParadigmInBold* bold_win;
	//pravite methods
	double* calculate_img_center(vtkSmartPointer<vtkImageData>);
	void set_slider_volume_range(int);
	void set_data_container(vector<img_view_base_Type > );
	void refresh_view();
	void print_Info(QString,QString); 


	//qt vtk views
	slice_view_base* view_axial;
	slice_view_base* view_cornoal;
	slice_view_base* view_saggital;
	construct_base* new_3d_view;

	//vtk image views
	vtkSmartPointer<vtkRenderer> axial_renderer;
	vtkSmartPointer<vtkRenderer> coronal_renderer;
	vtkSmartPointer<vtkRenderer> sagittal_renderer;

	//actor for mask
	vtkSmartPointer<vtkImageActor> axial_mask_Actor;
	vtkSmartPointer<vtkImageActor> coronal_mask_Actor;
	vtkSmartPointer<vtkImageActor> sagittal_mask_Actor;
};


class slice_view_base : QObject
{
	Q_OBJECT
public:
	explicit slice_view_base(QWidget *parent = 0);
	slice_view_base(vtkRenderWindow*,char);
	~slice_view_base();

	void SetSlice(int x) {this->slice_n = x;};
	void Set_View_Img(vtkSmartPointer<vtkImageData>);
	int RenderView(int x);
	int Slice_Position;

	vtkSmartPointer<vtkRenderer> new_render;
	vtkSmartPointer<vtkImageViewer2> img_viewer2;
	vtkSmartPointer<vtkRenderWindow> view_window;

	public slots:
		void on_scroll_mouse_back(vtkObject*);
		void on_scroll_mouse_forward(vtkObject*);
private:
	double* view_dirX;
	double* view_dirY;
	double* view_dirZ;


	vtkSmartPointer<vtkImageData> img_to_view;
	//vtkSmartPointer<vtkImageReslice> reslice;
	//vtkSmartPointer<vtkWindowLevelLookupTable> lookup_table;
	//vtkSmartPointer<vtkImageMapToColors> color_map;

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

class new_interactor_style : public vtkInteractorStyleImage
{
public:
	static new_interactor_style* New();
	vtkTypeMacro(new_interactor_style, vtkInteractorStyleImage);

	void OnMouseWheelBackward() {};
	void OnMouseWheelForward() {};
};





#endif

