#ifndef ONVIFDEMO_H
#define ONVIFDEMO_H

#include <QtGui/QMainWindow>
#include "ui_onvifdemo.h"

class OnvifDemo : public QMainWindow
{
	Q_OBJECT

public:
	OnvifDemo(QWidget *parent = 0, Qt::WFlags flags = 0);
	~OnvifDemo();

private slots:
		void lineEditSlotTextChanged(QString qsText);

private:
	Ui::OnvifDemoClass ui;
};

#endif // ONVIFDEMO_H
