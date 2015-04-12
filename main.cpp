#include "MainWindow.h"
//
#include <QApplication>
#include <QMainWindow>

//
#include <QTextCodec>


int main( int argc, char** argv )
{
	QTextCodec *codec=QTextCodec::codecForName("GBK");
	QTextCodec::setCodecForLocale(codec);   
	QTextCodec::setCodecForTr(codec);   
	QTextCodec::setCodecForCStrings(codec);  

	QApplication fmri_main_app(argc,argv);

	MainWindow fmri_main_window;

	fmri_main_window.show();

	return fmri_main_app.exec();
}












