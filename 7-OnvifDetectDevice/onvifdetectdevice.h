#ifndef ONVIFDETECTDEVICE_H
#define ONVIFDETECTDEVICE_H

#include <QtGui/QWidget>
#include "ui_onvifdetectdevice.h"

#define SOAP_ASSERT     assert

#define SOAP_TO         "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION     "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"

#define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"                       // onvif规定的组播地址

#define SOAP_ITEM       ""                                                      // 寻找的设备范围
#define SOAP_TYPES      "dn:NetworkVideoTransmitter"                            // 寻找的设备类型

#define SOAP_SOCK_TIMEOUT    (10)                                               // socket超时时间（单秒秒）

#define USERNAME    "admin"
#define PASSWORD    "98765432i"

class soap;
class wsdd__ProbeType;
class __wsdd__ProbeMatches;

class OnvifDetectDevice : public QWidget
{
	Q_OBJECT

public:
	OnvifDetectDevice(QWidget *parent = 0, Qt::WFlags flags = 0);
	~OnvifDetectDevice();

private slots:
	void pushButtonSearchSlotClicked();

private:
	void detectDevice();
	soap* initOnvifSoap(int timeout);

/************************************************************************
**函数：initOnvifHeader
**功能：初始化soap描述消息头
**参数：
        [in] soap - soap环境变量
**返回：无
**备注：
    1). 在本函数内部通过ONVIF_soap_malloc分配的内存，将在ONVIF_soap_delete中被释放
************************************************************************/
	void initOnvifHeader(soap* pSoap);

/************************************************************************
**函数：initOnvifProbeType
**功能：初始化探测设备的范围和类型
**参数：
        [in]  soap  - soap环境变量
        [out] probe - 填充要探测的设备范围和类型
**返回：
        0表明探测到，非0表明未探测到
**备注：
    1). 在本函数内部通过ONVIF_soap_malloc分配的内存，将在ONVIF_soap_delete中被释放
************************************************************************/
	void initOnvifProbeType(soap* pSoap, wsdd__ProbeType* stuReq);
	void printSoapError(soap* pSoap, const char* szMsg = NULL);
	void printProbeMatches(const __wsdd__ProbeMatches* pstuRep);
	void printMsg(QString qsText);
	void deleteOnvifSoap(soap* pSoap);
	void* mallocOnvifSoap(soap* pSoap, int nLen);

/************************************************************************
**函数：getOnvifDeviceInformation
**功能：获取设备基本信息
**参数：
        [in] szAddrs - 设备服务地址
**返回：
        0表明成功，非0表明失败
**备注：
************************************************************************/
	int getOnvifDeviceInformation(const char* szAddrs);


/************************************************************************
**函数：setOnvifAuthInfo
**功能：设置认证信息
**参数：
        [in] soap     - soap环境变量
        [in] username - 用户名
        [in] password - 密码
**返回：
        0表明成功，非0表明失败
**备注：
************************************************************************/
	int setOnvifAuthInfo(soap* pSoap, const char* szUsername, const char* szPassword);

private:
	Ui::OnvifDetectDeviceClass ui;
};

#endif // ONVIFDETECTDEVICE_H
