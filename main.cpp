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

	MainWindow fmri_main_window;
	ROIBasedPanel roi_based_panel;

	fmri_main_window.show();
	roi_based_panel.show();

	return fmri_main_app.exec();
}












