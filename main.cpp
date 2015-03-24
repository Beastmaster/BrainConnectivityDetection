#include "MainWindow.h"
//
#include <QApplication>
#include <QMainWindow>

int main( int argc, char** argv )
{
	QApplication fmri_main_app(argc,argv);

	MainWindow fmri_main_window;

	fmri_main_window.show();

	return fmri_main_app.exec();
}












