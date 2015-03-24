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

private:
	//GUI
	Ui::MainWindow	*ui;

	//file names
	QString file_name;

	//vtk image views
	vtkSmartPointer<vtkImageViewer2> axial_vtk_viewer;
	vtkSmartPointer<vtkRenderer> axial_vtk_renderer;
	vtkSmartPointer<vtkImageData> img_to_view;
};

#endif

