#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <stdio.h>
#include <stdlib.h>

//qt open file_dialog
#include <QFileDialog>
//messagebox
#include <QMessageBox>
//qt string class
#include <QString>

//qt + vtk include
#include "QVTKWidget.h"

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


	//display filter
	void display_in_axial(vtkSmartPointer<vtkImageData>);
	void display_in_coronal(vtkSmartPointer<vtkImageData>);
	void display_in_sagittal(vtkSmartPointer<vtkImageData>);
	double* calculate_img_center(vtkSmartPointer<vtkImageData>);
private:
	//GUI
	Ui::MainWindow	*ui;

	//file names
	QString file_name;

	vtkSmartPointer<vtkImageData> img_to_view;

	//vtk image views
	vtkSmartPointer<vtkRenderer> axial_renderer;
	vtkSmartPointer<vtkRenderer> coronal_renderer;
	vtkSmartPointer<vtkRenderer> sagittal_renderer;
};




class slice_view_base
{
public:
	slice_view_base(vtkRenderWindow*,char);
	~slice_view_base();

	void SetSlice(int x) {this->slice_n = x;};
	void Set_View_Img(vtkSmartPointer<vtkImageData>);
	void RenderView(int x);

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
	void Set_Direction(char);
	double* calculate_img_center(vtkSmartPointer<vtkImageData>);
	void Set_Window(vtkRenderWindow*);
};








#endif

