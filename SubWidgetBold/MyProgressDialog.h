#ifndef MyProgressDialog_h
#define MyProgressDialog_h

#include <QObject>
#include <QWidget>
#include <QProgressDialog>

class MyProgressDialog : public QProgressDialog
{
	Q_OBJECT

public:
	//Construtor, destructor
	MyProgressDialog ( QWidget *parent = 0);
	virtual ~MyProgressDialog();

protected:
};

#endif


