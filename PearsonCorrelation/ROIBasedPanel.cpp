#include "ROIBasedPanel.h"
#include "ui_ROIBasedPanel.h"



ROIBasedPanel::ROIBasedPanel(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ROIBasedPanel)
{
	    ui->setupUi(this);
}

ROIBasedPanel::~ROIBasedPanel()
{
	    delete ui;
}


void ROIBasedPanel::Init_Para()
{
	templete_image = NULL;
	atlas_image = NULL;


}



void ROIBasedPanel::on_click_sel_volume()
{
	this->data_container = this->main_win->GetDataContainer();
}

void ROIBasedPanel::on_click_Run()
{
	if ( this->atlas_image == NULL ||
		 this->data_container.size() == 0 ||
		 this->label_value.size() == 0)
	{
		return;
	}

	//clear region time course 
	if (this->RegionTimecourse.size() != 0 )
	{
		this->RegionTimecourse.clear();
	}

	//define a handle to generate time course
	GenerateTimecourse* timecourse_generator = new GenerateTimecourse;
	//add data one by one
	for (int i = 0;i<this->data_container.size();i++)
	{
		timecourse_generator->AddInputData(this->data_container[i].second);
	}

	//iterate through label value map to generate all time course
	for (std::map< std::string , float >::iterator it 
							= this->label_value.begin();
		 it!=this->label_value.end();
		 it++)
	{
		timecourse_generator->SetLabelValue((*it).second);
		std::pair< std::string, std::vector< double > > temp;
		temp.first  = (*it).first;
		temp.second = timecourse_generator->GetTimecourse();

		this->RegionTimecourse.push_back(temp);
	}

	//delete timecourse_generator in the end
	delete timecourse_generator;

	//run pearson correlation analysis
	
}

void ROIBasedPanel::on_click_Create_Mask()
{

}

void ROIBasedPanel::on_click_load_Mask()
{

}


void ROIBasedPanel::sel_templete(int index)
{
	//get templete and atlas name
	switch (index)
	{
	case 0:
		{

			break;
		}
	case 1:
		{


			break;
		}
	case 2:
		{

			break;
		}
	default:
		{
			break;
		}
	}


	Image_Convert_Base read_templete;
	read_templete.SetFileName(templete_file_name);
	this->templete_image = read_templete.GetOutput();

	Image_Convert_Base read_atlas;
	read_atlas.SetFileName(atlas_file_name);
	this->atlas_image = read_atlas.GetOutput();

	//put label and map to label_map
	PearsomMethod(RegionTimecourse);
}

void ROIBasedPanel::sel_correlation(int)
{

}

void ROIBasedPanel::sel_method(int index)
{

}


void ROIBasedPanel::PearsomMethod(std::vector< std::pair< std::string, std::vector< double > > > timecourse)
{
	pearson_method* new_method = new pearson_method;

	typedef std::vector< std::pair< std::string, std::vector< double > > > temp_type;
	//iterate through the time course to add data
	for(temp_type::iterator it = timecourse.begin();it!=timecourse.end();it++)
	{
		new_method->Add_Row_name((*it).first);
		new_method->Add_Row((*it).second);
	}

	//calculate
	auto test = new_method->Calculate_Correlation();

	delete new_method;

	//ouput to a text file for test
	//std::ofstream test_out("correlation_matrix.txt");
	//if (test_out)
	//{
	//	if ()
	//	{
	//	}
	//}
}



