#ifndef XAP_LOG
#define XAP_LOG

#include <stdio.h>

class UT_String;
class AV_View;
class EV_EditMethodCallData;

class XAP_Log
{
public:
	void log(const UT_String &method_name, AV_View *pAV_View, EV_EditMethodCallData *pCallData);
	static XAP_Log *get_instance();
	virtual ~XAP_Log();

private:
	XAP_Log(const UT_String &logfile);
	FILE *m_pOutput;
	static XAP_Log *m_pInstance;
};

#endif /* XAP_LOG */
