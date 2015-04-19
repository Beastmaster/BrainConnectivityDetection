#include "ROIBasedPanel.h"
#include "ui_ROIBasedPanel.h"



ROIBasedPanel::ROIBasedPanel(MainWindow* win,QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ROIBasedPanel)
{
	ui->setupUi(this);
	this->main_win = win;
	connect(this->ui->sel_volume_Btn,SIGNAL(clicked()),this,SLOT(on_click_sel_volume()));
	connect(this->ui->run_Btn,SIGNAL(clicked()),this,SLOT(on_click_Run()));
	this->Init_Para();
}

ROIBasedPanel::~ROIBasedPanel()
{
	delete ui;
}


void ROIBasedPanel::Init_Para()
{
	templete_image = NULL;
	atlas_image = NULL;

	//read default templete and atlas first
	templete_file_name = "C:\\Users\\USER\\Desktop\\Graduation Project\\Templete\\AAL\\structure_T2_smoothed.nii";
	atlas_file_name    = "C:\\Users\\USER\\Desktop\\Graduation Project\\Templete\\AAL\\waal.nii";

	Image_Convert_Base* init_reader = new Image_Convert_Base;
	init_reader->SetFileName(templete_file_name);
	templete_image = vtkSmartPointer<vtkImageData>::New();
	templete_image = init_reader->GetOutput();

	init_reader->SetFileName(atlas_file_name);
	atlas_image = vtkSmartPointer<vtkImageData>::New();
	atlas_image = init_reader->GetOutput();
	delete init_reader;

	//map label and value
	std::string map_list = "C:\\Users\\USER\\Desktop\\Graduation Project\\Templete\\AAL\\aal.nii.txt";

	std::ifstream ifs;
	ifs.open (map_list, std::ifstream::in);
	if (!ifs)
	{
		std::cout<<"file does not exist"<<std::endl;
		return;
	}
	else
	{//file exist, read line in the file
		std::string temp_line;
		while (std::getline(ifs,temp_line,'\n'))
		{
			QString q_tem_line = temp_line.data();

			QStringList line_list = q_tem_line.split(tr(" "));
			if (line_list.size() != 3) // skip line if there is no 3 components
			{
				continue;
			}

			//put value into map
			label_value[line_list[1].toStdString()] = line_list[0].toFloat();
		}
	}
	if (ifs.is_open())
	{
		ifs.close();
	}

}



void ROIBasedPanel::on_click_sel_volume()
{
	//this->data_container = this->main_win->GetDataContainer();

//	disconnect(this->ui->view_vol_Slider,SIGNAL(valueChanged(int)),this,SLOT(on_slider_volume_move(int)));
	QStringList file_name_list = 
		QFileDialog::getOpenFileNames(this,
		tr("open log file"),"./",tr("(*)"));
	if (file_name_list.isEmpty()){return;}

	this->data_container.clear();

	for (int fn = 0;fn<file_name_list.size();fn++)
	{
		std::cout<<"registering "<<fn<<"th image ..."<<std::endl;

		std::string file_name = file_name_list[fn].toStdString();

		Image_Convert_Base* reader = new Image_Convert_Base;
		reader->SetFileName(file_name);
		vtkSmartPointer<vtkImageData> img_data = reader->GetOutput();
		std::pair<std::string, vtkSmartPointer<vtkImageData> > img;
		img.first = file_name;
		img.second = img_data;
		this->data_container.push_back(img);
		delete reader;

		int dim[3];
		img_data->GetDimensions(dim);
		int dim_temp[3];
		this->atlas_image->GetDimensions(dim_temp);
	}

}



void ROIBasedPanel::on_click_normalize()
{



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
	timecourse_generator->SetLabelMap(this->atlas_image);
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
		std::cout<<"Computing "<<(*it).first<<std::endl;
		timecourse_generator->SetLabelValue((*it).second);
		std::pair< std::string, std::vector< double > > temp;
		temp.first  = (*it).first;
		temp.second = timecourse_generator->GetTimecourse();

		this->RegionTimecourse.push_back(temp);
	}

	//delete timecourse_generator in the end
	delete timecourse_generator;

	//run pearson correlation analysis
	std::string out_time_course = "out_time_course.txt";
	std::remove(out_time_course.data());
	std::ofstream out_file(out_time_course);  
	if (!out_file.is_open())
	{
		std::cout<<"opening file: "<<out_time_course<<std::endl;
		out_file.open(out_time_course,std::ofstream::out || std::ofstream::app);
	}
	for (int i=0;i<RegionTimecourse.size();i++)
	{
		out_file<<RegionTimecourse[i].first<<": ";
		for (int j = 0;j<RegionTimecourse[i].second.size();j++)
		{
			out_file<<RegionTimecourse[i].second[j]<<", ";
		}
		out_file<<"\n";
	}
	if (out_file.is_open())
	{
		out_file.close();
	}

	std::cout<<"running pearson correlation analysis"<<std::endl;
	PearsomMethod(this->RegionTimecourse);

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
	//clear label_value first
	this->label_value.clear();
	//for ()
	//{
	//}
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
		new_method->Make_Matrix();
	}

	//calculate
	auto test = new_method->Calculate_Correlation();

	delete new_method;

	std::string out_time_course = "out_correlation.txt";
	std::remove(out_time_course.data());
	std::ofstream out_file(out_time_course);  
	if (!out_file.is_open())
	{
		std::cout<<"opening file: "<<out_time_course<<std::endl;
		out_file.open(out_time_course,std::ofstream::out || std::ofstream::app);
	}
	for (int i=0;i<test.size();i++)
	{
		out_file<<test[i].first<<": ";
		for (int j = 0;j<test[i].second.size();j++)
		{
			out_file<<test[i].second[j]<<", ";
		}
		out_file<<"\n";
	}
	if (out_file.is_open())
	{
		out_file.close();
	}
}



