#include "MyProgressDialog.h"
#include <QLabel>
#include <QPalette>

MyProgressDialog::MyProgressDialog(QWidget *parent): QProgressDialog(parent)
{
	this->setWindowTitle("Please Wait");
	this->setWindowModality(Qt::WindowModal);
	this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint);
	this->setCancelButton(0);
	this->setMinimumDuration(0);
	this->setMinimum(0);
	this->setMaximum(100);
	
	QPalette pal = this->palette();
	pal.setColor(this->backgroundRole(),QColor(37,37,38));
	this->setPalette(pal);

	QLabel* label = new QLabel;
	label->setStyleSheet("color: rgb(255, 255, 255);");
	this->setLabel(label);
}

MyProgressDialog::~MyProgressDialog()
{
}