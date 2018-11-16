#ifndef ONVIFDETECTDEVICE_H
#define ONVIFDETECTDEVICE_H

#include <QtGui/QWidget>
#include "ui_onvifdetectdevice.h"

class OnvifDetectDevice : public QWidget
{
	Q_OBJECT

public:
	OnvifDetectDevice(QWidget *parent = 0, Qt::WFlags flags = 0);
	~OnvifDetectDevice();

private slots:
	void pushButtonSearchSlotClicked();

private:
	Ui::OnvifDetectDeviceClass ui;
};

#endif // ONVIFDETECTDEVICE_H
