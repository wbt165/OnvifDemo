#include "onvifdemo.h"
#include <QtGui/QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
	QTextCodec *codec = QTextCodec::codecForName("GBK");
	QTextCodec::setCodecForTr(codec);
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForCStrings(codec);

	QApplication a(argc, argv);
	OnvifDemo w;
	w.show();
	return a.exec();
}
