#include "MainWindow.h"
#include "ROIBasedPanel.h"
//
#include <QApplication>
#include <QMainWindow>

//support for Chinese
#include <QTextCodec>


int main( int argc, char** argv )
{
	QTextCodec *codec=QTextCodec::codecForName("GBK");
	QTextCodec::setCodecForLocale(codec);   
	QTextCodec::setCodecForTr(codec);   
	QTextCodec::setCodecForCStrings(codec);  

	QApplication fmri_main_app(argc,argv);

	//MainWindow fmri_main_window;

	//ROIBasedPanel roi_based_panel(&fmri_main_window);
	QWidget	* main_win = new MainWindow;
	MainWindow* textxxxx = new MainWindow;
	QWidget* bct_panel = new ROIBasedPanel(textxxxx);//( (MainWindow*) main_win);
	multi_thread_main* thread_main[1];
	thread_main[0] = new multi_thread_main(main_win);
	multi_thread_main* thread_bct[1];
	thread_bct[0]  = new multi_thread_main(bct_panel);

	thread_main[0]->run();
	thread_bct[0]->run();
	//fmri_main_window.show();
	//roi_based_panel.show();

	return fmri_main_app.exec();
}





