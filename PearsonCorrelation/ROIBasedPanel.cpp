#include "ROIBasedPanel.h"
#include "ui_ROIBasedPanel.h"

#include <functional>
#include <thread>

ROIBasedPanel::ROIBasedPanel(MainWindow* win,QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ROIBasedPanel)
{
	ui->setupUi(this);
	this->main_win = win;
	connect(this->ui->sel_volume_Btn,SIGNAL(clicked()),this,SLOT(on_click_sel_volume()));
	connect(this->ui->run_Btn,SIGNAL(clicked()),this,SLOT(on_click_Run()));
	connect(this->ui->ica_test_Btn,SIGNAL(clicked()),this,SLOT(fastICA_Analysis()));
	connect(this->ui->sel_base_method,SIGNAL(currentIndexChanged(int)),this,SLOT(sel_method(int)));
	connect(this->ui->sel_templete,SIGNAL(currentIndexChanged(int)),this,SLOT(sel_templete(int)));
	connect(this->ui->sel_folder_Btn,SIGNAL(clicked()),this,SLOT(fastICA_Load()));
	connect(this->ui->save_comp_Btn,SIGNAL(clicked()),this,SLOT(fastICA_Save()));
	connect(this->ui->save_matrix_Btn,SIGNAL(clicked()),this,SLOT(save_Matrix_Veiw()));
	this->Init_Para();
}

ROIBasedPanel::~ROIBasedPanel()
{
	delete ui;



	//delete text items in graph view
	for (int i=0;i<this->rectItem_to_delete.size();i++)
	{
		delete this->rectItem_to_delete[i];
	}
}


void ROIBasedPanel::Init_Para()
{

	templete_ID = 0;
	method_ID = 0;
	correlation_ID = 0;

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

	QStringList file_name_list = 
		QFileDialog::getOpenFileNames(this,
		tr("open log file"),"./",tr("(*)"));
	if (file_name_list.isEmpty()){return;}

	this->data_container.clear();

	for (int fn = 0;fn<file_name_list.size();fn++)
	{
		std::cout<<"Reading "<<fn<<"th image ..."<<std::endl;

		std::string file_name = file_name_list[fn].toStdString();

		Image_Convert_Base* reader = new Image_Convert_Base;
		reader->SetFileName(file_name);
		vtkSmartPointer<vtkImageData> img_data = reader->GetOutput();
		if (img_data == NULL)
		{
			delete reader;
			continue;
		}

		std::pair<std::string, vtkSmartPointer<vtkImageData> > img;
		img.first = file_name;
		img.second = img_data;
		this->data_container.push_back(img);
		delete reader;
	}
	std::cout<<"load image done!"<<std::endl;
}



void ROIBasedPanel::on_click_normalize()
{



}

void ROIBasedPanel::on_click_Run()
{
	int dim[3];
	int dim2[3];
	data_container[0].second->GetDimensions(dim);
	atlas_image->GetDimensions(dim2);
	for (unsigned int i=0;i<3;i++)
	{
		if (dim2[i]!=dim[i])
		{
			std::cout<<"Normalize first: dimensions do not match.."<<std::endl;
			return;
		}
	}

	switch(method_ID)
	{
	case 0:
		{
			this->on_run_ROIbased();
			break;
		}
	case 1:
		{
			this->on_run_Seedbased();
			break;
		}
	default:
		{
			this->on_run_ROIbased();
			break;
		}
	}
}

void ROIBasedPanel::on_run_Seedbased()
{
	//---------define a label to generate pearson correlation map----------//
	int i_th_label = 0;
	//---------define a label to generate pearson correlation map----------//

	if (this->data_container.size() == 0 )
	{
		return;
	}

	//------1. overlay seed area and generate timecourse------//
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
	for (std::map< std::string , float >::iterator it = this->label_value.begin();
		it!=this->label_value.end();
		it++)
	{
		std::cout<<"Masking Region: "<<(*it).first<<std::endl;
		timecourse_generator->SetLabelValue((*it).second);
		std::pair< std::string, std::vector< double > > temp;
		temp.first  = (*it).first;
		temp.second = timecourse_generator->GetTimecourse();

		this->RegionTimecourse.push_back(temp);
	}


	//------2. pearson correlation calculation-------------//
	//allocate memory for one component
	vtkSmartPointer<vtkImageData> output = vtkSmartPointer<vtkImageData>::New();
	vtkSmartPointer<vtkImageData> input = data_container[0].second;
	int dims[3] = {0};
	input->GetDimensions(dims);
#if VTK_MAJOR_VERSION <=5
	output->SetScalarType(VTK_FLOAT);
	output->SetOrigin(input->GetOrigin());
	output->SetSpacing(input->GetSpacing());
	output->SetNumberOfScalarComponents(1);
	output->SetDimensions(dims[0], dims[1], dims[2]);
	output->AllocateScalars();
#else
	output->SetOrigin(input->GetOrigin());
	output->SetSpacing(input->GetSpacing());
	output->SetDimensions(dims[0], dims[1], dims[2]);
	output->AllocateScalars(VTK_FLOAT, 1);
#endif

	vtkDataArray *scalarsOutput = output->GetPointData()->GetScalars();

	// Voxel iteration through the entire image volume
	int indx = 0;
	for (int kk = 0; kk < dims[2]; kk++)
	{
		for (int jj = 0; jj < dims[1]; jj++)
		{
			for (int ii = 0; ii < dims[0]; ii++)
			{
				auto point_timecourse = timecourse_generator->GetPointTimecourse(ii,jj,kk);
				std::vector<double> point_tim;
				for (auto it = point_timecourse.begin();it!=point_timecourse.end();it++)
				{
					point_tim.push_back((*it));
				}
				float com_value = float(pearson(point_tim,this->RegionTimecourse[i_th_label].second));
				scalarsOutput->SetComponent(indx, 0, com_value);
				indx++;
			}
		} 
	}
	//delete timecourse_generator in the end
	delete timecourse_generator;

	//------3. write to image--------------//
	Image_Convert_Base::WriteTonii(output,this->RegionTimecourse[i_th_label].first.append(".nii"));
	std::cout<<"seed based method done"<<std::endl;
}

void ROIBasedPanel::on_run_ROIbased()
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
		std::cout<<"Masking Region: "<<(*it).first<<std::endl;
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

	std::cout<<"Process done"<<std::endl;

	view_correlation_matrix();
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

void ROIBasedPanel::sel_correlation(int index)
{
	correlation_ID = index;
	switch(method_ID)
	{
	case 0:
		{
			std::cout<<"select AAL templete"<<std::endl;
			break;
		}
	case 1:
		{
			std::cout<<"select Talarich templete"<<std::endl;
			break;
		}
	case 2:
		{
			std::cout<<"select Broadman templete"<<std::endl;
			break;
		}
	default:
		{
			std::cout<<"select un-resonable templete"<<std::endl;
			break;
		}
	}
}

void ROIBasedPanel::sel_method(int index)
{
	method_ID = index;
	switch(method_ID)
	{
	case 0:
		{
			std::cout<<"select ROI based method"<<std::endl;
			break;
		}
	case 1:
		{
			std::cout<<"select Seed based method"<<std::endl;
			break;
		}
	default:
		{
			std::cout<<"select un-resonable method"<<std::endl;
			break;
		}
	}
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
	this->correlation_matrix = test;

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
void ROIBasedPanel::view_correlation_matrix2()
{

}
void ROIBasedPanel::view_correlation_matrix()
{

	this->correlation_MatrixCanvas = new QGraphicsScene(this->ui->graphicsView);
	this->ui->graphicsView->setScene(this->correlation_MatrixCanvas);

	//get data range and cols/rows of designmatrix
	double cor_MatRange[2] = {-1,1};
	double range_temp = cor_MatRange[1]-cor_MatRange[0];
	//this->designMat->GetRange(designMatRange,-1);
	int draw_col = this->correlation_matrix.size();
	int draw_row = this->correlation_matrix.size();

	//map number to 0-255
	auto colo_map_func = [] (float num) 
	{
		if (num<0){return (-num)*255;}
		else      {return num*255;}
	};

	//map position
	float origin_x = 0;
	float origin_y = 0;
	float x_width = 600;
	float y_height = 600;

	//width and height of per area
	float x_width_per = x_width/draw_row; 
	float y_height_per = y_height/draw_col;

	float x_pos = 0;
	float y_pos = 0;
	//map color
	double color_step = 512/range_temp;

	//add a color look_up_table to view
	for (int i=0;i<0;i++)
	{
		if (i<256)
		{
			QColor disp_color = QColor(i,255,255);
			QGraphicsRectItem* new_rect_item = 
				new QGraphicsRectItem(x_pos-150,i,x_width_per/2,1);
			new_rect_item->setPen(QPen(Qt::NoPen));
			new_rect_item->setBrush(disp_color);
			correlation_MatrixCanvas->addItem(new_rect_item);
			this->rectItem_to_delete.push_back((QObject*)new_rect_item);
		}
		else
		{
			QColor disp_color = QColor(255,255,i-255);
			QGraphicsRectItem* new_rect_item = 
				new QGraphicsRectItem(x_pos-150,i,x_width_per/2,1);
			new_rect_item->setPen(QPen(Qt::NoPen));
			new_rect_item->setBrush(disp_color);
			correlation_MatrixCanvas->addItem(new_rect_item);
			this->rectItem_to_delete.push_back((QObject*)new_rect_item);
		}
	}
	

	//col name items
	float name_height = 0;
	for (int i=0;i<draw_row;i++)
	{
		//add name
		QGraphicsTextItem* name_text_item = new QGraphicsTextItem;
		name_text_item->setPos(-100,name_height);
		name_text_item->setPlainText(this->correlation_matrix[i].first.data());
		correlation_MatrixCanvas->addItem(name_text_item);
		name_height += y_height_per;
		this->rectItem_to_delete.push_back((QObject*)name_text_item);
	}


	//add cor-value color-map items
	for (int i = 0;i<draw_row;i++)
	{
		//create items and add
		float y_pos = 0;
		for(int j=0;j<draw_col;j++)
		{
			//get value of component
			double tem_component_value = this->correlation_matrix[i].second[j];//this->designMat->GetComponent(j,i);
			QColor disp_color;// = QColor(int(red_color_temp),0,0);//,int(red_color_temp),int(red_color_temp));
			if (tem_component_value<0)
			{
				double red_color_temp = colo_map_func(tem_component_value);
				disp_color = QColor(255,255,int(red_color_temp));
			}
			else
			{
				double red_color_temp = colo_map_func(tem_component_value);
				disp_color = QColor(int(red_color_temp),255,255);
			}
			QGraphicsRectItem* new_rect_item = new QGraphicsRectItem(x_pos,y_pos,x_width_per,y_height_per);
			new_rect_item->setPen(QPen(Qt::NoPen));
			new_rect_item->setBrush(disp_color);
			correlation_MatrixCanvas->addItem(new_rect_item);
			this->rectItem_to_delete.push_back((QObject*)new_rect_item);
			y_pos += y_height_per;
		}
		x_pos += x_width_per;
	}

	//add text to view
	QGraphicsTextItem* new_text_item = new QGraphicsTextItem;
	new_text_item->setPos(0,-20);
	new_text_item->setPlainText("Coorelation Matrix");
	correlation_MatrixCanvas->addItem(new_text_item);
	this->rectItem_to_delete.push_back((QObject*)new_text_item);

}

void ROIBasedPanel::fastICA_Analysis()
{
	std::cout<<"begin ICA analysis"<<std::endl;
	this->main_win->print_Info("Start ICA analysis..");
	//delete component container
	this->component_container.clear();

	//get image size
	if (data_container.size()<=30)
	{
		std::cout<<"Less than 30 volume.. exiting"<<std::endl;
		this->main_win->print_Info("Less than 30 volume.. Exiting....");
		return;
	}

	//number of component
	int num_compc = this->ui->in_num_Compc->text().toInt();
	if ((num_compc<2)||(num_compc>data_container.size()))
	{
		num_compc = 5;
	}
	//get number of components and allocate memory
	char component_name[20];
	for (unsigned int i=0;i<num_compc;i++)
	{
		vtkSmartPointer<vtkImageData> output = vtkSmartPointer<vtkImageData>::New();
		sprintf(component_name,"%dth_component",i);
		std::pair< std::string,vtkSmartPointer<vtkImageData> > 
			cop_data_temp = std::make_pair(component_name,output);
		component_container.push_back(cop_data_temp);
	}

	int dims[3] = {0,0,0};
	data_container[0].second->GetDimensions(dims);
	int time_length = data_container.size();
	//create new matrix;
	int ica_data_cols = time_length;
	int ica_data_rows = dims[0]*dims[1];//*dims[2];
	double** src_mat = mat_create(ica_data_rows,ica_data_cols);

	double  **K, **W, **A;//**S;
	W = mat_create(num_compc, num_compc);//de-mix matrix
	A = mat_create(num_compc, num_compc);//mix matrix
	K = mat_create(ica_data_cols, num_compc);
	std::vector<double**> S_vect(dims[2]);
	std::vector<double**> S_vect_Z(dims[2]);

	for (auto it=S_vect.begin();it!=S_vect.end();it++)
		(*it) = mat_create(ica_data_rows, num_compc);	//source
	for (auto it=S_vect_Z.begin();it!=S_vect_Z.end();it++)
		(*it) = mat_create(ica_data_rows, num_compc);	//source

	//put data to matrix
	for (int k=0; k<dims[2];k++)
	{
		int tim_cnt=0;
		for (auto it=data_container.begin();it!=data_container.end();it++)
		{
			//1. put a a slice data to mat
			int vox_cnt=0;
			for (int j=0; j<dims[1];j++)
			{
				for (int i=0; i<dims[0];i++)
				{
					src_mat[vox_cnt++/*((i+1)*(j+1)*(k+1)-1)*/][tim_cnt] =*(float*)(*it).second->GetScalarPointer(i,j,k);
				}
			}
			tim_cnt++;
		}
		/*****  2. process data  *****/
		// ICA computation
		fastICA(src_mat, ica_data_rows, ica_data_cols, num_compc, K, W, A, S_vect[k]);

		//z score components (ica_data_rows)
		for (int i=0;i<num_compc;i++)
		{
			compute_Z_score(S_vect[k],ica_data_rows,num_compc,S_vect_Z[k]);
		}

		std::cout<<"processing "<<k+1<<" th slice"<<std::endl;
	}


	/*****3. release memory******/
	mat_delete(src_mat,ica_data_rows,ica_data_cols);
	mat_delete(W,num_compc, num_compc);
	mat_delete(A,num_compc, num_compc);
	mat_delete(K,ica_data_cols, num_compc);
	for (auto it=S_vect.begin();it!=S_vect.end();it++)
		mat_delete(*it,ica_data_rows, num_compc);	

	//write to output image
	//1. clean components image container
	if (component_container.size()!=0)
	{
		component_container.clear();
	}
	//2. write voxel value
	for (int vol_cnt=0;vol_cnt<num_compc;vol_cnt++)//for each component
	{
		std::stringstream ss;
		ss<<vol_cnt;
		std::string name;
		ss>>name;
		name.append("th_comp");

		//allocate memory for one component
		vtkSmartPointer<vtkImageData> output = vtkSmartPointer<vtkImageData>::New();
		vtkSmartPointer<vtkImageData> input = data_container[0].second;
		output->SetOrigin(input->GetOrigin());
		output->SetSpacing(input->GetSpacing());
		output->SetDimensions(dims[0], dims[1], dims[2]);
#if VTK_MAJOR_VERSION <= 5
		output->SetScalarType(VTK_FLOAT);
		output->SetNumberOfScalarComponents(1);
		output->AllocateScalars();
#else
		output->AllocateScalars(VTK_FLOAT,1);
#endif
		vtkDataArray *scalarsOutput = output->GetPointData()->GetScalars();
		//vtkDataArray *scalarsInput = input->GetPointData()->GetScalars();

		// Voxel iteration through the entire image volume
		int indx = 0;
		for (int kk = 0; kk < dims[2]; kk++)
		{
			int index_vox=0;
			for (int jj = 0; jj < dims[1]; jj++)
			{
				for (int ii = 0; ii < dims[0]; ii++)
				{
					float com_value = float(S_vect_Z[kk][index_vox][vol_cnt]);
					scalarsOutput->SetComponent(indx, 0, com_value);
					index_vox++;
					indx++;
				}
			} 
		}

		//add to component container
		this->component_container.push_back(std::make_pair(name,output));

		//write to nii for test
		this->main_win->add_image(name,output);
	}


	//delete cache
	for (auto it=S_vect_Z.begin();it!=S_vect_Z.end();it++)
		mat_delete(*it,ica_data_rows, num_compc);	


	std::cout<<"ICA process done!"<<std::endl;
	this->main_win->print_Info("ICA process done!");
}



void ROIBasedPanel::fastICA_Load()
{
	this->on_click_sel_volume();
}

void ROIBasedPanel::fastICA_Save()
{
	//iterate component container to save
	for(auto it = this->component_container.begin();it!=component_container.end();it++)
	{
		Image_Convert_Base::WriteTonii((*it).second,(*it).first);
	}
}

void ROIBasedPanel::save_Matrix_Veiw()
{
	QPixmap screen;
	QString format = "PNG";
	QString filename_screen = "cor_matrix";//QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	filename_screen.append(".png");
	screen = QPixmap::grabWidget(this->ui->graphicsView);
	screen.save(filename_screen,"PNG");
}

