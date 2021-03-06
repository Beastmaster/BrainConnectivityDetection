
#ifndef _ROIBASEDPANEL_H_
#define _ROIBASEDPANEL_H_

#include <fstream>
#include <iostream>
#include <string.h>

#include <QWidget>
#include <QFileDialog>

#include "ImageConvert.h"
#include "MainWindow.h"
#include "GenerateTimeCourse.h"
#include "PearsonCorrelationMethod.h"
#include "Register.h"
#include "libICA.h"

namespace Ui {
	class ROIBasedPanel;
}

class ROIBasedPanel : public QWidget
{
	Q_OBJECT

public:
	explicit ROIBasedPanel(MainWindow* win,QWidget *parent = 0);
	~ROIBasedPanel();

	enum Templete
	{
		AAL_Templete = 0,
		Talarich,
		Broadman
	} templete;

	enum Correlation
	{
		Pearson_Correlation = 0
	} correlation;

	enum Method
	{
		Seed_Based = 0,
		ROI_Based
	} method;

	public slots:
		void on_click_sel_volume();
		void on_click_normalize();
		void on_click_Run();
		void on_run_ROIbased();
		void on_run_Seedbased();
		void on_click_Create_Mask();
		void on_click_load_Mask();
		void sel_templete(int);
		void sel_correlation(int);
		void sel_method(int);
		void view_correlation_matrix();
		void view_correlation_matrix2();
		void save_Matrix_Veiw();
		void fastICA_Analysis();
		void fastICA_Load();
		void fastICA_Save();

public:
	//public parameters
	//hold region timecourse: 1.region name; 2.timecourse.
	std::vector< std::pair< std::string, std::vector< double > > > 
									RegionTimecourse;
	//map that match all label name and value: 1. region label name; 2. value
	std::map< std::string , float > label_value;

private:
	Ui::ROIBasedPanel *ui;
	MainWindow* main_win;

	int templete_ID;
	int method_ID;
	int correlation_ID;

	std::string templete_file_name;
	std::string atlas_file_name;
	vtkSmartPointer<vtkImageData> templete_image;  //structure image 
	vtkSmartPointer<vtkImageData> atlas_image;     //label value map
	//data container hold all data
	std::vector< std::pair<std::string, vtkSmartPointer<vtkImageData> > >
		data_container;
	std::vector< std::pair<std::string, vtkSmartPointer<vtkImageData> > >
		component_container;
	col_type2 correlation_matrix;

	//private functions:
	void Init_Para();
	void PearsomMethod(std::vector< std::pair< std::string, std::vector< double > > > );
	
	//for display matrix
	QGraphicsScene* correlation_MatrixCanvas;
	std::vector<QObject*> rectItem_to_delete;

};








#endif

