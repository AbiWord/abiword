/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_bytebuf.h"

#include "pd_Document.h"

#include "ie_types.h"
#include "ie_imp_MsWord_97.h"

#define X_ReturnIfFail(exp,ies)     do { UT_Bool b = (exp); if (!b) return (ies); } while (0)
#define X_ReturnNoMemIfError(exp)   X_ReturnIfFail(exp,IES_NoMemory)

#define X_CheckError(v)         do {  if (!(v))                             \
                                      {  m_iestatus = IES_Error;            \
                                         return; } } while (0)


int CharProc(wvParseStruct *ps,U16 eachchar,U8 chartype);
int ElementProc(wvParseStruct *ps,wvTag tag, void *props);
int DocProc(wvParseStruct *ps,wvTag tag);



/*****************************************************************/

IEStatus IE_Imp_MsWord_97::importFile(const char * szFilename)
{
	FILE *fp = NULL;
	char buf[4096];
	UT_DEBUGMSG(("got to import file\n"));

	fp = fopen(szFilename, "rb");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		m_iestatus = IES_FileNotFound;
	}
	UT_DEBUGMSG(("wv importer\n"));

	wvParseStruct ps;
	if (wvInitParser(&ps,fp))
		{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		m_iestatus = IES_BogusDocument;
		return m_iestatus;
		}
	
	UT_DEBUGMSG(("just here\n"));
	ps.userData = this;
	wvSetElementHandler(ElementProc);
	wvSetCharHandler(CharProc);
	wvSetDocumentHandler(DocProc);

	wvText(&ps);

	m_iestatus = IES_OK;

	return m_iestatus;
}

int CharProc(wvParseStruct *ps,U16 eachchar,U8 chartype)
	{
	IE_Imp_MsWord_97* pDocReader = (IE_Imp_MsWord_97 *) ps->userData;
	UT_UCSChar *pbuf=&eachchar;
	UT_DEBUGMSG(("word 97 char is %c, type is %d\n",eachchar,chartype));
	return(pDocReader->_charData(&eachchar, 1));
	}

int DocProc(wvParseStruct *ps,wvTag tag)
	{
	IE_Imp_MsWord_97* pDocReader = (IE_Imp_MsWord_97 *) ps->userData;
	return(pDocReader->_docProc(ps, tag));
	}

int ElementProc(wvParseStruct *ps,wvTag tag,void *props)
	{
	IE_Imp_MsWord_97* pDocReader = (IE_Imp_MsWord_97 *) ps->userData;
	return(pDocReader->_eleProc(ps, tag, props));
	UT_DEBUGMSG(("ele begins\n"));
	return(0);
	}

int IE_Imp_MsWord_97::_charData(U16 *charstr, int len)
	{
	UT_UCSChar buf[1];
	buf[0] = charstr[0];
	X_CheckError(m_pDocument->appendSpan(buf,1));
	return(0);
	}

int IE_Imp_MsWord_97::_docProc(wvParseStruct *ps,wvTag tag)
	{
	switch(tag)
		{
		case DOCBEGIN:
			X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Section, NULL));
			break;
		case DOCEND:	
			/*abiword doesn't need this*/
			break;
		}
	return(0);
	}

int IE_Imp_MsWord_97::_eleProc(wvParseStruct *ps,wvTag tag, void *props)
	{
	XML_Char propBuffer[1024];
	XML_Char* pProps = "PROPS";
	const XML_Char* propsArray[3];

	propBuffer[0] = 0;
	UT_DEBUGMSG((" started\n"));
	PAP *apap;
	CHP *achp;
	   
	switch(tag)
		{
		case PARABEGIN:
                        apap = (PAP*)props;
		        strcat(propBuffer, "text-align:");
			switch(apap->jc)
				{
				case 0:
					strcat(propBuffer, "left");
					break;
				case 1:
					strcat(propBuffer, "center");
					break;
				case 2:
					strcat(propBuffer, "right");
					break;
				case 3:
					strcat(propBuffer, "justify");
					break;
				case 4:			
				/* this type of justification is of unknown purpose and is 
				undocumented , but it shows up in asian documents so someone
				should be able to tell me what it is someday C. */
					strcat(propBuffer, "justify");
					break;
				}
			//strcat(propBuffer, "; ");
			propsArray[0] = pProps;
			propsArray[1] = propBuffer;
			propsArray[2] = NULL;
			UT_DEBUGMSG(("the propBuffer is %s\n",propBuffer));
			X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block, propsArray));
			/*
			X_ReturnNoMemIfError(m_pDocument->appendStrux(PTX_Block,NULL));
			*/
			break;
		case CHARPROPBEGIN:
		   achp = (CHP*)props;
		   if (achp->fBold) { 
		      strcat(propBuffer, "font-weight:bold;");
		   } else {
		      strcat(propBuffer, "font-weight:normal;");
		   }
		   if (achp->fItalic) {
		      strcat(propBuffer, "font-style:italic;");
		   } else {
		      strcat(propBuffer, "font-style:normal;");
		   }
		   strcat(propBuffer, "text-decoration:");
		   if (achp->fStrike && achp->kul) {
		      strcat(propBuffer, "underline line-through;");
		   } else if (achp->fStrike) {
		      strcat(propBuffer, "line-through;");
		   } else if (achp->kul) {
		      strcat(propBuffer, "underline;");
		   } else {
		      strcat(propBuffer, "normal;");
		   }
		   propsArray[0] = pProps;
		   // remove trailing ;
		   propBuffer[strlen(propBuffer)-1] = 0;
		   propsArray[1] = propBuffer;
		   propsArray[2] = NULL;
		   UT_DEBUGMSG(("the propBuffer is %s\n",propBuffer));
		   X_ReturnNoMemIfError(m_pDocument->appendFmt(propsArray));
		   break;
		case PARAEND:	/* not needed */
		case CHARPROPEND: /* not needed */
		default:
		   break;
		}
	UT_DEBUGMSG(("ended\n"));
	return(0);
	}


/*****************************************************************/

IE_Imp_MsWord_97::~IE_Imp_MsWord_97()
{
}

IE_Imp_MsWord_97::IE_Imp_MsWord_97(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
	UT_DEBUGMSG(("constructed wv\n"));
	m_iestatus = IES_OK;
}

/*****************************************************************/
/*****************************************************************/

UT_Bool IE_Imp_MsWord_97::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".doc") == 0);
}

IEStatus IE_Imp_MsWord_97::StaticConstructor(PD_Document * pDocument,
											 IE_Imp ** ppie)
{
	IE_Imp_MsWord_97 * p = new IE_Imp_MsWord_97(pDocument);
	*ppie = p;
	return IES_OK;
}

UT_Bool	IE_Imp_MsWord_97::GetDlgLabels(const char ** pszDesc,
									   const char ** pszSuffixList,
									   IEFileType * ft)
{
	*pszDesc = "Microsoft Word (wv) (.doc)";
	*pszSuffixList = "*.doc";
	*ft = IEFT_MsWord_97;
	return UT_TRUE;
}

UT_Bool IE_Imp_MsWord_97::SupportsFileType(IEFileType ft)
{
	return (IEFT_MsWord_97 == ft);
}
