
#include <stdio.h>
#include <malloc.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_AbiWord_1.h"
#include "pd_Document.h"

/*****************************************************************
******************************************************************
** C-style callback functions that we register with the XML parser
******************************************************************
*****************************************************************/

static void startElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	IE_Imp_AbiWord_1* pDocReader = (IE_Imp_AbiWord_1*) userData;
	pDocReader->_startElement(name, atts);
}

static void endElement(void *userData, const XML_Char *name)
{
	IE_Imp_AbiWord_1* pDocReader = (IE_Imp_AbiWord_1*) userData;
	pDocReader->_endElement(name);
}

static void charData(void* userData, const XML_Char *s, int len)
{
	IE_Imp_AbiWord_1* pDocReader = (IE_Imp_AbiWord_1*) userData;
	pDocReader->_charData(s, len);
}

/*****************************************************************/
/*****************************************************************/

IE_Imp::IEStatus IE_Imp_AbiWord_1::importFile(const char * szFilename)
{
	XML_Parser parser = NULL;
	FILE *fp = NULL;
	int done = 0;
	char buf[4096];

	fp = fopen(szFilename, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		m_iestatus = IE_Imp::IES_FileNotFound;
		goto Cleanup;
	}
	
	parser = XML_ParserCreate(NULL);
	XML_SetUserData(parser, this);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, charData);

	while (!done)
	{
		size_t len = fread(buf, 1, sizeof(buf), fp);
		done = (len < sizeof(buf));

		if (!XML_Parse(parser, buf, len, done)) 
		{
			UT_DEBUGMSG(("%s at line %d\n",
						 XML_ErrorString(XML_GetErrorCode(parser)),
						 XML_GetCurrentLineNumber(parser)));
			m_iestatus = IE_Imp::IES_BogusDocument;
			goto Cleanup;
		}

		if (m_iestatus != IE_Imp::IES_OK)
		{
			UT_DEBUGMSG(("Problem reading document\n"));
			goto Cleanup;
		}
	} 
	
	m_iestatus = IE_Imp::IES_OK;

Cleanup:
	if (parser)
		XML_ParserFree(parser);
	if (fp)
		fclose(fp);
	return m_iestatus;
}

IE_Imp_AbiWord_1::~IE_Imp_AbiWord_1()
{
}

IE_Imp_AbiWord_1::IE_Imp_AbiWord_1(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
	m_iestatus = IE_Imp::IES_OK;
	m_parseState = _PS_Init;
}

/*****************************************************************/
/*****************************************************************/

#define TT_OTHER		0
#define TT_DOCUMENT		1
#define TT_SECTION		2
#define TT_COLUMNSET	3
#define TT_COLUMN		4
#define TT_BLOCK		5
#define TT_INLINE		6

struct _TokenTable
{
	const char *	m_name;
	int				m_type;
};

static struct _TokenTable s_Tokens[] =
{	{	"awml",			TT_DOCUMENT		},
	{	"section",		TT_SECTION		},
	{	"columnmodel",	TT_COLUMNSET	},
	{	"column",		TT_COLUMN		},
	{	"p",			TT_BLOCK		},
	{	"c",			TT_INLINE		},
	{	"*",			TT_OTHER		}};	// must be last

#define TokenTableSize	((sizeof(s_Tokens)/sizeof(s_Tokens[0])))

static UT_uint32 s_mapNameToToken(const XML_Char * name)
{
	for (unsigned int k=0; k<TokenTableSize; k++)
		if (s_Tokens[k].m_name[0] == '*')
			return k;
		else if (UT_stricmp(s_Tokens[k].m_name,name)==0)
			return k;
	UT_ASSERT(0);
	return 0;
}

/*****************************************************************/	
/*****************************************************************/	

#define X_TestParseState(ps)	((m_parseState==(ps)))

#define X_VerifyParseState(ps)	do {  if (!(X_TestParseState(ps)))					\
									  {  m_iestatus = IE_Imp::IES_BogusDocument;	\
										 return; } } while (0)

#define X_CheckDocument(b)		do {  if (!(b))										\
									  {  m_iestatus = IE_Imp::IES_BogusDocument;	\
										 return; } } while (0)

#define X_CheckError(v)			do {  if (!(v))										\
									  {  m_iestatus = IE_Imp::IES_Error;			\
										 return; } } while (0)

#define	X_EatIfAlreadyError()	do {  if (m_iestatus != IE_Imp::IES_OK) return; } while (0)

/*****************************************************************/
/*****************************************************************/

void IE_Imp_AbiWord_1::_startElement(const XML_Char *name, const XML_Char **atts)
{
	UT_DEBUGMSG(("startElement: %s\n", name));

	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	
	UT_uint32 tokenIndex = s_mapNameToToken(name);
	switch (s_Tokens[tokenIndex].m_type)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Init);
		m_parseState = _PS_Doc;
		return;

	case TT_SECTION:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Sec;
		X_CheckError(m_pDocument->appendStrux(PTX_Section,atts));
		return;

	case TT_COLUMNSET:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_ColSet;
		X_CheckError(m_pDocument->appendStrux(PTX_ColumnSet,atts));
		return;
		
	case TT_COLUMN:
		X_VerifyParseState(_PS_ColSet);
		m_parseState = _PS_Col;
		X_CheckError(m_pDocument->appendStrux(PTX_Column,atts));
		return;
		
	case TT_BLOCK:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Block;
		X_CheckError(m_pDocument->appendStrux(PTX_Block,atts));
		return;
		
	case TT_INLINE:
		X_VerifyParseState(_PS_Block);
		X_CheckError(_pushInlineFmt(atts));
		X_CheckError(m_pDocument->appendFmt(&m_vecInlineFmt));
		return;
		
	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown tag [%s]\n",name));
		m_iestatus = IE_Imp::IES_BogusDocument;
		return;
	}
}

void IE_Imp_AbiWord_1::_endElement(const XML_Char *name)
{
	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	
	UT_uint32 tokenIndex = s_mapNameToToken(name);

	switch (s_Tokens[tokenIndex].m_type)
	{
	case TT_DOCUMENT:
		X_VerifyParseState(_PS_Doc);
		m_parseState = _PS_Init;
		return;

	case TT_SECTION:
		X_VerifyParseState(_PS_Sec);
		m_parseState = _PS_Doc;
		return;

	case TT_COLUMNSET:
		X_VerifyParseState(_PS_ColSet);
		m_parseState = _PS_Sec;
		return;
		
	case TT_COLUMN:
		X_VerifyParseState(_PS_Col);
		m_parseState = _PS_ColSet;
		return;
		
	case TT_BLOCK:
		X_VerifyParseState(_PS_Block);
		m_parseState = _PS_Sec;
		X_CheckDocument(_getInlineDepth()==0);
		return;
		
	case TT_INLINE:
		X_VerifyParseState(_PS_Block);
		X_CheckDocument(_getInlineDepth()>0);
		_popInlineFmt();
		return;
		
	case TT_OTHER:
	default:
		UT_DEBUGMSG(("Unknown end tag [%s]\n",name));
		m_iestatus = IE_Imp::IES_BogusDocument;
		return;
	}
}

void IE_Imp_AbiWord_1::_charData(const XML_Char *s, int len)
{
	X_EatIfAlreadyError();				// xml parser keeps running until buffer consumed
	
	if (!X_TestParseState(_PS_Block))
	{
		UT_DEBUGMSG(("charData DISCARDED [length %d]\n",len));
		return;
	}
	
	// TODO fix this to use proper methods to convert XML_Char to UT_UCSChar

	UT_ASSERT(sizeof(XML_Char) != sizeof(UT_UCSChar));
	{
		UT_UCSChar * xx = new UT_UCSChar[len];
		int iConverted = 0;
		/*
		  This code is intended to convert a single EOL to a single space.
		  */
		for (int k=0; k<len; k++)
		{
			if (13 == s[k])			// CR
			{
				xx[iConverted++] = 32;	// space
				if (10 == s[k+1])
				{
					k++;			// skip the LF
				}
			}
			else if (10 == s[k])
			{
				xx[iConverted++] = 32;	// space
			}
			else
			{
				xx[iConverted++] = s[k];
			}
		}

		UT_Bool bResult = m_pDocument->appendSpan(xx,iConverted);
		delete xx;
		X_CheckError(bResult);
	}
	return;
}

/*****************************************************************/
/*****************************************************************/

UT_uint32 IE_Imp_AbiWord_1::_getInlineDepth(void) const
{
	return m_stackFmtStartIndex.getDepth();
}

UT_Bool IE_Imp_AbiWord_1::_pushInlineFmt(const XML_Char ** atts)
{
	UT_uint32 start = m_vecInlineFmt.getItemCount();
	UT_uint32 k;

	for (k=0; (atts[k]); k++)
	{
		XML_Char * p;
		if (!UT_XML_cloneString(p,atts[k]))
			return UT_FALSE;
		if (m_vecInlineFmt.addItem(p)!=0)
			return UT_FALSE;
	}
	if (!m_stackFmtStartIndex.push((void*)start))
		return UT_FALSE;
	return UT_TRUE;
}

void IE_Imp_AbiWord_1::_popInlineFmt(void)
{
	UT_uint32 start;
	if (!m_stackFmtStartIndex.pop((void **)&start))
		return;
	UT_uint32 k;
	UT_uint32 end = m_vecInlineFmt.getItemCount();
	for (k=end; k>=start; k--)
	{
		const XML_Char * p = (const XML_Char *)m_vecInlineFmt.getNthItem(k-1);
		m_vecInlineFmt.deleteNthItem(k-1);
		if (p)
			free((void *)p);
	}
}

