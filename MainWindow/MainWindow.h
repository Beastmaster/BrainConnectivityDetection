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
#include <QStringList>
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

//imageviewer2base class
#include "ImageViewer2View.h"

// my class
#include "ResliceView.h"

//include my image convert class
#include "ImageConvert.h"

//include my construct
#include "MarchingCube_construct.h"


class vtkimageview2_base;

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
	void on_slider_strip_val_move(int);
	void info_Panel_Scroll();
	void mouse_Wheel_move(QWheelEvent *e);
	void init_Parameters();

private:
	//GUI
	Ui::MainWindow	*ui;

	//file names
	QString file_name;
	QStringList file_name_list;

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
	void print_Info(QString,QString,QString,QString);

	//qt vtk views
	vtkimageview2_base* view_axial;
	vtkimageview2_base* view_cornoal;
	vtkimageview2_base* view_saggital;


	//
	reslice_view_base* view_axial_reslice;
	reslice_view_base* view_coronal_reslice;
	reslice_view_base* view_saggital_reslice;

	//marching cube class base
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






#endif

