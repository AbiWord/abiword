#include "xap_Log.h"
#include <stdio.h>
#include "ut_string_class.h"
#include "ut_string.h"
#include "xav_View.h"
#include "ev_EditMethod.h"

XAP_Log *XAP_Log::m_pInstance = 0;

class XAP_LogDestructor
{
public:
	XAP_LogDestructor() : t_(0) {}
	~XAP_LogDestructor() { delete t_; }
	void control(XAP_Log *t) { t_ = t; }
private:
	XAP_Log *t_;
};

static XAP_LogDestructor g_pLogDestructor;

XAP_Log::XAP_Log(const UT_String &logfile)
{
	m_pOutput = fopen(logfile.c_str(), "w");
	fprintf(m_pOutput, "<?xml version=\"1.0\"?>\n");
	fprintf(m_pOutput, "<logger>\n");
}

XAP_Log::~XAP_Log()
{
	if (m_pOutput != 0)
	{
		fprintf(m_pOutput, "</logger>\n");
		fclose(m_pOutput);
	}
}

void XAP_Log::log(const UT_String &method_name, AV_View *pAV_View, EV_EditMethodCallData *pCallData)
{
	UT_ASSERT(m_pOutput != 0);
	fprintf(m_pOutput, "\t<event name=\"%s\"", method_name.c_str());

	if (pCallData != 0)
	{
		fprintf(m_pOutput, ">\n\t\t<calldata x=\"%d\" y=\"%d\"",
				pCallData->m_xPos,
				pCallData->m_yPos);
		
		// the pCallData->m_bAllocatedData flag is not set when it should
		if (pCallData->m_pData != 0)
		{
			fprintf(m_pOutput, ">");
			UT_UCSChar *orig_data;
			UT_UCSChar *data = orig_data = pCallData->m_pData;
			UT_uint32 data_length = pCallData->m_dataLength;
			UT_String stData(UT_encodeUTF8char(*data++));

			while (static_cast<size_t>(data - pCallData->m_pData) < data_length)
				stData += UT_encodeUTF8char(*data++);

			fprintf(m_pOutput, "%s</calldata>\n\t</event>\n", stData.c_str());
			
		}
		else
			fprintf(m_pOutput, "/>\n\t</event>\n");
			
	}
	else
		fprintf(m_pOutput, "/>\n");
/*
	UT_UCSChar *		m_pData;
	UT_uint32			m_dataLength;
	bool				m_bAllocatedData;
	UT_sint32			m_xPos;
	UT_sint32			m_yPos;
*/
}

XAP_Log *XAP_Log::get_instance()
{
	if (m_pInstance == 0)
	{
		m_pInstance = new XAP_Log("fixme_log.txt");
		g_pLogDestructor.control(m_pInstance);
	}

	return m_pInstance;
}

