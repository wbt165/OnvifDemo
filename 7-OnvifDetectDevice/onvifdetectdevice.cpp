#include "onvifdetectdevice.h"
#include "stdsoap2.h"
#include "wsdd.nsmap"
#include "soapStub.h"
#include <assert.h>
#include "soapH.h"
#include "QTextEdit"
#include "wsseapi.h"
#include "wsaapi.h"

extern "C"
{
#include "libavformat/avformat.h"
};

OnvifDetectDevice::OnvifDetectDevice(QWidget *parent, Qt::WFlags flags)
	: QWidget(parent, flags)
{
	ui.setupUi(this);
}

OnvifDetectDevice::~OnvifDetectDevice()
{

}

void OnvifDetectDevice::pushButtonSearchSlotClicked()
{
	detectDevice();
}

void OnvifDetectDevice::detectDevice()
{
	int result = 0;
	unsigned int count = 0;                                                     // 搜索到的设备个数
	soap* pSoap = NULL;                                                   // soap环境变量
	wsdd__ProbeType      stuReq;                                            // 用于发送Probe消息
	__wsdd__ProbeMatches stuRep;                                            // 用于接收Probe应答
	wsdd__ProbeMatchType *probeMatch;

	SOAP_ASSERT(NULL != (pSoap = initOnvifSoap(SOAP_SOCK_TIMEOUT)));

	initOnvifHeader(pSoap);                                                    // 设置消息头描述
	initOnvifProbeType(pSoap, &stuReq);                                           // 设置寻找的设备的范围和类型
	result = soap_send___wsdd__Probe(pSoap, SOAP_MCAST_ADDR, NULL, &stuReq);        // 向组播地址广播Probe消息
	while (SOAP_OK == result)                                                   // 开始循环接收设备发送过来的消息
	{
		result = soap_recv___wsdd__ProbeMatches(pSoap, &stuRep);
		if (SOAP_OK == result)
		{
			if (pSoap->error)
			{
				printSoapError(pSoap, "ProbeMatches");
			}
			else
			{                                                            // 成功接收到设备的应答消息
				printProbeMatches(&stuRep);

				if (NULL != stuRep.wsdd__ProbeMatches)
				{
					count += stuRep.wsdd__ProbeMatches->__sizeProbeMatch;
					for(int i = 0; i < stuRep.wsdd__ProbeMatches->__sizeProbeMatch; i++)
					{
						probeMatch = stuRep.wsdd__ProbeMatches->ProbeMatch + i;

						QString qsAddrs = QString::fromUtf8(probeMatch->XAddrs);
						QStringList qslAddrs = qsAddrs.split(" ");

						getOnvifDeviceInformation(qslAddrs[0].toUtf8().data());
						QString qsMediaXAddr;
						getOnvifCapabilities(qslAddrs[0].toUtf8().data(), qsMediaXAddr);

						vector<CProfile> vecProfile;
						int nProfileCount = getOnvifProfiles(qsMediaXAddr, vecProfile);

						QString qsUri;			// 不带认证信息的URI地址
						QString qsUriAuth;		// 带有认证信息的URI地址

						if (nProfileCount > 0)
						{
							getOnvifStreamUri(qsMediaXAddr, vecProfile[0].m_qsToken, qsUri); // 获取RTSP地址

 							makeUriWithauth(qsUri, USERNAME, PASSWORD, qsUriAuth); // 生成带认证信息的URI（有的IPC要求认证）

							openRtsp(qsUriAuth);                                                    // 读取主码流的音视频数据
						}

					}
				}
			}
		}
		else if (pSoap->error)
		{
			break;
		}
	}

	printMsg(QString("detect end! It has detected %1 devices!").arg(count));

	if (NULL != pSoap)
	{
		deleteOnvifSoap(pSoap);
	}
}

soap* OnvifDetectDevice::initOnvifSoap(int timeout)
{
	soap *pSoap = NULL;                                                   // soap环境变量

	SOAP_ASSERT(NULL != (pSoap = soap_new()));

	soap_set_namespaces(pSoap, namespaces);                                      // 设置soap的namespaces
	pSoap->recv_timeout    = timeout;                                            // 设置超时（超过指定时间没有数据就退出）
	pSoap->send_timeout    = timeout;
	pSoap->connect_timeout = timeout;

#if defined(__linux__) || defined(__linux)                                      // 参考https://www.genivia.com/dev.html#client-c的修改：
	pSoap->socket_flags = MSG_NOSIGNAL;                                          // To prevent connection reset errors
#endif

	soap_set_mode(pSoap, SOAP_C_UTFSTRING);                                      // 设置为UTF-8编码，否则叠加中文OSD会乱码

	return pSoap;
}

void OnvifDetectDevice::initOnvifHeader(soap* pSoap)
{

	struct SOAP_ENV__Header *header = NULL;

	SOAP_ASSERT(NULL != pSoap);

	header = (struct SOAP_ENV__Header *)mallocOnvifSoap(pSoap, sizeof(struct SOAP_ENV__Header));
	soap_default_SOAP_ENV__Header(pSoap, header);
	header->wsa__MessageID = (char*)soap_wsa_rand_uuid(pSoap);
	header->wsa__To        = (char*)mallocOnvifSoap(pSoap, strlen(SOAP_TO) + 1);
	header->wsa__Action    = (char*)mallocOnvifSoap(pSoap, strlen(SOAP_ACTION) + 1);
	strcpy(header->wsa__To, SOAP_TO);
	strcpy(header->wsa__Action, SOAP_ACTION);
	pSoap->header = header;
}

void OnvifDetectDevice::initOnvifProbeType(soap* pSoap, wsdd__ProbeType* pstuReq)
{
	struct wsdd__ScopesType *scope = NULL;                                      // 用于描述查找哪类的Web服务

	SOAP_ASSERT(NULL != pSoap);
	SOAP_ASSERT(NULL != pstuReq);

	scope = (struct wsdd__ScopesType *)mallocOnvifSoap(pSoap, sizeof(struct wsdd__ScopesType));
	soap_default_wsdd__ScopesType(pSoap, scope);                                 // 设置寻找设备的范围
	scope->__item = (char*)mallocOnvifSoap(pSoap, strlen(SOAP_ITEM) + 1);
	strcpy(scope->__item, SOAP_ITEM);

//	memset(pstuReq, 0x00, sizeof(struct wsdd__ProbeType));
	soap_default_wsdd__ProbeType(pSoap, pstuReq);
	pstuReq->Scopes = scope;
	pstuReq->Types  = (char*)mallocOnvifSoap(pSoap, strlen(SOAP_TYPES) + 1);     // 设置寻找设备的类型
	strcpy(pstuReq->Types, SOAP_TYPES);
}

void OnvifDetectDevice::printSoapError(soap* pSoap, const char* szMsg)
{
	QString qsTest;
	if (NULL == szMsg)
	{
		qsTest.sprintf("[soap] error: %d, %s, %s", pSoap->error, *soap_faultcode(pSoap), *soap_faultstring(pSoap));
	}
	else
	{
		qsTest.sprintf("[soap] %s error: %d, %s, %s", szMsg, pSoap->error, *soap_faultcode(pSoap), *soap_faultstring(pSoap));
	}

	printMsg(qsTest);
}

void OnvifDetectDevice::printProbeMatches(const __wsdd__ProbeMatches* pstuRep)
{

}

void OnvifDetectDevice::printMsg(QString qsText)
{
	ui.textEditResult->append(qsText);
}

void OnvifDetectDevice::deleteOnvifSoap(soap* pSoap)
{
	soap_destroy(pSoap);                                                         // remove deserialized class instances (C++ only)
	soap_end(pSoap);                                                             // Clean up deserialized data (except class instances) and temporary data
	soap_done(pSoap);                                                            // Reset, close communications, and remove callbacks
	soap_free(pSoap);                                                            // Reset and deallocate the context created with soap_new or soap_copy
}

void* OnvifDetectDevice::mallocOnvifSoap(soap* pSoap, int nLen)
{
	void *p = NULL;

	if (nLen > 0) {
		p = soap_malloc(pSoap, nLen);
		SOAP_ASSERT(NULL != p);
//		memset(p, 0x00 ,nLen);
	}
	return p;
}

int OnvifDetectDevice::getOnvifDeviceInformation(const char* szAddrs)
{
	int result = 0;
	soap* pSoap = NULL;
	_tds__GetDeviceInformation           objDevinfoReq;
	_tds__GetDeviceInformationResponse   objDevinfoResp;

	SOAP_ASSERT(NULL != szAddrs);
	SOAP_ASSERT(NULL != (pSoap = initOnvifSoap(SOAP_SOCK_TIMEOUT)));

//	memset(&devinfo_req, 0x00, sizeof(devinfo_req));
//	memset(&devinfo_resp, 0x00, sizeof(devinfo_resp));
	setOnvifAuthInfo(pSoap, USERNAME, PASSWORD);
	result = soap_call___tds__GetDeviceInformation(pSoap, szAddrs, NULL, &objDevinfoReq, objDevinfoResp);

	if (SOAP_OK == result)
	{
		if (SOAP_OK != pSoap->error)
		{
			printSoapError(pSoap, "GetDeviceInformation");
		}
		else
		{
			// 此处增加成功后处理
		}
	}
	else
	{
		printSoapError(pSoap, "GetDeviceInformation");
	}

	if (NULL != pSoap)
	{
		deleteOnvifSoap(pSoap);
	}

	return result;
}

int OnvifDetectDevice::setOnvifAuthInfo(soap* pSoap, const char* szUsername, const char* szPassword)
{
	int result = 0;

	SOAP_ASSERT(NULL != szUsername);
	SOAP_ASSERT(NULL != szPassword);

 	result = soap_wsse_add_UsernameTokenDigest(pSoap, NULL, szUsername, szPassword);

	if (SOAP_OK == result)
	{
		if (SOAP_OK != pSoap->error)
		{
			printSoapError(pSoap, "UsernameTokenDigest");
		}
	}
	else
	{
		printSoapError(pSoap, "UsernameTokenDigest");
	}

	return result;
}

int OnvifDetectDevice::getOnvifCapabilities(const char* szAddrs, QString& qsMediaXAddr)
{
	int result = 0;
	soap* pSoap = NULL;
	_tds__GetCapabilities           objCapabilitiesReq;
	_tds__GetCapabilitiesResponse   objCapabilitiesResp;

	SOAP_ASSERT(NULL != szAddrs);
	SOAP_ASSERT(NULL != (pSoap = initOnvifSoap(SOAP_SOCK_TIMEOUT)));

	setOnvifAuthInfo(pSoap, USERNAME, PASSWORD);
	result = soap_call___tds__GetCapabilities(pSoap, szAddrs, NULL, &objCapabilitiesReq, objCapabilitiesResp);

	if (SOAP_OK == result)
	{
		if (SOAP_OK != pSoap->error)
		{
			printSoapError(pSoap, "GetCapabilities");
		}
		else
		{
			// 此处增加成功后处理
			qsMediaXAddr = QString::fromUtf8(objCapabilitiesResp.Capabilities->Media->XAddr);
		}
	}
	else
	{
		printSoapError(pSoap, "GetCapabilities");
	}

	if (NULL != pSoap)
	{
		deleteOnvifSoap(pSoap);
	}

	return result;
}

int OnvifDetectDevice::getOnvifProfiles(const QString& qsMediaXAddr, vector<CProfile>& vecProfile)
{
	int i = 0;
	int result = 0;
	soap* pSoap = NULL;
	_trt__GetProfiles            objProfiles;
	_trt__GetProfilesResponse    objProfilesResponse;

	SOAP_ASSERT(NULL != (pSoap = initOnvifSoap(SOAP_SOCK_TIMEOUT)));

	setOnvifAuthInfo(pSoap, USERNAME, PASSWORD);

	result = soap_call___trt__GetProfiles(pSoap, qsMediaXAddr.toUtf8().data(), NULL, &objProfiles, objProfilesResponse);

	// 分配缓存
	if (objProfilesResponse.__sizeProfiles > 0)
	{
		vecProfile.resize(objProfilesResponse.__sizeProfiles);
	}

	// 提取所有配置文件信息（我们所关心的）
	for(i = 0; i < objProfilesResponse.__sizeProfiles; i++)
	{
		tt__Profile* ttProfile = objProfilesResponse.Profiles[i];
		vector<CProfile>::iterator it = vecProfile.begin() + i;

		// 配置文件Token
		if (NULL != ttProfile->token)
		{
			it->m_qsToken = QString::fromUtf8(ttProfile->token);
		}

		// 视频编码器配置信息
		if (NULL != ttProfile->VideoEncoderConfiguration)
		{
			// 视频编码器Token
			if (NULL != ttProfile->VideoEncoderConfiguration->token)
			{
				it->m_objVideoEncoderConfiguration.m_qsToken = QString::fromUtf8(ttProfile->VideoEncoderConfiguration->token);
			}
			// 视频编码器分辨率
			if (NULL != ttProfile->VideoEncoderConfiguration->Resolution)
			{
				it->m_objVideoEncoderConfiguration.m_nWidth  = ttProfile->VideoEncoderConfiguration->Resolution->Width;
				it->m_objVideoEncoderConfiguration.m_Height = ttProfile->VideoEncoderConfiguration->Resolution->Height;
			}
		}
	}

	if (NULL != pSoap)
	{
		deleteOnvifSoap(pSoap);
	}

	return objProfilesResponse.__sizeProfiles;
}

int OnvifDetectDevice::getOnvifStreamUri(const QString& qsMediaXAddr,const QString& qsToken, QString& qsUri)
{
	int result = 0;
    soap* pSoap = NULL;
    tt__StreamSetup              objStreamSetup;
    tt__Transport                objTransport;
    _trt__GetStreamUri           objGetStreamUri;
    _trt__GetStreamUriResponse   objGetStreamUriResponse;

	qsUri = "";
 
    SOAP_ASSERT(NULL != (pSoap = initOnvifSoap(SOAP_SOCK_TIMEOUT)));
 
    objStreamSetup.Stream                = tt__StreamType__RTP_Unicast;
    objStreamSetup.Transport             = &objTransport;
    objStreamSetup.Transport->Protocol   = tt__TransportProtocol__RTSP;
    objStreamSetup.Transport->Tunnel     = NULL;
    objGetStreamUri.StreamSetup                     = &objStreamSetup;
	char ProfileToken[ONVIF_ADDRESS_SIZE] = {0};
	strncpy(ProfileToken, qsToken.toUtf8().data(), qsToken.toUtf8().size());
    objGetStreamUri.ProfileToken                    = ProfileToken;
 
    setOnvifAuthInfo(pSoap, USERNAME, PASSWORD);
    result = soap_call___trt__GetStreamUri(pSoap, qsMediaXAddr.toUtf8().data(), NULL, &objGetStreamUri, objGetStreamUriResponse);

	if (SOAP_OK == result)
	{
		if (SOAP_OK != pSoap->error)
		{
			printSoapError(pSoap, "GetStreamUri");
		}
		else
		{
			// 此处增加成功后处理
			result = -1;
			if (NULL != objGetStreamUriResponse.MediaUri)
			{
				if (NULL != objGetStreamUriResponse.MediaUri->Uri)
				{
					qsUri = QString::fromUtf8(objGetStreamUriResponse.MediaUri->Uri);
					result = SOAP_OK;
				}
			}
		}
	}
	else
	{
		printSoapError(pSoap, "GetStreamUri");
	}
 
    if (NULL != pSoap)
	{
        deleteOnvifSoap(pSoap);
    }
 
    return result;
}

int OnvifDetectDevice::makeUriWithauth(const QString& qsUri, const char* szUsername, const char * szPassword, QString& qsUriAuth)
{
	// 生成新的uri地址
	if (0 == strlen(szUsername) && 0 == strlen(szPassword))
	{
		qsUriAuth = qsUri;
	}
	else
	{
		int nIndex = qsUri.indexOf("//");
		if (-1 == nIndex)
		{
			printMsg(QString("can't found '//', src uri is: " + qsUri));
			return -1;
		}
		nIndex += 2;
		qsUriAuth = qsUri.left(nIndex) + szUsername + ":" + szPassword + "@" + qsUri.right(qsUri.length() - nIndex);
	}

	return 0;
}

void OnvifDetectDevice::openRtsp(const QString& qsUri)
{
	unsigned int    i;
	int             ret;
	int             video_st_index = -1;
	int             audio_st_index = -1;
	AVFormatContext *ifmt_ctx = NULL;
	AVPacket        pkt;
	AVStream        *st = NULL;
	char            errbuf[64];
	QString qsMsg;

	av_register_all();                                                          // Register all codecs and formats so that they can be used.
	avformat_network_init();                                                    // Initialization of network components

	// Open the input file for reading.
	if ((ret = avformat_open_input(&ifmt_ctx, qsUri.toUtf8().data(), 0, NULL)) < 0)
	{
		qsMsg.sprintf("Could not open input file '%s' (error '%s')", qsUri.toUtf8().data(), av_make_error_string(errbuf, sizeof(errbuf), ret));
		printMsg(qsMsg);
		if (NULL != ifmt_ctx)
		{
			avformat_close_input(&ifmt_ctx);
			ifmt_ctx = NULL;
		}
		return;
	}

	// Get information on the input file (number of streams etc.).
	if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0)
	{
		qsMsg.sprintf("Could not open find stream info (error '%s')", av_make_error_string(errbuf, sizeof(errbuf), ret));
		printMsg(qsMsg);
		if (NULL != ifmt_ctx)
		{
			avformat_close_input(&ifmt_ctx);
			ifmt_ctx = NULL;
		}
		return;
	}

	// dump information
	for (i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		av_dump_format(ifmt_ctx, i, qsUri.toUtf8().data(), 0);
	}

	// find video stream index
	for (i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		st = ifmt_ctx->streams[i];
		switch(st->codec->codec_type)
		{
		case AVMEDIA_TYPE_AUDIO: audio_st_index = i; break;
		case AVMEDIA_TYPE_VIDEO: video_st_index = i; break;
		default: break;
		}
	}
	if (-1 == video_st_index)
	{
		qsMsg.sprintf("No H.264 video stream in the input file");
		printMsg(qsMsg);
		if (NULL != ifmt_ctx)
		{
			avformat_close_input(&ifmt_ctx);
			ifmt_ctx = NULL;
		}
		return;
	}

	av_init_packet(&pkt);                                                       // initialize packet.
	pkt.data = NULL;
	pkt.size = 0;

	while (1)
	{
		do
		{
			ret = av_read_frame(ifmt_ctx, &pkt);                                // read frames
		} while (ret == AVERROR(EAGAIN));

		if (ret < 0)
		{
			qsMsg.sprintf("Could not read frame (error '%s')", av_make_error_string(errbuf, sizeof(errbuf), ret));
			printMsg(qsMsg);
			break;
		}

		if (pkt.stream_index == video_st_index)
		{                               // video frame
			qsMsg.sprintf("Video Packet size = %d", pkt.size);
			printMsg(qsMsg);
		}
		else if(pkt.stream_index == audio_st_index)
		{                         // audio frame
			qsMsg.sprintf("Audio Packet size = %d", pkt.size);
			printMsg(qsMsg);
		}
		else
		{
			qsMsg.sprintf("Unknow Packet size = %d", pkt.size);
			printMsg(qsMsg);
		}

		av_packet_unref(&pkt);
		Sleep(3000);
	}

	if (NULL != ifmt_ctx)
	{
		avformat_close_input(&ifmt_ctx);
		ifmt_ctx = NULL;
	}
}

CProfile::CProfile()
{
	m_qsToken = "";
}

CProfile::~CProfile()
{

}

CVideoEncoderConfiguration::CVideoEncoderConfiguration()
{
	m_qsToken = "";
	m_nWidth = 0;
	m_Height = 0;
}

CVideoEncoderConfiguration::~CVideoEncoderConfiguration()
{

}
