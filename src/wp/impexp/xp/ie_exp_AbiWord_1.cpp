
#include "ie_exp_AbiWord_1.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Span.h"
#include "px_ChangeRecord_Strux.h"

/*****************************************************************/
/*****************************************************************/

IE_Exp_AbiWord_1::IE_Exp_AbiWord_1(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
	m_lid = 0;
}

IE_Exp_AbiWord_1::~IE_Exp_AbiWord_1()
{
}

/*****************************************************************/
/*****************************************************************/

IEStatus IE_Exp_AbiWord_1::writeFile(const char * szFilename)
{
	UT_ASSERT(m_pDocument);
	UT_ASSERT(szFilename && *szFilename);

	if (!_openFile(szFilename))
		return IES_CouldNotOpenForWriting;

	IEStatus status = _writeDocument();
	if (status == IES_OK)
		_closeFile();
	else
		_abortFile();

	// Note: we let our caller worry about resetting the dirty bit
	// Note: on the document and possibly updating the filename.
	
	return status;
}

void IE_Exp_AbiWord_1::write(const char * sz)
{
	if (m_error)
		return;
	m_error |= ! _writeBytes((UT_Byte *)sz);
	return;
}

void IE_Exp_AbiWord_1::write(const char * sz, UT_uint32 length)
{
	if (m_error)
		return;
	if (_writeBytes((UT_Byte *)sz,length) != length)
		m_error = UT_TRUE;
	
	return;
}

/*****************************************************************/
/*****************************************************************/

class ie_Exp_Listener : public PL_Listener
{
public:
	ie_Exp_Listener(PD_Document * pDocument,
					IE_Exp_AbiWord_1 * pie);
	~ie_Exp_Listener();

	virtual UT_Bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual UT_Bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual UT_Bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual UT_Bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_StruxFmtHandle * psfh);

protected:
	void				_closeSection(void);
	void				_closeColumnSet(void);
	void				_closeBlock(void);
	void				_closeSpan(void);
	void				_openTag(const char * szPrefix, const char * szSuffix,
								 UT_Bool bNewLineAfter, PT_AttrPropIndex api);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	
	PD_Document *		m_pDocument;
	IE_Exp_AbiWord_1 *	m_pie;
	UT_Bool				m_bInSection;
	UT_Bool				m_bInColumnSet;
	UT_Bool				m_bInBlock;
	UT_Bool				m_bInSpan;
};

void ie_Exp_Listener::_closeSection(void)
{
	if (!m_bInSection)
		return;
	
	m_pie->write("</section>\n");
	m_bInSection = UT_FALSE;
	return;
}

void ie_Exp_Listener::_closeColumnSet(void)
{
	if (!m_bInColumnSet)
		return;

	m_pie->write("</columnmodel>\n");
	m_bInColumnSet = UT_FALSE;
	return;
}

void ie_Exp_Listener::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	m_pie->write("</p>\n");
	m_bInBlock = UT_FALSE;
	return;
}

void ie_Exp_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	m_pie->write("</c>");
	m_bInSpan = UT_FALSE;
	return;
}

void ie_Exp_Listener::_openTag(const char * szPrefix, const char * szSuffix,
							   UT_Bool bNewLineAfter, PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	UT_Bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	m_pie->write("<");
	UT_ASSERT(szPrefix && *szPrefix);
	m_pie->write(szPrefix);
	if (bHaveProp && pAP)
	{
		const XML_Char * szName;
		const XML_Char * szValue;
		UT_uint32 k = 0;

		while (pAP->getNthAttribute(k++,szName,szValue))
		{
			// TODO we force double-quotes on all values.
			// TODO consider scanning the value to see if it has one
			// TODO in it and escaping it or using single-quotes.
			
			m_pie->write(" ");
			m_pie->write(szName);
			m_pie->write("=\"");
			m_pie->write(szValue);
			m_pie->write("\"");
		}
		if (pAP->getNthProperty(0,szName,szValue))
		{
			m_pie->write(" style=\"");
			m_pie->write(szName);
			m_pie->write(":");
			m_pie->write(szValue);
			UT_uint32 j = 1;
			while (pAP->getNthProperty(j++,szName,szValue))
			{
				m_pie->write("; ");
				m_pie->write(szName);
				m_pie->write(":");
				m_pie->write(szValue);
			}
			m_pie->write("\"");
		}
	}

	if (szSuffix && *szSuffix)
		m_pie->write(szSuffix);
	m_pie->write(">");
	if (bNewLineAfter)
		m_pie->write("\n");
}

void ie_Exp_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	// TODO deal with unicode.
	// TODO for now, just squish it into ascii.
	
#define MY_BUFFER_SIZE		1024
	char buf[MY_BUFFER_SIZE];
	char * pBuf;
	const UT_UCSChar * pData;

	for (pBuf=buf, pData=data; (pData<data+length); /**/)
	{
		if (pBuf == buf+MY_BUFFER_SIZE)
		{
			m_pie->write(buf,MY_BUFFER_SIZE);
			pBuf = buf;
		}

		UT_ASSERT(*pData < 256);
		*pBuf++ = (UT_Byte)*pData++;
	}

	if (pBuf > buf)
		m_pie->write(buf,(pBuf-buf));
}

ie_Exp_Listener::ie_Exp_Listener(PD_Document * pDocument,
								 IE_Exp_AbiWord_1 * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSection = UT_FALSE;
	m_bInColumnSet = UT_FALSE;
	m_bInBlock = UT_FALSE;
	m_bInSpan = UT_FALSE;
	
	m_pie->write("<awml>\n");
}

ie_Exp_Listener::~ie_Exp_Listener()
{
	_closeSpan();
	_closeBlock();
	_closeColumnSet();
	_closeSection();

	m_pie->write("</awml>\n");
}

UT_Bool ie_Exp_Listener::populate(PL_StruxFmtHandle sfh,
								  const PX_ChangeRecord * pcr)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertSpan);
	const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

	PT_AttrPropIndex api = pcr->getIndexAP();
	if (api)
	{
		_openTag("c","",UT_FALSE,api);
		m_bInSpan = UT_TRUE;
	}

	PT_BufIndex bi = pcrs->getBufIndex();
	_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

	if (api)
		_closeSpan();
	return UT_TRUE;
}

UT_Bool ie_Exp_Listener::populateStrux(PL_StruxDocHandle sdh,
									   const PX_ChangeRecord * pcr,
									   PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		{
			_closeSpan();
			_closeBlock();
			_closeColumnSet();
			_closeSection();
			_openTag("section","",UT_TRUE,pcr->getIndexAP());
			m_bInSection = UT_TRUE;
			return UT_TRUE;
		}

	case PTX_ColumnSet:
		{
			_openTag("columnmodel","",UT_TRUE,pcr->getIndexAP());
			m_bInColumnSet = UT_TRUE;
			return UT_TRUE;
		}

	case PTX_Column:
		{
			_openTag("column","/",UT_TRUE,pcr->getIndexAP());
			return UT_TRUE;
		}

	case PTX_Block:
		{
			_closeColumnSet();
			_closeSpan();
			_closeBlock();
			_openTag("p","",UT_FALSE,pcr->getIndexAP());
			m_bInBlock = UT_TRUE;
			return UT_TRUE;
		}

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool ie_Exp_Listener::change(PL_StruxFmtHandle sfh,
								const PX_ChangeRecord * pcr)
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}

UT_Bool ie_Exp_Listener::insertStrux(PL_StruxFmtHandle sfh,
									 const PX_ChangeRecord * pcr,
									 PL_StruxDocHandle sdh,
									 PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(0);						// this function is not used.
	return UT_FALSE;
}


/*****************************************************************/
/*****************************************************************/

IEStatus IE_Exp_AbiWord_1::_writeDocument(void)
{
	m_pListener = new ie_Exp_Listener(m_pDocument,this);
	if (!m_pListener)
		return IES_NoMemory;
	if (!m_pDocument->addListener(static_cast<PL_Listener *>(m_pListener),&m_lid))
		return IES_Error;
	m_pDocument->removeListener(m_lid);
	delete m_pListener;

	m_lid = 0;
	m_pListener = NULL;
	
	return ((m_error) ? IES_CouldNotWriteToFile : IES_OK);
}

