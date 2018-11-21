#ifndef ONVIFDETECTDEVICE_H
#define ONVIFDETECTDEVICE_H

#include <QtGui/QWidget>
#include "ui_onvifdetectdevice.h"
#include <vector>
using namespace std;

#define SOAP_ASSERT     assert

#define SOAP_TO         "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION     "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"

#define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"                       // onvif规定的组播地址

#define SOAP_ITEM       ""                                                      // 寻找的设备范围
#define SOAP_TYPES      "dn:NetworkVideoTransmitter"                            // 寻找的设备类型

#define SOAP_SOCK_TIMEOUT    (10)                                               // socket超时时间（单秒秒）
#define ONVIF_ADDRESS_SIZE   (128)                                              // URI地址长度

#define USERNAME    "admin"
#define PASSWORD    "98765432i"

class soap;
class wsdd__ProbeType;
class __wsdd__ProbeMatches;

class CVideoEncoderConfiguration			// 视频编码器配置信息
{
public:
	CVideoEncoderConfiguration();
	~CVideoEncoderConfiguration();

	QString m_qsToken;					// 唯一标识该视频编码器的令牌字符串
	int m_nWidth;                       // 分辨率
	int m_Height;
};

/* 设备配置信息 */
class CProfile
{
public:
	CProfile();
	~CProfile();

	QString m_qsToken;						// 唯一标识设备配置文件的令牌字符串
	CVideoEncoderConfiguration m_objVideoEncoderConfiguration;
protected:

private:
};

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

/************************************************************************
**函数：getOnvifCapabilities
**功能：获取设备能力信息
**参数：
        [in] szAddrs - 设备服务地址
		[out] qsMediaXAddr - 设备媒体地址
**返回：
        0表明成功，非0表明失败
**备注：
    1). 其中最主要的参数之一是媒体服务地址
************************************************************************/
	int getOnvifCapabilities(const char* szAddrs, QString& qsMediaXAddr);

/************************************************************************
**函数：ONVIF_GetProfiles
**功能：获取设备的音视频码流配置信息
**参数：
        [in] qsMediaXAddr - 媒体服务地址
        [out] vecProfile  - 返回的设备音视频码流配置信息列表
**返回：
        返回设备可支持的码流数量（通常是主/辅码流），即是profiles列表个数
**备注：
        1). 注意：一个码流（如主码流）可以包含视频和音频数据，也可以仅仅包含视频数据。
************************************************************************/
	int getOnvifProfiles(const QString& qsMediaXAddr, vector<CProfile>& vecProfile);

/************************************************************************
**函数：getOnvifStreamUri
**功能：获取设备码流地址(RTSP)
**参数：
        [in]  qsMediaXAddr    - 媒体服务地址
        [in]  qsToken  - the media profile token
        [out] uri           - 返回的地址
**返回：
        0表明成功，非0表明失败
**备注：
************************************************************************/
	int getOnvifStreamUri(const QString& qsMediaXAddr,const QString& qsToken, QString& qsUri);

/************************************************************************
**函数：make_uri_withauth
**功能：构造带有认证信息的URI地址
**参数：
        [in]  qsUri       - 未带认证信息的URI地址
        [in]  username      - 用户名
        [in]  password      - 密码
        [out] qsUriAuth      - 返回的带认证信息的URI地址
**返回：
        0成功，非0失败
**备注：
    1). 例子：
    无认证信息的uri：rtsp://100.100.100.140:554/av0_0
    带认证信息的uri：rtsp://username:password@100.100.100.140:554/av0_0
************************************************************************/
	int makeUriWithauth(const QString& qsUri, const char* szUsername, const char * szPassword, QString& qsUriAuth);
private:
	Ui::OnvifDetectDeviceClass ui;
};

#endif // ONVIFDETECTDEVICE_H
