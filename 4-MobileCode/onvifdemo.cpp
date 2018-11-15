#include "onvifdemo.h"
#include "MobileCodeWSSoap.nsmap"
#include "soapStub.h"

OnvifDemo::OnvifDemo(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
}

OnvifDemo::~OnvifDemo()
{

}

void OnvifDemo::lineEditSlotTextChanged(QString qsText)
{
	if (qsText.length() != 11)
	{
		ui.label->setText(QString("国内手机号码归属地查询"));
		return;
	}
	struct soap *soap = NULL;
	const char  *endpoint = "http://ws.webxml.com.cn/WebServices/MobileCodeWS.asmx";
	struct _ns1__getMobileCodeInfo          req;
	struct _ns1__getMobileCodeInfoResponse  resp;

	soap = soap_new();                                                          // allocate and initalize a context

	soap_set_mode(soap, SOAP_C_UTFSTRING);                                      // support multibyte string(for Chinese)

	// 此处将req的虚函数表干掉了，导致后续崩溃
	// memset(&req, 0x00, sizeof(req));

	char szCode[64];
	strcpy(szCode, qsText.toUtf8().data());
	req.mobileCode = szCode;
	req.userID     = NULL;

	if(SOAP_OK == soap_call___ns1__getMobileCodeInfo(soap, endpoint, NULL, &req, resp))
	{
		if (NULL != resp.getMobileCodeInfoResult)
		{
			ui.label->setText(QString::fromUtf8(resp.getMobileCodeInfoResult));
		}
		else
		{
			ui.label->setText(QString("Null"));
		}
	}
	else
	{
		ui.label->setText("Error");
	}

	soap_destroy(soap);                                                         // delete deserialized objects
	soap_end(soap);                                                             // delete allocated data
	soap_free(soap);                                                            // free the soap struct context data
}
