#include "SubWidgetParadigmInBold.h"
#include "ui_SubWidgetParadigmInBold.h"

#ifndef QS_DEBUG_MODE
#include "MainWindow.h"
#include "Volume.h"
#include "LayerCollection.h"
#include "LayerBold.h"
#include "LayerVolume.h"
#include "RenderView.h"
#include "RenderView3D.h"
#include "RenderView2D.h"
#endif

SubWidgetParadigmInBold::SubWidgetParadigmInBold(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubWidgetParadigmInBold)
{
    ui->setupUi(this);
	this->init_para();
	connect(ui->conditionNameBox,SIGNAL(activated(const QString &)),
		this,SLOT( on_select_condition()),Qt::UniqueConnection);
	connect(ui->file_open_btn,SIGNAL(clicked()),this,SLOT(on_click_open()));
	connect(ui->bold_load_btn,SIGNAL(clicked()),this,SLOT(on_click_loadimage()));
	connect(ui->contrastNameBox,SIGNAL(activated(const QString &)),
		this,SLOT( on_select_contrast()),Qt::UniqueConnection);

	connect(ui->user_input_Btn,SIGNAL(clicked()),this,SLOT(on_click_user_input()));

	connect(ui->pValue_in,SIGNAL(editingFinished()),this,SLOT(on_threshold_changed()));

	//connect test button
	connect(ui->one_click_test_Btn,SIGNAL(clicked()),this,SLOT(on_click_test()));
	connect(ui->advance_Btn,SIGNAL(clicked()),this,SLOT(on_click_advance()));
	connect(ui->use_default_Btn,SIGNAL(clicked()),this,SLOT(on_click_use_default())); 
	connect(ui->threshold_Btn,SIGNAL(clicked()),this,SLOT(on_click_threshold()));  
}

SubWidgetParadigmInBold::~SubWidgetParadigmInBold()
{
    delete ui;
}


//read input numbers and generate design matrix
void SubWidgetParadigmInBold::on_click_addcondition()
{
	if (ui->onsetLineEdit->text().isEmpty()||ui->durationLineEdit->text().isEmpty())
	{
		QMessageBox::about(NULL, "ERROR", "Input Empty");
	}

	QStringList onset_list = ui->onsetLineEdit->text().split(",");
	QStringList duration_list = ui->durationLineEdit->text().split(",");

	if((onset_list.count() != duration_list.count())||onset_list.empty())
	{
		QMessageBox::about(NULL, "ERROR", "length error: diff");
	}

	vector<int> onset_tem;
	vector<int> duration_tem;
	for(int i = 0;i<onset_list.size();i++)
	{
		onset_tem.push_back(onset_list.at(i).toInt());
		duration_tem.push_back(duration_list.at(i).toInt());
	}

	this->onset.push_back(onset_tem);
	this->duration.push_back(duration_tem);

	qDebug()<<"number of tuples"<<this->onset.size();
	
	//for each time "add" clicked the num_condition increase 1
	this->num_Condition = 1;

	//clean input box
	//ui->conditionNameLineEdit->clear();
	ui->onsetLineEdit->clear();
	ui->durationLineEdit->clear();
}

void SubWidgetParadigmInBold::on_select_condition()
{
	QString str;
	str = this->ui->conditionNameBox->currentText();
	
	if (str == "default")
	{
		this->ui->onsetLineEdit->setText(tr("10,30,50"));
		this->ui->durationLineEdit->setText(tr("10,10,10"));
	}

	this->on_click_addcondition();
}

void SubWidgetParadigmInBold::on_click_use_default()
{
	//use default condition
	this->ui->conditionNameBox->setCurrentIndex(0);
	//trigger
	this->on_select_condition();

	this->on_click_viewdesign();

	this->ui->contrastNameBox->setCurrentIndex(0);
	this->on_select_contrast();

}

void SubWidgetParadigmInBold::on_click_viewdesign()
{

	if(this->num_Condition == 0)
	{
		QMessageBox::information(NULL,"design error","no volume select",
			QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
		return;
	}

	//number of volume
	if(this->numVolume == 0)
	{
		this->numVolume = this->onset.back().back()+this->duration.back().back();
	}


	//this->numVolume = 60;
	this->siglm->SetLen(this->numVolume);
	this->siglm->SetTR(this->TR);

	//put data in vector into vtkfloat array
	vtkFloatArray* tem_onset    = vtkFloatArray::New();
	vtkFloatArray* tem_duration = vtkFloatArray::New();

	tem_onset->SetNumberOfComponents(this->onset.size());
	//tem_onset->SetNumberOfTuples(this->onset.front().size());
	tem_duration->SetNumberOfComponents(this->duration.size());
	//tem_duration->SetNumberOfTuples(this->duration.front().size());
	int j_onset = 0;//component
	for (vector< vector<int> >::iterator it = this->onset.begin();it!=this->onset.end();++it)
	{
		int i=0;//tuple
		for(vector<int>::iterator ti = it->begin();ti!=it->end();++ti)
		{
			tem_onset->InsertComponent(i++,j_onset,*ti);
		}
		j_onset++;
	}
	int j_dur = 0;//component
	for (vector< vector<int> >::iterator it = this->duration.begin();it!=this->duration.end();++it)
	{
		int i=0;//tuple
		for(vector < int >::iterator ti = it->begin();ti!=it->end();++ti)
		{
			tem_duration->InsertComponent(i++,j_dur,*ti);
		}
		j_dur++;
	}

	this->siglm->SetnumComponent(this->num_Condition);
	this->siglm->SetOnset(tem_onset);
	this->siglm->SetDuration(tem_duration);

	this->designMat = GenerateDesignMatrix(siglm);

	std::ofstream file ("designmatrix.txt");
	int tuples=this->designMat->GetNumberOfTuples();
	int comp  =this->designMat->GetNumberOfComponents();
	if(file.is_open())
	{
		file<<"point data"<<std::endl;
		file<<"number of tuples:"<<tuples<<endl;
		file<<"number of component:"<<comp<<endl;

		for(int j=0;j<tuples;j++)
		{
			//file<<"\ncomponent:"<<j<<"\n";
			for(int i=0;i<comp;i++)
		   {
			 file<<designMat->GetComponent(j,i)<<" ";
			}
			file<<";";
		}
	}
	file.close();

	//plot design matrix
	//this->Design_plot();

	//clear conditions
	this->num_Condition = 0;
	
	this->onset.clear();
	this->duration.clear();
}

void SubWidgetParadigmInBold::on_click_fitmode()
{
	////generate designmatrix
	//this->on_click_viewdesign();

	//find null estimator
	this->GLMEstimator->RemoveAllInputs();

	//add detector first to estimator
	this->GLMEstimator->SetDetector(this->GLMDetector);

	//add data to GLMEstimator
	//for(int i = 0;i<this->data_container.size();i++)
	this->GLMEstimator->RemoveAllInputs();
	for(int i = 0;i<this->data_container.size();i++)
		this->GLMEstimator->AddInput(this->data_container[i]);

	// pass design matrix to GLMDetector
	this->GLMDetector->SetDesignMatrix(this->designMat);
	
	this->GLMEstimator->Update();
	this->BetaMap = this->GLMEstimator->GetOutput();

	//change workflow flag
	this->pip_flag = 100;
	qDebug()<<"fit model done !";
}
//beta is save as .vti file
void SubWidgetParadigmInBold::on_click_savebeta()
{
	vtkXMLImageDataWriter *writer2 =vtkXMLImageDataWriter::New();
	  writer2->SetFileName("betamap.vti");
	 writer2->SetInput(this->BetaMap);
	  writer2->Write();
}
void SubWidgetParadigmInBold::on_click_open()
{
	//to see if filenames_list Null or not
	if(!this->filenames_list.isEmpty())
	{
		this->filenames_list.clear();
	}
	
	this->filenames_list = QFileDialog::getOpenFileNames( this, QString(tr("Open The File")),"D:");
	//decide number of volumes
	this->numVolume = this->filenames_list.size();
}
void SubWidgetParadigmInBold::on_click_loadimage()
{

	//progress bar
	QProgressDialog* dialogLoading_load_file = new QProgressDialog("Load Files", "", 0, 3 , this);
	dialogLoading_load_file->setWindowModality(Qt::WindowModal);
	dialogLoading_load_file->setCancelButton(0);
	QLabel* label_temp_lf = new QLabel (dialogLoading_load_file);
	label_temp_lf->setStyleSheet("color: rgb(255, 255, 255);");
	dialogLoading_load_file->setLabel(label_temp_lf);
	dialogLoading_load_file->setMinimumDuration(0);
	dialogLoading_load_file->setLabelText("Loading files..");
	dialogLoading_load_file->setValue(1);



	//clear data_container
	if(!this->data_container.empty())
		this->data_container.clear();

	//select read mod, judging by filename_list null or not
	if(this->filenames_list.isEmpty())
	{
		//-----single .dcm files load from log file------//
		qDebug()<<"begin parse log file !";
		//open log file
#ifdef QS_DEBUG_MODE
		QString log = 
			QFileDialog::getOpenFileName(this,
			tr("open log file"),"./",tr("(*)"));
		if (log.isEmpty()){return;}

#else	//get file name from main window
		MainWindow* mainwnd_gf = MainWindow::GetMainWindow();
		QString log = mainwnd_gf->GetDirectory();
		log.append("\DicomSortList.txt");
#endif
		qDebug()<<log;
		//create file handle
		this->file.setFileName(log);
		//read file line by line
		if(!this->file.open(QIODevice::ReadOnly | QIODevice::Text))
			return;

		//use text stream to read line
		QTextStream in(&this->file);	
		//get col number
		//int col_num = ui->in_col->text().toInt();
		QString line1;
		qint64 line_number=0;

		dialogLoading_load_file->setValue(2);

		//clear File_info
		if (this->info != NULL)
		{
			delete this->info;
		}
		this->info = new File_info;

		while(!in.atEnd())
		{
			line1 = in.readLine();

			//find if there is "<",">"
			//if not, skip to next line
			if(!line1.contains("<",Qt::CaseInsensitive))
				continue;
			//split by "<" ">"
			QString name = line1.split("<").at(1);
			QString head = name.split(">").at(0);
			QString name_2 = name.split(">").at(1);

			if(head == "Patient_Name")
			{
				//qDebug()<<"Patient_Name";
				this->info->name.append(name_2);
			}
			if(head == "Series_Description")
			{
				//qDebug()<<"Series_Description";
				this->info->description.append(name_2);
			}
			if(head == "Slice_Number")
			{
				//qDebug()<<"Slice_Number";
				this->info->slice_number.append(name_2);
			}
			if(head == "NumberOfTemporalPositions")
			{
				//qDebug()<<"NumberOfTemporalPositions";
				this->info->temperal_number.append(name_2);
			}		
			if(head == "Path")
			{
				//qDebug()<<"path position";
				this->info->path_position<<in.pos();
			}
		}
		qDebug()<<"parse log done !";
		//close file
		this->file.close();

		dialogLoading_load_file->setValue(3);
		delete dialogLoading_load_file;

		//valid log file
		if(this->info->description.empty())
		{
			QMessageBox::information(NULL,"ERROR","The log file is invalid!",
				QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
			return;
		}

		//Tree display of series
		//if (ENABLE_TEST)
		if (this->ui->enable_test_Btn->isChecked())
		{
			this->select_index = 5;
		}
		else
		{
			Tree_Dialog* tree_index = new Tree_Dialog;
			connect(tree_index->index_tree,SIGNAL(itemClicked( QTreeWidgetItem *, int)),
				tree_index,SLOT(on_click_item(QTreeWidgetItem*,int)));
			tree_index->SetInfo(this->info,&this->select_index);	
			if (tree_index->exec()!=100)
			{
				delete tree_index;
				return;
			}
			delete tree_index;
		}
		
		qDebug()<<"index = "<<this->select_index;

		//parse from log and add data to container
		this->Log2Container(this->info, this->data_container,this->select_index);	
		
		qDebug()<<"load files successfully!!";
		
	}
	else
	{
		//-----connected multi slice volume------//
		QFileInfo fileinfo(this->filenames_list.at(0));
		QString directory=fileinfo.absolutePath();
		qDebug()<<directory;
		//connected volume
		QString suffix_dcm = "dcm";
		QString suffix_mha = "mha";
		QString suffix_nii = "nii";
		QString suffix_hdr = "hdr";
		for(int i=0;i<this->numVolume;i++)
		{
			QFileInfo fileinfox(this->filenames_list.at(i));
			QString directory_x = directory;
			QString full_name = directory_x.append(fileinfox.fileName());
			const char* filename = this->filenames_list.at(i).toStdString().data();//full_name.toStdString().data();
			qDebug()<<this->filenames_list.at(i).toStdString().data();
		
			if(fileinfox.suffix() == suffix_dcm)
			{//read dicom
				vtkSmartPointer<vtkDICOMImageReader> reader=
					vtkSmartPointer<vtkDICOMImageReader>::New();
				reader->SetFileName(this->filenames_list.at(i).toStdString().data());//(filename);
				reader->Update();
				qDebug()<<"reader "<<filename<<" successfully";
				//add file into data_container
				this->data_container[i] = reader->GetOutput();
			}
			else if(fileinfox.suffix() == suffix_mha)
			{
				//vtkSmartPointer<vtkMetaImageReader> reader=
				//	vtkSmartPointer<vtkMetaImageReader>::New();
				//reader->SetFileName(this->filenames_list.at(i).toStdString().data());//(filename);
				//qDebug()<<"reader "<<filename<<" successfully";
				//reader->Update();
				////add file into data_container
				//this->data_container[i] = reader->GetOutput();
			}
			else if(fileinfox.suffix() == suffix_nii || fileinfox.suffix() == suffix_hdr)
			{
				//nifti io
				//typedef itk::NiftiImageIO NiftiIOType;
				NiftiIOType::Pointer niftiIO = 
					NiftiIOType::New();	
				//itk read nii file
				//typedef itk::Image<float, 3> ImageType;
				//typedef itk::ImageSeriesReader< ImageType >  ReaderType;
				ReaderType_b::Pointer reader = ReaderType_b::New();
				reader->SetImageIO(niftiIO);
				reader->SetFileName(this->filenames_list.at(i).toStdString().data());
				reader->Update();

				//itk-vtk connector
				//typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
				i2vConnectorType::Pointer connector = i2vConnectorType::New();
				connector->SetInput(reader->GetOutput());
				connector->Update();

				this->data_container[i] = vtkSmartPointer<vtkImageData>::New();
				this->data_container[i]->DeepCopy(connector->GetOutput());
				qDebug()<<"(nii/hdr)reader "<<this->filenames_list.at(i).toStdString().data()<<" successfully";
			}

			else
			{
				QMessageBox::information(NULL,"File format error","Should be dcm/nii/mha",
					QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
			}
		}
	}
	//data preprocessing!!
//	if(ENABLE_TEST)
	//if (this->ui->enable_test_Btn->isChecked())
	//	{;}
	//else
	{PreProcess();}
	
	this->filenames_list.clear();
}

void SubWidgetParadigmInBold::on_click_addcontrast()
{	
	if (this->ui->Vector_in->text().isEmpty())
	{
		QMessageBox::about(NULL, "ERROR", "Contrast Empty");
		return;
	}
	//clear contrast vector in the first
	this->contrast->SetNumberOfComponents(1);
	for(int i=0;i<this->designMat->GetNumberOfComponents();i++)
	{
		this->contrast->InsertComponent(i,0,0);
	}
	//add component one by one 
	QStringList contrast_list = ui->Vector_in->text().split(",");
	for(int i=0;i<contrast_list.count();i++)
	{
		this->contrast->SetComponent(i,0,contrast_list.at(i).toInt());
	}
	std::cout<<"number component:"<<std::endl;
	for (int i=0;i<this->contrast->GetNumberOfTuples();i++)
	{
		std::cout<<this->contrast->GetComponent(i,0)<<std::endl;
	}
	//clear edit box
	//ui->Vector_in->clear();
}
void SubWidgetParadigmInBold::on_select_contrast()
{
	QString str;
	str = this->ui->contrastNameBox->currentText();

	if (str == "default")
	{
		this->ui->Vector_in->setText(tr("1,0,0"));
	}
	this->on_click_addcontrast();
}

void SubWidgetParadigmInBold::on_click_activate()
{
	////check flag
	//if(this->pip_flag<90)
	//{	QMessageBox::information(NULL,"ERROR","have not fit model",
	//		QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
	//	return;
	//}
	
	//to see if there is input to this filter
	if(this->BetaMap==NULL)
	{
		QMessageBox::information(NULL,"activate","no beta map gen",
			QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
		return;
	}

	if(this->contrast->GetNumberOfTuples()==0)
	{
		QMessageBox::information(NULL,"Error","No contrast specified",
			QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
		return;
	}


	//clear input to GLMVolumeGenerator
	vtkSmartPointer<vtkGLMVolumeGenerator> GLMVolmeGenerator = 
		vtkGLMVolumeGenerator::New();

	//generating tMap
	GLMVolmeGenerator->SetContrastVector(this->contrast);
	GLMVolmeGenerator->SetDesignMatrix(this->designMat);
	GLMVolmeGenerator->AddInput(this->BetaMap);
	GLMVolmeGenerator->Update();
	this->tMap = GLMVolmeGenerator->GetOutput();
	//this->Register_process(this->tMap);
	qDebug()<<"activate done";
}

void SubWidgetParadigmInBold::on_click_threshold()
{
	//progress bar
	QProgressDialog* dialogLoading_pre_threshold = new QProgressDialog("Thresholding", "", 0, 2 , this);
	dialogLoading_pre_threshold->setWindowModality(Qt::WindowModal);
	dialogLoading_pre_threshold->setCancelButton(0);
	QLabel* label_temp_pt = new QLabel (dialogLoading_pre_threshold);
	label_temp_pt->setStyleSheet("color: rgb(255, 255, 255);");
	dialogLoading_pre_threshold->setLabel(label_temp_pt);
	dialogLoading_pre_threshold->setMinimumDuration(0);
	dialogLoading_pre_threshold->setLabelText("Running Threshold..");
	dialogLoading_pre_threshold->setValue(1);

	//-----------fit model-------//
	this->on_click_fitmode();
	dialogLoading_pre_threshold->setValue(1);

	//activate --->t map
	this->on_click_activate();
	dialogLoading_pre_threshold->setValue(2);
	delete dialogLoading_pre_threshold;

	this->on_threshold_changed();
}
void SubWidgetParadigmInBold::on_threshold_changed()
{
	//thresholding
	int di[3];
	this->tMap->GetDimensions(di);

	int dof = this->numVolume-1;
	this->p = ui->pValue_in->value();
	this->t=this->CDF->p2t(this->p,dof);

	//set t to display
	ui->tStat_in->setText(QString::number(this->t));
	qDebug()<<"threshold t : "<<this->t;

	vtkImageData* threshold_temp = vtkImageData::New();
		//-------threshold----//
	if (0)
	{	
		vtkSmartPointer<vtkIsingActivationThreshold> IsingActivationThreshold= 
		vtkIsingActivationThreshold::New();
		IsingActivationThreshold->AddInput(this->tMap);
		IsingActivationThreshold->Setthreshold(this->t);
		IsingActivationThreshold->Update();
		threshold_temp = IsingActivationThreshold->GetOutput();

		//the following code is vtk image data to itk image data 
		//write to .nii file!
		//image cast
		vtkSmartPointer<vtkImageCast> caster = 
			vtkSmartPointer<vtkImageCast>::New();
		caster->SetInput(threshold_temp);
		caster->SetOutputScalarTypeToFloat();
		caster->Update();
		//cast pixel type to float type
		this->ActArea = caster->GetOutput();
		this->ActArea->GetDimensions(di);
		std::cout<<"act_area info"<<std::endl;
		std::cout<<di[0]<<di[1]<<di[2]<<std::endl;
	
	}
	else
	{
		vtkSmartPointer<ActivationThreshold> act_Threshold_hd = vtkSmartPointer<ActivationThreshold>::New();
		float neg_threshold = -this->t;
		float pos_threshold =  this->t;
		act_Threshold_hd->Setneg_threshold(neg_threshold);
		act_Threshold_hd->Setpos_threshold(pos_threshold);
		act_Threshold_hd->SetInput(this->tMap);
		act_Threshold_hd->Update();
		threshold_temp = act_Threshold_hd->GetOutput();
		this->ActArea->DeepCopy(threshold_temp);
		this->ActArea->GetDimensions(di);
		std::cout<<"act_area info"<<std::endl;
		std::cout<<di[0]<<di[1]<<di[2]<<std::endl;
	}

	this->Register_process(this->ActArea);
	//double act_origin[3];
	//this->ActArea->GetOrigin(act_origin);
	//qDebug()<<"origin"<<act_origin[0]<<act_origin[1]<<act_origin[2];

	//int act_dimensions[3];
	//this->ActArea->GetDimensions(act_dimensions);
	//qDebug()<<"dimensions"<<act_dimensions[0]<<act_dimensions[1]<<act_dimensions[2];
	qDebug()<<"threshold done";
#ifndef QS_DEBUG_MODE
	/***************render here*************/
	//Visualize
	MainWindow* mainwnd = MainWindow::GetMainWindow();

	LayerBold* m_layerBold = new LayerBold();
	m_layerBold->SetUniqueName("activation area");
	m_layerBold->Create(this->ActArea);
	m_layerBold->SetLookUpTable();

	LayerCollection* col_bold = mainwnd->GetLayerCollection( "Bold" );
	col_bold->AddLayer(m_layerBold);

	//Refresh Fiducial Actor
	RenderView* view_s = mainwnd->GetRenderView(0);
	RenderView* view_c = mainwnd->GetRenderView(1);
	RenderView* view_a = mainwnd->GetRenderView(2);
	RenderView* view_3d = mainwnd->GetRenderView(3);

	((RenderView2D*) view_s)->AppendLayerOnTop(m_layerBold);
	((RenderView2D*) view_c)->AppendLayerOnTop(m_layerBold);
	((RenderView2D*) view_a)->AppendLayerOnTop(m_layerBold);
	((RenderView3D*) view_3d)->AppendLayerOnTop(m_layerBold);
	/**************render here****************/
#endif
}

void SubWidgetParadigmInBold::keyPressEvent(QKeyEvent *event)
{
	if(event->key() == Qt::Key_0)
	{
		int dof = this->numVolume-1;
		this->p = ui->pValue_in->text().toFloat();
		this->t=this->CDF->p2t(this->p,dof);

		////use FDR
		//vtkSmartPointer<vtkActivationFalseDiscoveryRate> th_FDR = 
		//	vtkActivationFalseDiscoveryRate::New();
		//th_FDR->SetOption(1);//1:cind;2:cdep
		//th_FDR->SetInput(this->tMap);
		//th_FDR->SetDOF(this->numVolume-1);
		//th_FDR->Update();
		//this->t = th_FDR->GetFDRThreshold();

		//set t to display
		ui->tStat_in->setText(QString::number(this->t));
		qDebug()<<"threshold t : "<<this->t;
	}
}


void SubWidgetParadigmInBold::on_click_plot()
{

}

void SubWidgetParadigmInBold::init_para()	
{

	//pipeline flag
	this->pip_flag = 0;
	//file name information
	this->info = new File_info;
	//set TR to default
	this->TR = 3.0;
	//paradigm
	this->num_Condition = 0;
	
	//this->onset = vtkFloatArray::New();
	//this->onset->SetNumberOfComponents(5);
	//this->duration = vtkFloatArray::New();
	//this->duration->SetNumberOfComponents(5);

	//parameters
	this->numVolume=0;
	this->siglm = new SignalModeling;      //model paradigm
	this->designMat = vtkFloatArray::New();   //design matrix
	this->contrast  = vtkIntArray::New();	//contrast vector
	this->t = 0.0;
	this->p = 0.0;

	//GLMDector
	this->GLMDetector = vtkGLMDetector::New();
	//GLMEstimator
	this->GLMEstimator = vtkGLMEstimator::New();
	//CDF:compute t
	this->CDF = vtkCDF::New();
	//betamap
	this->BetaMap = vtkImageData::New();
	//t map
	this->tMap = vtkImageData::New();
	//act area
	this->ActArea = vtkImageData::New();
	
	//--------p value input----//
	//set p value input spinbox parameters
	this->ui->pValue_in->setValue(0.01);//p value default=0.01
	this->ui->pValue_in->setRange(0.00,0.10);//set p value vary range
	this->ui->pValue_in->setSingleStep(0.001); //set inc/dec step
	this->ui->pValue_in->setDecimals(3); //set precision

	//--------init condition select-----//
	QListWidget* list_widget_condition_item = new QListWidget;
	this->ui->conditionNameBox->setModel(list_widget_condition_item->model());
	this->ui->conditionNameBox->setView(list_widget_condition_item);
	this->ui->conditionNameBox->setEditable(true);
	this->ui->conditionNameBox->addItem((QWidget::tr("default")));
	this->ui->conditionNameBox->setEditText(tr("please select"));

	//--------init contrast vector select-------//
	QListWidget* list_widget_contrast_item = new QListWidget;
	this->ui->contrastNameBox->setModel(list_widget_contrast_item->model());
	this->ui->contrastNameBox->setView(list_widget_contrast_item);
	this->ui->contrastNameBox->setEditable(true);
	this->ui->contrastNameBox->addItem((QWidget::tr("default")));
	this->ui->contrastNameBox->setEditText(tr("please select"));

	this->advance_dialog = new Dialog_Paradigm_Advance;
}


void SubWidgetParadigmInBold::Log2Container(File_info* info, 
	std::map<int,vtkSmartPointer<vtkImageData> >& container,int index)
{
	int size = 0;
	int slice_number = 0;
	int volume_number= 0;
	//find number of item in QStringList
	size = info->description.size();
	slice_number = info->slice_number.at(index).toInt();
	volume_number= info->temperal_number.at(index).toInt();
	//add number of volumes ;
	this->numVolume = volume_number;

	//progress bar
	QProgressDialog* dialogLoading_File = new QProgressDialog("Copying files...", "", 0, this->numVolume, this);
	dialogLoading_File->setWindowModality(Qt::WindowModal);
	dialogLoading_File->setCancelButton(0);
	QLabel* label_temp = new QLabel (dialogLoading_File);
	label_temp->setStyleSheet("color: rgb(255, 255, 255);");
	dialogLoading_File->setLabel(label_temp);
	dialogLoading_File->setMinimumDuration(0);
	dialogLoading_File->setValue(0);
	dialogLoading_File->setLabelText("Reading fMRI Images!");
	dialogLoading_File->setValue(1);

	qDebug()<<"volume info:"<<info->description.at(index)
		<<info->slice_number.at(index)<<info->temperal_number.at(index);
	
	//--read log again to find file path--//
	//read file line by line
	if(!this->file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	//use text stream to read line
	QTextStream in(&this->file);
	//seek to path
	in.seek(info->path_position.at(index));
	//QString to hold file names
	std::vector< std::vector<std::string> > name_holder(volume_number);
	int cnt = 0;
	while(cnt<slice_number-(slice_number%volume_number))
	{
		QString f_name = in.readLine();
		int xx = cnt%volume_number;
		name_holder[xx].push_back(f_name.toStdString());
		cnt++;
	}
	this->file.close();
	//to find slice number and volume number match or not
	if(name_holder.begin()->size()!=name_holder.back().size())
	{
		QMessageBox::about(NULL, "ERROR", "slice and volume do not match");
		return;
	}

	//define itk reader
	//typedef itk::Image< float , 3 >             ImageType; //image type pixel:float;dimension:3
	//typedef itk::ImageSeriesReader< ImageType > ReaderType;
	ReaderType_b::Pointer itk_reader = ReaderType_b::New();
	DicomIOType::Pointer  dicomIO = DicomIOType::New();
	itk_reader->SetImageIO(dicomIO);

	//typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
	i2vConnectorType::Pointer connector = i2vConnectorType::New();

	//clear data container if not NULL
	if(!container.empty())
		container.clear();



	for(int i = 0;i<volume_number;i++)
	{
		itk_reader->SetFileNames(name_holder[i]);
		itk_reader->Update();
		//get information from dicom tags( thickness among slices)
		const  DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();
		DictionaryType::ConstIterator itr = dictionary.Begin();
		DictionaryType::ConstIterator end = dictionary.End();

		std::string sliceThicknessID = "0018|0050";
		std::string sliceOriginID    = "0020|0032";
		DictionaryType::ConstIterator Thickness_tagItr = dictionary.Find( sliceThicknessID );
		DictionaryType::ConstIterator Origin_tagItr    = dictionary.Find( sliceOriginID );

		if( Thickness_tagItr == end | Origin_tagItr == end)
		{
			std::cerr << "Tag " << sliceThicknessID<<"or"<<"sliceOriginID";
			std::cerr << " not found in the DICOM header" << std::endl;
			return;
		}
		MetaDataStringType::ConstPointer Thickness_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Thickness_tagItr->second.GetPointer() );
		MetaDataStringType::ConstPointer Origin_entryvalue = 
			dynamic_cast<const MetaDataStringType *>( Origin_tagItr->second.GetPointer() );

		std::string Thickness_tagvalue;
		std::string Origin_tagvalue;

		if( Thickness_entryvalue || Origin_entryvalue)
		{
			Thickness_tagvalue = Thickness_entryvalue->GetMetaDataObjectValue();
			Origin_tagvalue    = Origin_entryvalue   ->GetMetaDataObjectValue();
			std::cout << "slice thickness is (" << sliceThicknessID <<  ") ";
			std::cout << " is: " << Thickness_tagvalue << std::endl;
			std::cout << "origin is (" << sliceOriginID <<  ") ";
			std::cout << " is: " << Origin_tagvalue << std::endl;
		}
		else
		{
			std::cerr << "Entry was not of string type" << std::endl;
			return ;
		}
		double slice_thickness = std::stod(Thickness_tagvalue);

		//connect
		connector->SetInput(itk_reader->GetOutput());
		connector->Update();
		//output vtkimagedata to data_container
		//vtksmartpointer act as buff
		vtkSmartPointer<vtkImageData> buff =
			vtkSmartPointer<vtkImageData>::New();
		buff = connector->GetOutput();
		qDebug()<<"converter: "<<i<<" done!";
		//write to .mha for test
		//here we found a tough bug --- tem-solution: write to a file and read again...
		if(1)
		{		
			//-----------change information(origin and spacing)--------------//
			vtkSmartPointer<vtkImageChangeInformation> changer = 
				vtkSmartPointer<vtkImageChangeInformation>::New();
			//get information :origin
			//it seems that there is no need to change origin
			double origin[3]={0,0,0};
			buff->GetOrigin(origin);
			
			qDebug()<<"origin:"<<origin[0]<<origin[1]<<origin[2];
			
			double spacing[3]={0,0,0};
			buff->GetSpacing(spacing);
			spacing[2] = slice_thickness;
			changer->SetOutputSpacing(spacing);
			//change 
			changer->SetInput(buff);
			//changer->SetOutputOrigin(origin);
			changer->Update();

			//vtkSmartPointer<vtkImageData> buff_c =
			//	vtkSmartPointer<vtkImageData>::New();
			//buff_c = changer->GetOutput();
			container[i] = vtkSmartPointer<vtkImageData>::New();
			container[i]->DeepCopy(changer->GetOutput());
		}
		//change process bar
		dialogLoading_File->setValue(i);
		// if cancel read image, stop and clear all
		if (dialogLoading_File->wasCanceled())
		{
			//close file first
			this->file.close();
			//clear data container 
			this->data_container.clear();
			//delete 
			break;
		}
	}
	if (this->file.isOpen())
	{
		this->file.close();
	}
	//delete progress dialog
	delete dialogLoading_File;
}

void SubWidgetParadigmInBold::PreProcess()
{
	//progress bar
	QProgressDialog* dialogLoading_reg_strip = new QProgressDialog("Pre-Process", "", 0, 2 , this);
	dialogLoading_reg_strip->setWindowModality(Qt::WindowModal);
	dialogLoading_reg_strip->setCancelButton(0);
	QLabel* label_temp_bs = new QLabel (dialogLoading_reg_strip);
	label_temp_bs->setStyleSheet("color: rgb(255, 255, 255);");
	dialogLoading_reg_strip->setLabel(label_temp_bs);
	dialogLoading_reg_strip->setMinimumDuration(0);
	dialogLoading_reg_strip->setLabelText("Running Brain Strip..");
	dialogLoading_reg_strip->setValue(1);

	qDebug()<<"pre-processing ! !";
	//---creat T2 reference image----//
	this->Get_Register_Image_process();
	dialogLoading_reg_strip->setValue(1);
	//---------brain striping--------//
	this->Brain_Striping_process();
	dialogLoading_reg_strip->setValue(2);
	delete dialogLoading_reg_strip;


	//progress bar
	QProgressDialog* dialogLoading_File_reg = new QProgressDialog("Pre-Process", "", 0, this->data_container.size(), this);
	dialogLoading_File_reg->setWindowModality(Qt::WindowModal);
	dialogLoading_File_reg->setCancelButton(0);
	QLabel* label_temp = new QLabel (dialogLoading_File_reg);
	label_temp->setStyleSheet("color: rgb(255, 255, 255);");
	dialogLoading_File_reg->setLabel(label_temp);
	dialogLoading_File_reg->setMinimumDuration(0);
	dialogLoading_File_reg->setValue(0);
	dialogLoading_File_reg->setLabelText("Running Registration..");
	dialogLoading_File_reg->setValue(1);
	//0. registration
	typedef Register< float, float, float > fMRI2T2RegisterType_pre;
	vtkSmartPointer<vtkImageData> temp_fixed_fmri_image = vtkSmartPointer<vtkImageData>::New();
	temp_fixed_fmri_image->DeepCopy(this->data_container[0]);

	v2iConnectorType::Pointer v2iconnector_pre_temp = v2iConnectorType::New();
	v2iconnector_pre_temp->SetInput(temp_fixed_fmri_image);
	try
	{
		v2iconnector_pre_temp->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "**Error converting vtk type to itk type" << std::endl;
		std::cerr << err << std::endl;
		return ;
	}
	ImageTypex::Pointer itk_temp_fixed_fmri_image = v2iconnector_pre_temp->GetOutput();

	//generate transform matrix
	fMRI2T2RegisterType_pre * reg_first_fmri = new fMRI2T2RegisterType_pre;    
	reg_first_fmri->SetFixedImage( this->T2_image_reference );
	reg_first_fmri->SetMovingImage( itk_temp_fixed_fmri_image );
	reg_first_fmri->GenerateTranformMatrix();    
	this->T2_fmri_register_matrix = reg_first_fmri->GetTranformMatrix();
	delete reg_first_fmri;
	std::cout<<"register first with T2 image done"<<std::endl;

	//iterate through the data container
	int tt=0;
	for(std::map<int,vtkSmartPointer<vtkImageData> >::iterator it = (this->data_container.begin())++;
		it!=this->data_container.end();++it)////////////////////////////////
	{
		//progress change
		tt++;
		dialogLoading_File_reg->setValue(tt);

		//vtk image data to itk image data 
		v2iConnectorType::Pointer v2iconnector_pre_temp = v2iConnectorType::New();
		v2iconnector_pre_temp->SetInput((*it).second);
		try
		{
			v2iconnector_pre_temp->Update();
		}
		catch( itk::ExceptionObject &err )
		{
			std::cerr << "**Error converting vtk type to itk type" << std::endl;
			std::cerr << err << std::endl;
			return ;
		}

		ImageTypex::Pointer registered_fmri_img     = ImageTypex::New(); 
		fMRI2T2RegisterType_pre * reg1 = new fMRI2T2RegisterType_pre;    
		reg1->SetFixedImage( itk_temp_fixed_fmri_image );
		reg1->SetMovingImage( v2iconnector_pre_temp->GetOutput() );
		reg1->GenerateTranformMatrix();    
		reg1->GetRegisteredMovingImage( registered_fmri_img );
		delete reg1;

		//convert back to vtk image
		i2vConnectorType::Pointer i2vconnector_pre_temp = i2vConnectorType::New();
		i2vconnector_pre_temp->SetInput(registered_fmri_img);
		try
		{
			i2vconnector_pre_temp->Update();
		}
		catch( itk::ExceptionObject &err )
		{
			std::cerr << "**Error converting vtk type to itk type" << std::endl;
			std::cerr << err << std::endl;
			return ;
		}
		(*it).second->DeepCopy(i2vconnector_pre_temp->GetOutput());

		std::cout<<".\n register "<<(*it).first<<" th image done \n \n"<<std::endl;
	}
	delete dialogLoading_File_reg;

	//progress bar for gassian smoothing
	QProgressDialog* dialogLoading_File_smooth = new QProgressDialog("Pre-Process", "", 0, this->data_container.size(), this);
	dialogLoading_File_smooth->setWindowModality(Qt::WindowModal);
	dialogLoading_File_smooth->setCancelButton(0);
	QLabel* label_temp1 = new QLabel (dialogLoading_File_smooth);
	label_temp1->setStyleSheet("color: rgb(255, 255, 255);");
	dialogLoading_File_smooth->setLabel(label_temp1);
	dialogLoading_File_smooth->setMinimumDuration(0);
	dialogLoading_File_smooth->setValue(0);
	dialogLoading_File_smooth->setLabelText("Running Gassian Smoothing..");
	//1.  Gassian smoothing
	int ii=0;
	for(std::map<int,vtkSmartPointer<vtkImageData> >::iterator it = this->data_container.begin();
		it!=this->data_container.end();++it)
	{
		//progress change
		dialogLoading_File_smooth->setValue(ii);

		vtkSmartPointer<vtkImageGaussianSmooth> smooth_h=
			vtkSmartPointer<vtkImageGaussianSmooth>::New();
		//smooth parameter
		//smooth_h->SetRadiusFactor(3);
		//smooth_h->SetStandardDeviation();
		vtkSmartPointer<vtkImageData> buff_h=
			vtkSmartPointer<vtkImageData>::New();
		smooth_h->SetInput((*it).second);

		smooth_h->Update();

		buff_h = smooth_h->GetOutput();
		(*it).second = buff_h;
		qDebug()<<"smoothing "<<ii++<<"th image";
	}
	delete dialogLoading_File_smooth;


	//progress bar for thresholding
	QProgressDialog* dialogLoading_File_thrd = new QProgressDialog("Pre-Process", "", 0, this->data_container.size(), this);
	dialogLoading_File_thrd->setWindowModality(Qt::WindowModal);
	dialogLoading_File_thrd->setCancelButton(0);
	QLabel* label_temp2 = new QLabel (dialogLoading_File_thrd);
	label_temp2->setStyleSheet("color: rgb(255, 255, 255);");
	dialogLoading_File_thrd->setLabel(label_temp2);
	dialogLoading_File_thrd->setMinimumDuration(0);
	dialogLoading_File_thrd->setValue(0);
	dialogLoading_File_thrd->setLabelText("Thresholding..");
	//2.   thresholding
	int jj=0;
	for(std::map<int,vtkSmartPointer<vtkImageData> >::iterator it = this->data_container.begin();
		it!=this->data_container.end();++it)
	{
		//progress change
		dialogLoading_File_thrd->setValue(jj);

		vtkSmartPointer<vtkImageThreshold> threshold_h=
			vtkSmartPointer<vtkImageThreshold>::New();
		//threshold parameter
		threshold_h->ThresholdBetween(150,2000);
		threshold_h->ReplaceOutOn();
		threshold_h->SetOutValue(0.0);
		threshold_h->SetInput((*it).second);
		threshold_h->Update();
		vtkSmartPointer<vtkImageData> buff_h=
			vtkSmartPointer<vtkImageData>::New();
		buff_h = threshold_h->GetOutput();
		(*it).second = buff_h;
		qDebug()<<"thresholding "<<jj++<<"th image";
	}
	delete dialogLoading_File_thrd;

	//3.  write to file for test
	int kk=0;
	for(std::map<int,vtkSmartPointer<vtkImageData> >::iterator it = this->data_container.begin();
		it!=this->data_container.end();++it)//"!=" do /"==" undo
	{
		char name[50];
		sprintf(name,"th-smod%02d.nii",kk++);

		//the following code is vtk image data to itk image data 
		//write to .nii file!
		v2iConnectorType::Pointer v2iconnector = 
			v2iConnectorType::New();
		v2iconnector->SetInput((*it).second);
		v2iconnector->Update();

		WriterType_b::Pointer nii_writer = 
			WriterType_b::New();
		nii_writer->SetInput(v2iconnector->GetOutput());
		nii_writer->SetFileName(name);
		nii_writer->Update();
		qDebug()<<"writing "<<kk<<"th image";
	}
	qDebug()<<"pre-processing done!";
}

void SubWidgetParadigmInBold::Design_plot()
{
	//--------put design matrix into QVector to display-------//
	QVector<double> X_axis(this->designMat->GetNumberOfTuples());
	vector< QVector<double> > Y_axis(this->designMat->GetNumberOfComponents());
	vector< QVector<double> > Y_axis_block(this->designMat->GetNumberOfTuples());
	for (int i=0;i<this->designMat->GetNumberOfTuples();i++)
	{
		X_axis[i]=double(i);
		for(int j=0;j<this->designMat->GetNumberOfComponents();j++)
		{
			Y_axis[j].push_back(double(this->designMat->GetComponent(i,j)));
			Y_axis_block[j].push_back(0);
		}
	}
	for(int i=0;i<this->onset.size();i++)
	{
		for (int j=0;j<this->onset[i].size();j++)
		{
			for (int k=this->onset[i][j];k<this->onset[i][j]+this->duration[i][j];k++)
			{
				Y_axis_block[i][k]=1;
			}
		}
	}
	//set up dialog
	QDialog* plot_dialog = new QDialog;
	plot_dialog->resize(600, 400);
	QScrollArea* scrollArea = new QScrollArea(plot_dialog);
	QCustomPlot* plot_graph = new QCustomPlot(scrollArea);
	//setup scroll area
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(plot_graph);
	scrollArea->setGeometry(QRect(20, 20, 580, 380));
	plot_graph->setObjectName(QString::fromUtf8("plot"));
	plot_dialog->setModal(false);

	// add two new graphs and set their look:
	plot_graph->addGraph();
	plot_graph->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
	plot_graph->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
	plot_graph->addGraph();
	plot_graph->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph
	// configure right and top axis to show ticks but no labels:
	// (see QCPAxisRect::setupFullAxesBox for a quicker method to do this)
	plot_graph->xAxis2->setVisible(false);//	upper axis visible
	plot_graph->xAxis2->setTickLabels(false);//
	plot_graph->yAxis2->setVisible(false);//	right axis visible
	plot_graph->yAxis2->setTickLabels(false);//
	//set axis end arrow;
	plot_graph->xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
	plot_graph->xAxis->setLabel("volume");
	plot_graph->yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
	plot_graph->yAxis->setLabel("density");
	//plot_graph->xAxis->
	// make left and bottom axes always transfer their ranges to right and top axes:
	connect(plot_graph->xAxis, SIGNAL(rangeChanged(QCPRange)), plot_graph->xAxis2, SLOT(setRange(QCPRange)));
	connect(plot_graph->yAxis, SIGNAL(rangeChanged(QCPRange)), plot_graph->yAxis2, SLOT(setRange(QCPRange)));
	
	//----- pass data points to graphs: ----//
	//input QVector
	plot_graph->graph(0)->setData(X_axis, Y_axis_block[0]);
	plot_graph->graph(0)->setName("DESIGN");
	plot_graph->graph(1)->setData(X_axis, Y_axis[0]);
	plot_graph->graph(1)->setName("BOLD Model");

	// let the ranges scale themselves so graph 0 fits perfectly in the visible area:
	plot_graph->graph(0)->rescaleAxes();
	// same thing for graph 1, but only enlarge ranges (in case graph 1 is smaller than graph 0):
	plot_graph->graph(1)->rescaleAxes(true);
	// Note: we could have also just called customPlot->rescaleAxes(); instead
	// Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
	plot_graph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);


	plot_dialog->show();
}


void SubWidgetParadigmInBold::Brain_Striping_process()
{
	if (en_Brain_Strip_flag)
	{
		qDebug()<<"begin brain strip";
		double startTime = time(NULL);
		AtlasReaderType::Pointer atlasReader = AtlasReaderType::New();
		LabelReaderType::Pointer labelReader = LabelReaderType::New();	
		atlasReader->SetFileName(BRAINSTRIP_SRC);//("C:/Users/rc-mic/Desktop/Project-64bit/NeuroAres-223/BET/atlasImage.mha");
		labelReader->SetFileName(BRAINSTRIP_MASK);//("C:/Users/rc-mic/Desktop/Project-64bit/NeuroAres-223/BET/atlasMask.mha");
		try
		{
			atlasReader->Update();
			labelReader->Update();
		}
		catch ( itk::ExceptionObject &exception )
		{
			std::cerr << "Exception caught ! " << std::endl;
			std::cerr << exception << std::endl;
			return ;
		}
		// set up skull-stripping filter
		typedef itk::StripTsImageFilter<ImageTypex, AtlasImageType, AtlasLabelType> StripTsFilterType;
		StripTsFilterType::Pointer stripTsFilter = StripTsFilterType::New();

		// set the required inputs for the stripTsImageFilter
		stripTsFilter->SetInput(this->T2_image_reference);          //input image
		stripTsFilter->SetAtlasImage( atlasReader->GetOutput() );
		stripTsFilter->SetAtlasBrainMask( labelReader->GetOutput() );
		try
		{
			std::cerr << "Try brain strip!! " << std::endl;
			stripTsFilter->Update();
		}
		catch ( itk::ExceptionObject &exception )
		{
			std::cerr << "brain strip error !! " << std::endl;
			std::cerr << "Exception caught ! " << std::endl;
			std::cerr << exception << std::endl;
			return ;
		}
		//write mask for test
		typedef itk::ImageFileWriter  < AtlasLabelType > WriterType_mask;
		WriterType_mask::Pointer nii_writer_mask = WriterType_mask::New();
		this->Brain_Mask = AtlasLabelType::New();
		this->Brain_Mask = stripTsFilter->GetOutput();
		nii_writer_mask->SetInput( this->Brain_Mask);
		nii_writer_mask->SetFileName("mask.nii");
		try
		{
			nii_writer_mask->Update();
		}
		catch( itk::ExceptionObject &err )
		{
			std::cerr << "error write mask";
			std::cerr << err;
			return ;
		}
		double endTime = time(NULL);
		qDebug()<<"brain strip done";
		std::cout << "Total computation time: " << endTime-startTime << "seconds " << std::endl;
	}
}


void SubWidgetParadigmInBold::Get_Register_Image_process()
{
	if (en_Read_Register_flag)
	{
		//select registeration reference image
		if (this->ui->enable_test_Btn->isChecked())
		{
			this->select_index = 1;
		}
		else
		{
			Tree_Dialog* tree_index = new Tree_Dialog;
			tree_index->setWindowTitle("Select a Registration Reference Image");
			tree_index->setObjectName("Registeration image");
			connect(tree_index->index_tree,SIGNAL(itemClicked( QTreeWidgetItem *, int)),
				tree_index,SLOT(on_click_item(QTreeWidgetItem*,int)));
			tree_index->SetInfo(this->info,&this->select_index);
			if (tree_index->exec()!=100)
			{
				delete tree_index;
				return;
			}
			delete tree_index;
		}

		qDebug()<<"index = "<<this->select_index;
		//--read log again to find path--//
		//read file line by line
		if(!this->file.open(QIODevice::ReadOnly | QIODevice::Text))
			return;
		//use text stream to read line
		QTextStream in(&this->file);
		//seek to path
		in.seek(info->path_position.at(this->select_index));
		int slice_number = info->slice_number.at(this->select_index).toInt();
		//QString to hold file names
		std::vector<std::string>  name_holder;
		int cnt=0;
		while(cnt<slice_number)
		{
			QString f_name = in.readLine();
			name_holder.push_back(f_name.toStdString());
			cnt++;
		}
		this->file.close();

		//read dicom
		ReaderType_b::Pointer itk_reader_refer = ReaderType_b::New();
		DicomIOType::Pointer  dicomIO = DicomIOType::New();
		itk_reader_refer->SetImageIO(dicomIO);
		itk_reader_refer->SetFileNames(name_holder);
		itk_reader_refer->Update();
		//put to reference image container
		this->T2_image_reference = itk_reader_refer->GetOutput();
		//save T2_image for test
		WriterType_b::Pointer nii_writer = 
			WriterType_b::New();
		nii_writer->SetInput(this->T2_image_reference);
		nii_writer->SetFileName("T2_refer.nii");
		nii_writer->Update();
	}
	else
	{
#ifndef QS_DEBUG_MODE
		MainWindow* mainwndx = MainWindow::GetMainWindow();
		LayerCollection* lc_volume = mainwndx->GetLayerCollection( "Volume" );
		//find empty of volum_list
		QList<Layer*> volum_list = lc_volume->GetLayers();
		this->T2_image_reference =  ((LayerVolume*)volum_list[0])->GetITKImage();
		//T2_image = ((LayerVolume*)volum_list[0])->GetITKImage();
#endif
	}
}



void SubWidgetParadigmInBold::Register_process(vtkImageData* &input_im)
{
	//convert act_area to itk image
	v2iConnectorType::Pointer v2iconnector_a = 
		v2iConnectorType::New();
	v2iconnector_a->SetInput(input_im);
	try
	{
		v2iconnector_a->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "error convert act_image to itk";
		std::cerr << err;
		return ;
	}

	//-----apply matrix----//
	std::cout<<"applying transform matrix"<<std::endl;
	typedef itk::ResampleImageFilter< ImageTypex, ImageTypex > ResampleFilterType;
	ResampleFilterType::Pointer resampler_in_register = ResampleFilterType::New();
	resampler_in_register->SetTransform( this->T2_fmri_register_matrix );
	resampler_in_register->SetInput( v2iconnector_a->GetOutput() ); 
	resampler_in_register->SetSize( this->T2_image_reference->GetLargestPossibleRegion().GetSize() );
	resampler_in_register->SetOutputOrigin(  this->T2_image_reference->GetOrigin() );
	resampler_in_register->SetOutputSpacing( this->T2_image_reference->GetSpacing() );
	resampler_in_register->SetOutputDirection( this->T2_image_reference->GetDirection() );
	resampler_in_register->SetDefaultPixelValue( 0 );
	try
	{
		resampler_in_register->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "**Error resampling image" << std::endl;
		std::cerr << err << std::endl;
		return ;
	}
	std::cout<<"apply transform matrix done"<<std::endl;

	//-----------mask regd_activate_img-------//
	ImageTypex::Pointer masked_activate_img = ImageTypex::New();
	if (en_Brain_Strip_flag)
	{
		std::cout<<"putting mask"<<std::endl;
		typedef itk::MaskImageFilter<ImageTypex, AtlasLabelType, ImageTypex> MaskFilterType;
		MaskFilterType::Pointer maskFilter = MaskFilterType::New();
		maskFilter->SetInput1(  resampler_in_register->GetOutput() );
		maskFilter->SetInput2( this->Brain_Mask );
		try
		{
			maskFilter->Update();
		}
		catch ( itk::ExceptionObject &exception )
		{
			std::cerr << "mask error!!" << std::endl;
			std::cerr << "Exception caught ! " << std::endl;
			std::cerr << exception << std::endl;
			return ;
		}
		masked_activate_img = maskFilter->GetOutput();
		std::cout<<"mask process done"<<std::endl;
	}
	else
	{	
		masked_activate_img = resampler_in_register->GetOutput();
	}
	//--------------mask regd_activate_img-----------//


	//put registered image to container
	i2vConnectorType::Pointer i2vconnector_im2con = 
		i2vConnectorType::New();
	i2vconnector_im2con->SetInput(masked_activate_img);
	try
	{
		i2vconnector_im2con->Update();
	}
	catch ( itk::ExceptionObject &exception )
	{
		std::cerr << "itk to vtk error!!" << std::endl;
		std::cerr << "Exception caught ! " << std::endl;
		std::cerr << exception << std::endl;
		return ;
	}

	input_im->DeepCopy(i2vconnector_im2con->GetOutput());

	//write activated area to nii
	WriterType_b::Pointer nii_writer_itkrgd_image = 
		WriterType_b::New();
	nii_writer_itkrgd_image->SetInput(masked_activate_img);
	nii_writer_itkrgd_image->SetFileName("act_area.nii");
	try
	{
		nii_writer_itkrgd_image->Update();
	}
	catch( itk::ExceptionObject &err )
	{
		std::cerr << "error write act_image";
		std::cerr << err;
		return ;
	}
}





int SubWidgetParadigmInBold::Bold_Visualize(const vtkImageData* & img_to_vis)
{

	return 0;
}

void SubWidgetParadigmInBold::on_click_test()
{
	//to see if enable test
	if (this->ui->enable_test_Btn->isChecked())
		;
	else
	{
		qDebug()<<"test unable";
		return;
	}
	//--------add condation---------//
	vector<int> onset_tem;
	vector<int> duration_tem;
	onset_tem.push_back(10);
	onset_tem.push_back(30);
	onset_tem.push_back(50);
	duration_tem.push_back(10);
	duration_tem.push_back(10);
	duration_tem.push_back(10);

	this->onset.push_back(onset_tem);
	this->duration.push_back(duration_tem);
	this->num_Condition = 1;

	//------------add file---------//
	this->on_click_loadimage();

	//------generate design matrix-----//
	this->on_click_viewdesign();

	//-----------fit model-------//
	this->on_click_fitmode();

	//-----add contrast vector-------//
	this->contrast->SetNumberOfComponents(1);
	for(int i=0;i<this->designMat->GetNumberOfComponents();i++)
	{
		this->contrast->InsertComponent(i,0,0);
	}
	//add component one by one 
	this->contrast->InsertComponent(0,0,1);
	
	//activate --->t map
	this->on_click_activate();

	this->on_click_threshold();
}

void SubWidgetParadigmInBold::on_click_advance()
{
	if (this->advance_dialog->exec()!=100)
	{
		return;
	}
	if (this->advance_dialog->number_of_time_point!=this->numVolume)
	{
		QMessageBox::warning(this,tr("Paradigm Error"),tr("Number of Volume incorrect"),QMessageBox::Yes);
		return;
	}
	this->designMat = this->advance_dialog->designMat;
	this->contrast  = this->advance_dialog->contrast_vector;
}

void SubWidgetParadigmInBold::on_click_user_input()
{
	this->on_click_addcondition();
	this->on_click_viewdesign();
	this->on_click_addcontrast();
}

//----------tree widget-------------//

Tree_Dialog::Tree_Dialog(QWidget *parent): QDialog(parent)
{
	setWindowTitle("Select Index");
	index_tree = new QTreeWidget(this);
	index_tree->setObjectName(QString::fromUtf8("index_tree"));
	index_tree->setGeometry(QRect(20, 10, 351, 271));

	QStringList head;
	head<<"Name"<<"Volume";
	index_tree->setHeaderLabels(head);
}

Tree_Dialog::~Tree_Dialog()
{
}

void Tree_Dialog::on_click_item(QTreeWidgetItem* item,int column)
{

	int col = this->index_tree->indexOfTopLevelItem(item);
	
	qDebug()<<"selected col:"<<col;
	
	*this->index = col;
	this->done(100);
	this->hide();
}

void Tree_Dialog::SetInfo(File_info* in_info, int * select_index)
{
	infox = in_info;
	index = select_index;
	if(infox->description.size() == 0)
	{
		QStringList item_list;
		item_list<<"nothing"<<"nothing";
		QTreeWidgetItem * child = new QTreeWidgetItem(index_tree,item_list);
		QMessageBox::about(NULL, "ERROR", "read log failed!");
		this->close();
	}
	else
	{
		for (int i = 0;i<infox->name.size();i++)
		{
			QStringList item_list;
			item_list<<infox->description.at(i)<<infox->temperal_number.at(i);
			QTreeWidgetItem* child = new QTreeWidgetItem(index_tree,item_list);
			index_tree->addTopLevelItem(child);
		}
	}
}

//----threshold---//
vtkStandardNewMacro(ActivationThreshold);
ActivationThreshold::ActivationThreshold()
{
	this->pos_threshold = 0;
	this->neg_threshold = 0;
}

ActivationThreshold::~ActivationThreshold()
{
}

void ActivationThreshold::SimpleExecute(vtkImageData *input, vtkImageData *output)
{ 

	this->GetInput(0)->GetDimensions(dims);
	x = dims[0];
	y = dims[1];
	z = dims[2];
	std::cout<<dims[0]<<dims[1]<<dims[2]<<std::endl;
	size = x*y*z;
	int numberOfInputs;
#if (VTK_MAJOR_VERSION >= 5)
	numberOfInputs = this->GetNumberOfInputConnections(0);
#else  
	numberOfInputs = this->NumberOfInputs;
#endif
	output->SetDimensions(dims);
	output->SetScalarType(VTK_FLOAT);
	output->SetSpacing(input->GetSpacing());
	output->SetOrigin(input->GetOrigin());
	output->AllocateScalars();

	// get the data array from input image 
	vtkFloatArray *inputArray = (vtkFloatArray *)this->GetInput(0)->GetPointData()->GetScalars();
	vtkFloatArray *activation = vtkFloatArray::New();
			for (unsigned long int i=0; i<size; i++)
			{
				if (inputArray->GetValue(i) >= pos_threshold)
				{
					activation->InsertNextValue(inputArray->GetValue(i));
				}
				else if (inputArray->GetValue(i) <= neg_threshold)
				{
					activation->InsertNextValue(inputArray->GetValue(i));
				}
				else
				{
					activation->InsertNextValue(0.0);
				}
			} 
	output->GetPointData()->SetScalars(activation);
	activation->Delete();
}

// If the output image of a filter has different properties from the input image
// we need to explicitly define the ExecuteInformation() method
void ActivationThreshold::ExecuteInformation(vtkImageData *input, vtkImageData *output)
{
	output->SetDimensions(dims);
	output->SetScalarType(VTK_INT);
	output->SetSpacing(1.0,1.0,1.0);
	output->SetOrigin(0.0,0.0,0.0);
	output->AllocateScalars();
}




//#include "moc_SubWidgetParadigmInBold.cxx"