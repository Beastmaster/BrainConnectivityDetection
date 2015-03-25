#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>

//qt open file_dialog
#include <QFileDialog>
//messagebox
#include <QMessageBox>
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
	void on_click_bold();

	//display filter
	void display_in_axial(vtkSmartPointer<vtkImageData>);
	void display_in_coronal(vtkSmartPointer<vtkImageData>);
	void display_in_sagittal(vtkSmartPointer<vtkImageData>);
	
private:
	//GUI
	Ui::MainWindow	*ui;

	//file names
	QString file_name;

	vtkSmartPointer<vtkImageData> img_to_view;

	double* calculate_img_center(vtkSmartPointer<vtkImageData>);
	//vtk image views
	vtkSmartPointer<vtkRenderer> axial_renderer;
	vtkSmartPointer<vtkRenderer> coronal_renderer;
	vtkSmartPointer<vtkRenderer> sagittal_renderer;
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
	void RenderView(int x);

	public slots:
		void on_scroll_mouse_back(vtkObject*);
		void on_scroll_mouse_forward(vtkObject*);
private:
	double* view_dirX;
	double* view_dirY;
	double* view_dirZ;

	vtkSmartPointer<vtkImageData> img_to_view;
	vtkSmartPointer<vtkImageReslice> reslice;
	vtkSmartPointer<vtkWindowLevelLookupTable> lookup_table;
	vtkSmartPointer<vtkImageMapToColors> color_map;
	vtkSmartPointer<vtkRenderer> new_render;

	vtkSmartPointer<vtkImageViewer2> img_viewer2;
	vtkSmartPointer<vtkRenderWindow> view_window;
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

