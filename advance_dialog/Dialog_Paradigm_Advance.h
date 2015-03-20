#ifndef DIALOG_PARADIGM_ADVANCE__H
#define DIALOG_PARADIGM_ADVANCE__H

#include <QDialog>
#include <vector>
#include <utility>
#include <QList>
//convert content in lineedit to qstring
#include <QString>
//qt message box
#include <QMessageBox>
#include <QListWidgetItem>

//include for painter
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QColor>
#include <QRectF>
#include <QDebug>

//signal Modelling in fmri
#include "fmri/fMRIEngine_include.h"

//use vtk data array
#include "vtkFloatArray.h"
#include "vtkIntArray.h"
#include "vtkSmartPointer.h"

//get local time for null input name
#include <time.h>
#include <stdio.h>


namespace Ui {
class Dialog_Paradigm_Advance;
}

class Dialog_Paradigm_Advance : public QDialog
{
	Q_OBJECT
public:
	explicit Dialog_Paradigm_Advance(QWidget *parent = 0);
	~Dialog_Paradigm_Advance();


	//accessible parameters
	std::vector< std::vector<int> > onset;
	std::vector< std::vector<int> > duration;
	vtkFloatArray*                  designMat;   //design matrix
	float							TR;
	vtkIntArray*					contrast_vector;
	int								number_of_time_point;
	
public slots:
	void on_click_add_task();
	void on_click_del_task();

	void on_click_add_condition();
	void on_click_del_condition();

	void on_task_focus_change(int currentRow);
	void on_condition_focus_channge(int currentRow);

	void on_click_viewdesign();
	void on_click_select_task();

	void on_valid_vector();
	void on_click_done();

private:
	Ui::Dialog_Paradigm_Advance* ui;

//parameters
	std::vector< SignalModeling* >  siglm;      //model paradigm
	std::vector< int >     contrast;   //contrast vector

	
	typedef std::vector<int> onset_type;
	typedef std::vector<int> duration_type;
	typedef struct condition_struct_type
	{
		std::string      condition_name;
		onset_type       onset;
		onset_type       duration;
	} condition_struct;
	typedef struct task_struct_type
	{
		std::string               task_name;
		int						  num_volume;
		QList< condition_struct > condition_list;
	} task_struct;

	QList< task_struct > task_list;
	QGraphicsScene* designMatrixCanvas;
	vector<QGraphicsRectItem*> rectItem_to_delete;
	void print_self();
};











#endif