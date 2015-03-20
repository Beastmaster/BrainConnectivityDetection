#include "Dialog_Paradigm_Advance.h"
#include "ui_Dialog_Paradigm_Advance.h"

Dialog_Paradigm_Advance::Dialog_Paradigm_Advance(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Dialog_Paradigm_Advance)
{
	this->TR = 0;
	this->number_of_time_point = 0;
	ui->setupUi(this);
	this->designMatrixCanvas = new QGraphicsScene(this->ui->designMatrix_Graph);
	this->ui->designMatrix_Graph->setScene(this->designMatrixCanvas);

	connect(this->ui->add_Task_Btn,SIGNAL(clicked()),this,SLOT(on_click_add_task()));
	connect(this->ui->del_Task_Btn,SIGNAL(clicked()),this,SLOT(on_click_del_task()));
	//focus on task item change 
	connect(this->ui->task_List_Widget,SIGNAL(currentRowChanged(int )),
		this,SLOT(on_task_focus_change(int)));
	//focus on condition_item change
	connect(this->ui->condition_list_Widget,SIGNAL(currentRowChanged(int )),
		this,SLOT(on_condition_focus_channge(int)));
	//connect(this->ui->task_List_Widget,SIGNAL(currentRowChanged(int)),this->ui->condition_list_Widget,SLOT(clear()));
	connect(this->ui->add_Condition_Btn,SIGNAL(clicked()),this,SLOT(on_click_add_condition()));
	connect(this->ui->del_Condition_Btn,SIGNAL(clicked()),this,SLOT(on_click_del_condition()));
	connect(this->ui->done_advance_Btn,SIGNAL(clicked()),this,SLOT(on_click_done()));
	connect(this->ui->select_Task_Btn,SIGNAL(clicked()),this,SLOT(on_click_select_task()));
	connect(this->ui->valid_vector_Btn,SIGNAL(clicked()),this,SLOT(on_valid_vector()));
	connect(this->ui->view_Design_Btn,SIGNAL(clicked()),this,SLOT(on_click_viewdesign()));
}

Dialog_Paradigm_Advance::~Dialog_Paradigm_Advance()
{
	for (std::vector<QGraphicsRectItem*>::iterator it=rectItem_to_delete.begin();
		it!=rectItem_to_delete.end();it++)
	{
		delete *it;
	}
	delete designMatrixCanvas;
	delete ui;
}

void Dialog_Paradigm_Advance::on_click_add_task()
{
	task_struct new_task_temp ;
	
	//get task name, find empty first
	if (this->ui->in_TaskName->text().isEmpty())
	{
		QMessageBox::warning(this,tr("Paradigm Error"),tr("Input a Task name"),QMessageBox::Yes);
		return;
	}
	else
	{
		new_task_temp.task_name = this->ui->in_TaskName->text().toStdString();
	}
	//get number of volume
	if (this->ui->in_NumVolume->text().isEmpty())
	{
		QMessageBox::warning(this,tr("Paradigm Error"),tr("Input Number of Volume"),QMessageBox::Yes);
		return;
	}
	else
	{
		new_task_temp.num_volume = this->ui->in_NumVolume->text().toInt();
	}
	//create task items
	QListWidgetItem* task_item_temp = new QListWidgetItem;
	task_item_temp->setText(this->ui->in_TaskName->text());

	//put task item into view
	this->task_list.push_back(new_task_temp);
	this->ui->task_List_Widget->addItem(task_item_temp);

	//clear input
	this->ui->in_TaskName->clear();
}
void Dialog_Paradigm_Advance::on_click_del_task()
{
	//disconnect signal and slot first
	disconnect(this->ui->task_List_Widget,SIGNAL(currentRowChanged(int)),this,SLOT(on_task_focus_change(int)));
	
	int del_task_index = this->ui->task_List_Widget->currentRow();
	if (del_task_index<0)	{return;}

	//delete items by index
	//1. find current item
	QListWidgetItem* del_task_item_temp = 
		this->ui->task_List_Widget->takeItem(del_task_index);
	//2. delete current task item
	delete del_task_item_temp;

	//3. delete current condition items views in this task
	this->ui->condition_list_Widget->clear();

	//remove from task_list data
	task_list.removeAt(del_task_index);
	
	//re-connect signal and slot of item change
	connect(this->ui->task_List_Widget,SIGNAL(currentRowChanged(int )),this,SLOT(on_task_focus_change(int)));
}

void Dialog_Paradigm_Advance::on_click_add_condition()
{
	//get current selected task index
	int cur_task_index = this->ui->task_List_Widget->currentRow();
	if (cur_task_index<0)
	{
		QMessageBox::warning(this,tr("Paradigm Error"),tr("select a task"),QMessageBox::Yes);
		return;
	}

	//get TR
	if (this->ui->in_TR->text().isEmpty())
	{
		QMessageBox::warning(this,tr("Paradigm Error"),tr("please input TR"),QMessageBox::Yes);
		return;
	}
	else
	{
		bool ok___;
		this->TR = this->ui->in_TR->text().toFloat(&ok___);
		if (ok___ = 0.0)//validate TR (float type)
		{
			QMessageBox::warning(this,tr("Paradigm Error"),tr("please input valid TR"),QMessageBox::Yes);
			return;
		}
	}

	std::string condition_name_temp = this->ui->in_ConditionName->text().toStdString();
	QStringList onset_list_temp     = this->ui->in_Onset->text().split(",",QString::SkipEmptyParts);
	QStringList duration_list_temp  = this->ui->in_duration->text().split(",",QString::SkipEmptyParts);

	//creat new condition buff
	condition_struct new_condition_to_add;
	//add condition name
	new_condition_to_add.condition_name = condition_name_temp;

	//add condition item to view
	//create task items
	QListWidgetItem* condition_item_temp = new QListWidgetItem;
	condition_item_temp->setText(this->ui->in_ConditionName->text());

	//iterate through list, put number to vector
	for (QStringList::const_iterator constIterator = onset_list_temp.constBegin();
		 constIterator!=onset_list_temp.constEnd();++constIterator)
	{
		int tttt_m = (*constIterator).toInt();
		std::cout<<tttt_m<<std::endl;
		new_condition_to_add.onset.push_back(tttt_m);
	}
	//the iterator form above work well, while below collapse...
	//so I use a different form to traversal the list
	for (int i= 0;i< duration_list_temp.size();i++)
	{
		new_condition_to_add.duration.push_back(duration_list_temp[i].toInt());
	}

	//put para into condition list
	this->task_list[cur_task_index].condition_list.push_back(new_condition_to_add);

	//put task item into view
	this->ui->condition_list_Widget->addItem(condition_item_temp);

	//clear input
	this->ui->in_ConditionName->clear();
	this->ui->in_duration->clear();
	this->ui->in_Onset->clear();
}

void Dialog_Paradigm_Advance::on_click_del_condition()
{
	//disconnect connection first
	disconnect(this->ui->condition_list_Widget,SIGNAL(currentRowChanged(int)),this,SLOT(on_condition_focus_channge(int)));
	//get selected condition item index
	int del_condition_index;
	del_condition_index = this->ui->condition_list_Widget->currentRow();
	if (del_condition_index<0)
	{
		return;
	}
	//delete items by index
	//1. find current item
	QListWidgetItem* del_condition_item_temp 
		= this->ui->condition_list_Widget->takeItem(del_condition_index);
	//2. delete current task item from view
	delete del_condition_item_temp;

	//remove condition parameters last
	int del_task_index;
	del_task_index = this->ui->task_List_Widget->currentRow();
	task_list[del_task_index].condition_list.removeAt(del_condition_index);

	//clear parameters input
	this->ui->in_ConditionName->clear();
	this->ui->in_duration->clear();
	this->ui->in_Onset->clear();

	//connect signal and slots
	connect(this->ui->condition_list_Widget,SIGNAL(currentRowChanged(int)),this,SLOT(on_condition_focus_channge(int)));
}

//Description:
//when move mouse to a different task_item, then display its condition list
void Dialog_Paradigm_Advance::on_task_focus_change(int currentRow)
{
	//clear condition view first
	this->ui->condition_list_Widget->clear();

	//create new condition list items
	for (int i=0;i<this->task_list[currentRow].condition_list.size();i++)
	{
		QListWidgetItem* condition_item_to_add = new QListWidgetItem;
		condition_item_to_add->setText(this->task_list[currentRow].condition_list[i].condition_name.data());
		//add to view
		this->ui->condition_list_Widget->addItem(condition_item_to_add);
	}
}
void Dialog_Paradigm_Advance::on_condition_focus_channge(int currentRow)
{
	int sel_item_index = this->ui->task_List_Widget->currentRow();
	int sel_condition_index = currentRow;
}

void Dialog_Paradigm_Advance::on_click_viewdesign()
{
	if (this->onset.empty())
	{
		QMessageBox::warning(this,tr("Paradigm Error"),tr("select a task first"),QMessageBox::Yes);
		return;
	}

	//get data range and cols/rows of designmatrix
	double designMatRange[2] = {-1,2};
	double range_temp = designMatRange[1]-designMatRange[0];
	//this->designMat->GetRange(designMatRange,-1);
	int draw_col = number_of_time_point;
	int draw_row = this->designMat->GetNumberOfComponents();

	//map position
	float origin_x = 0;
	float origin_y = 0;
	float x_width = 300;
	float y_height = 300;

	//width and height of per area
	float x_width_per = x_width/draw_row; 
	float y_height_per = y_height/draw_col;

	float x_pos = 0;
	float y_pos = 0;
	//map color
	double color_step = 255/range_temp;

	//add a color look_up_table to view
	for (int i=0;i<255;i++)
	{
		QColor disp_color = QColor(i,i,i);
		QGraphicsRectItem* new_rect_item = 
			new QGraphicsRectItem(x_pos-100,i,x_width_per,1);
		new_rect_item->setPen(QPen(Qt::NoPen));
		new_rect_item->setBrush(disp_color);
		designMatrixCanvas->addItem(new_rect_item);
		this->rectItem_to_delete.push_back(new_rect_item);
	}

	//add items
	for (int i = 0;i<draw_row;i++)
	{
		//create items and add
		float y_pos = 0;
		for(int j=0;j<draw_col;j++)
		{
			//get value of component
			float tem_component_value = this->designMat->GetComponent(j,i);
			double red_color_temp = (tem_component_value-designMatRange[0])*color_step;
			QColor disp_color = QColor(int(red_color_temp),int(red_color_temp),int(red_color_temp));
			QGraphicsRectItem* new_rect_item = new QGraphicsRectItem(x_pos,y_pos,x_width_per,y_height_per);
			new_rect_item->setPen(QPen(Qt::NoPen));
			new_rect_item->setBrush(disp_color);
			designMatrixCanvas->addItem(new_rect_item);
			this->rectItem_to_delete.push_back(new_rect_item);
			y_pos += y_height_per;
		}
		x_pos += x_width_per;
	}

	//add text to view
	QGraphicsTextItem* new_text_item = new QGraphicsTextItem;
	new_text_item->setPos(0,-20);
	new_text_item->setPlainText("Design Matrix");
	designMatrixCanvas->addItem(new_text_item);
}

void Dialog_Paradigm_Advance::on_click_select_task()
{
	//get temp selected item index
	int sel_item_index = this->ui->task_List_Widget->currentRow();
	if (sel_item_index<0)
	{
		QMessageBox::warning(this,tr("Paradigm Error"),tr("select a task"),QMessageBox::Yes);
		return;
	}
	//get empty of condition input
	if (task_list[sel_item_index].condition_list.empty())
	{
		QMessageBox::warning(this,tr("Paradigm Error"),tr("conditions empty!"),QMessageBox::Yes);
		return;
	}

	//clear all containers first; then add date from the end 
	this->onset.clear();
	for (int i=0;i<this->task_list[sel_item_index].condition_list.size();i++)
	{
		this->onset.push_back(task_list[sel_item_index].condition_list[i].onset);
	}
	this->duration.clear();
	for (int i=0;i<this->task_list[sel_item_index].condition_list.size();i++)
	{
		this->duration.push_back(task_list[sel_item_index].condition_list[i].duration);
	}
	
	//new a handle, remember to release
	SignalModeling* signal_model_hd = new SignalModeling;
	this->number_of_time_point = this->task_list[sel_item_index].num_volume;
	signal_model_hd->SetLen(this->number_of_time_point);
	signal_model_hd->SetTR(this->TR);
	signal_model_hd->SetnumComponent(this->task_list[sel_item_index].condition_list.size());
	signal_model_hd->SetOnset(this->onset);
	signal_model_hd->SetDuration(this->duration);
	
	//new a design matrix
	this->designMat = vtkFloatArray::New();
	this->designMat = GenerateDesignMatrix(signal_model_hd);

	//delete signal model handle, release
	delete signal_model_hd;
}

void Dialog_Paradigm_Advance::on_valid_vector()
{
	//clear contrast
	if (this->ui->in_ContrastVector->text().isEmpty())
	{
		QMessageBox::warning(this,tr("Paradigm Error"),tr("Please input ContrastVector!"),QMessageBox::Yes);
		return;
	}
	else
	{
		this->contrast.clear();
	}

	QStringList contrast_vector_temp = this->ui->in_ContrastVector->text().split(",",QString::SkipEmptyParts);
	for (QStringList::const_iterator constIterator = contrast_vector_temp.constBegin();
		constIterator!=contrast_vector_temp.constEnd();++constIterator)
	{
		this->contrast.push_back((*constIterator).toInt());
	}

	this->contrast_vector = vtkIntArray::New();	
	this->contrast_vector->SetNumberOfComponents(1);
	for(int i=0;i<this->designMat->GetNumberOfComponents();i++)
	{
		if (i<this->contrast.size())
		{
			this->contrast_vector->InsertNextValue(this->contrast[i]);
		}
		else
		{
			this->contrast_vector->InsertNextValue(0);
		}
	}

	std::cout<<"number of components: "<<contrast_vector->GetNumberOfComponents()<<std::endl;
	std::cout<<"number of tuples:" <<contrast_vector->GetNumberOfTuples()<<std::endl;

	for (int i = 0;i<this->contrast_vector->GetNumberOfTuples();i++) 
	{
		for (int j = 0;j<this->contrast_vector->GetNumberOfComponents();j++) 
		{
			std::cout<<"tutu   "<<contrast_vector->GetComponent(i,j);
		}
		std::cout<<std::endl;
	}
}

void Dialog_Paradigm_Advance::on_click_done()
{
	this->done(100);
	this->print_self();
	this->hide();
}


void Dialog_Paradigm_Advance::print_self()
{
	std::cout<<"||-------------begin print self ----------------||"<<std::endl;
	for (int i = 0;i<this->task_list.size();i++)
	{
		std::cout<<"task name:  ";
		std::cout<<this->task_list[i].task_name.data()<<std::endl;
		for (int j = 0;j<this->task_list[i].condition_list.size();j++)
		{
			std::cout<<"condition name:  "<<std::endl;
			std::cout<<this->task_list[i].condition_list[j].condition_name.data()<<std::endl;
			std::cout<<" onset vector:"<<std::endl;
			for (int jj=0;jj<this->task_list[i].condition_list[j].onset.size();jj++)
			{
				std::cout<<this->task_list[i].condition_list[j].onset[jj]<<"  , ";
			}
			std::cout<<"\n duration vector"<<std::endl;
			for (int jj=0;jj<this->task_list[i].condition_list[j].duration.size();jj++)
			{
				std::cout<<this->task_list[i].condition_list[j].duration[jj]<<"  , ";
			}
		}
		std::cout<<" "<<std::endl;
	}
}











