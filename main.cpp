#include "SubWidgetParadigmInBold.h"
#include "Dialog_Paradigm_Advance.h"

//
#include <QApplication>
#include <QDialog>

int main( int argc, char** argv )
{
	QApplication fmri_test(argc,argv);

	SubWidgetParadigmInBold widget_bold;

	widget_bold.show();

	return fmri_test.exec();
}












