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

#endif

