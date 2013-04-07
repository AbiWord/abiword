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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

/******************************************************************
** This class is to be considered private to ie_exp_RTF.cpp
** This is a PL_Listener.  It's purpose is to gather information
** from the document (the font table and color table and anything
** else) that must be written to the rtf header.
******************************************************************/

#include "ut_string.h"
#include "ie_exp_RTF_listenerGetProps.h"
#include "ie_exp_RTF_AttrProp.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"

void s_RTF_ListenerGetProps::_closeSection(void)
{
	if (!m_bInSection)
		return;
	
	m_bInSection = false;
	m_apiThisSection = 0;

	return;
}

void s_RTF_ListenerGetProps::_closeBlock(void)
{
	if (!m_bInBlock)
		return;

	m_bInBlock = false;
	m_apiThisBlock = 0;
	
	return;
}

void s_RTF_ListenerGetProps::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	m_bInSpan = false;
	return;
}

void s_RTF_ListenerGetProps::_openSpan(PT_AttrPropIndex apiSpan)
{
	if (m_bInSpan)
	{
		if (m_apiLastSpan == apiSpan)
			return;
		_closeSpan();
	}

	const PP_AttrProp * pSpanAP = NULL;
	const PP_AttrProp * pBlockAP = NULL;
	const PP_AttrProp * pSectionAP = NULL;

	m_pDocument->getAttrProp(m_apiThisSection,&pSectionAP);
	m_pDocument->getAttrProp(m_apiThisBlock,&pBlockAP);
	m_pDocument->getAttrProp(apiSpan,&pSpanAP);

	_compute_span_properties(pSpanAP,pBlockAP,pSectionAP);
	
	m_bInSpan = true;
	m_apiLastSpan = apiSpan;
	return;
}

void s_RTF_ListenerGetProps::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	if (m_pie->m_bNeedUnicodeText)		/* we already have an answer */
		return;
	
	const UT_UCSChar * pData;
	for (pData=data; (pData<data+length); pData++)
	{
		if (*pData > 0x00ff)
		{
			m_pie->m_bNeedUnicodeText = true;
			return;
		}
	}

	return;
}

s_RTF_ListenerGetProps::s_RTF_ListenerGetProps(PD_Document * pDocument,
										 IE_Exp_RTF * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSection = false;
	m_bInBlock = false;
	m_bInSpan = false;
	m_apiLastSpan = 0;
	m_apiThisSection = 0;
	m_apiThisBlock = 0;
	m_bHasBlock = false;
}

s_RTF_ListenerGetProps::~s_RTF_ListenerGetProps()
{
	_closeSpan();
	_closeBlock();
	_closeSection();
}

bool s_RTF_ListenerGetProps::populate(fl_ContainerLayout* /*sfh*/,
										 const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();
			_openSpan(api);
			
			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
#if 0
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
			case PTO_Image:
				_closeSpan();
				_openTag("image",api);
				return true;

			case PTO_Field:
				_closeSpan();
				_openTag("field",api);
				return true;

			default:
				UT_ASSERT_NOT_REACHED();
				return false;
			}
#endif
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;
		
	default:
	  UT_ASSERT_NOT_REACHED();
		return false;
	}
}

bool s_RTF_ListenerGetProps::populateStrux(pf_Frag_Strux* /*sdh*/,
											  const PX_ChangeRecord * pcr,
											  fl_ContainerLayout* * psfh)
{
	UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_Section:
		{
			_closeSpan();
			_closeBlock();
			_closeSection();
			m_bInSection = true;
			m_apiThisSection = pcr->getIndexAP();
			return true;
		}

	case PTX_SectionHdrFtr:
		{
			_closeSpan();
			_closeBlock();
			_closeSection();
			m_bInSection = true;
			m_apiThisSection = pcr->getIndexAP();
			return true;
		}
	case PTX_SectionTable:
	    {
			_closeSpan();
			_searchTableAPI(pcr->getIndexAP());
			return true;
		}
	case PTX_SectionFootnote:
	    {
			_closeSpan();
			m_apiSavedBlock = m_apiThisBlock;
			return true;
		}
	case PTX_EndFootnote:
	    {
			_closeSpan();
			_closeBlock();
			m_apiThisBlock = m_apiSavedBlock;
			return true;
		}
	case PTX_SectionAnnotation:
	    {
			_closeSpan();
			m_apiSavedBlock = m_apiThisBlock;
			return true;
		}
	case PTX_EndAnnotation:
	    {
			_closeSpan();
			_closeBlock();
			m_apiThisBlock = m_apiSavedBlock;
			return true;
		}
	case PTX_SectionEndnote:
	    {
			_closeSpan();
			m_apiSavedBlock = m_apiThisBlock;
			return true;
		}
	case PTX_SectionTOC:
	    {
			_closeSpan();
			return true;
		}
	case PTX_EndTOC:
	    {
			_closeSpan();
			return true;
		}
	case PTX_SectionFrame:
	    {
			_closeSpan();
			return true;
		}
	case PTX_EndFrame:
	    {
			_closeSpan();
			return true;
		}
	case PTX_EndEndnote:
	    {
			_closeSpan();
			_closeBlock();
			m_apiThisBlock = m_apiSavedBlock;
			return true;
		}
	case PTX_SectionCell:
	    {
			_closeSpan();
			_searchCellAPI(pcr->getIndexAP());
			return true;
		}
	case PTX_EndTable:
	    {
			_closeSpan();
			return true;
		}
	case PTX_EndCell:
	    {
			_closeSpan();
			return true;
		}

	case PTX_Block:
		{
			_closeSpan();
			_closeBlock();
			m_bInBlock = true;
			m_bHasBlock = true;
			m_apiThisBlock = pcr->getIndexAP();

			// Find colours of the Paragraph borders and shading

			const PP_AttrProp * pBlockAP = NULL;
			m_pDocument->getAttrProp(m_apiThisBlock,&pBlockAP);
			const gchar * szColor = PP_evalProperty("bot-color",pBlockAP,NULL,NULL,m_pDocument,true);
			UT_sint32 ndxColor = 0;
			if (szColor)
			{
			    ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
			    if (ndxColor == -1)
			      m_pie->_addColor(static_cast<const char*>(szColor));
			}
			szColor = PP_evalProperty("left-color",pBlockAP,NULL,NULL,m_pDocument,true);
			if (szColor)
			{
			    ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
			    if (ndxColor == -1)
			      m_pie->_addColor(static_cast<const char*>(szColor));
			}

			szColor = PP_evalProperty("right-color",pBlockAP,NULL,NULL,m_pDocument,true);
			if (szColor)
			{
			    ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
			    if (ndxColor == -1)
			      m_pie->_addColor(static_cast<const char*>(szColor));
			}

			szColor = PP_evalProperty("top-color",pBlockAP,NULL,NULL,m_pDocument,true);
			if (szColor)
			{
			    ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
			    if (ndxColor == -1)
			      m_pie->_addColor(static_cast<const char*>(szColor));
			}

			szColor =  PP_evalProperty("shading-foreground-color",pBlockAP,NULL,NULL,m_pDocument,true);
			if (szColor)
			{
			    ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
			    if (ndxColor == -1)
			      m_pie->_addColor(static_cast<const char*>(szColor));
			}

			szColor =  PP_evalProperty("shading-background-color",pBlockAP,NULL,NULL,m_pDocument,true);
			if (szColor)
			{
			    ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
			    if (ndxColor == -1)
			      m_pie->_addColor(static_cast<const char*>(szColor));
			}

			return true;
		}

	default:
	  UT_ASSERT_NOT_REACHED();
		return false;
	}
}

bool s_RTF_ListenerGetProps::change(fl_ContainerLayout* /*sfh*/,
									   const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT_NOT_REACHED();	// this function is not used.
	return false;
}

bool s_RTF_ListenerGetProps::insertStrux(fl_ContainerLayout* /*sfh*/,
										  const PX_ChangeRecord * /*pcr*/,
										  pf_Frag_Strux* /*sdh*/,
										  PL_ListenerId /* lid */,
										  void (* /*pfnBindHandles*/)(pf_Frag_Strux* /* sdhNew */,
																	  PL_ListenerId /* lid */,
																	  fl_ContainerLayout* /* sfhNew */))
{
	UT_ASSERT_NOT_REACHED();	// this function is not used.
	return false;
}

bool s_RTF_ListenerGetProps::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT_NOT_REACHED();
	return false;
}

void s_RTF_ListenerGetProps::_searchTableAPI(PT_AttrPropIndex api)
{

	const PP_AttrProp * pTableAP = NULL;
	m_pDocument->getAttrProp(api,&pTableAP);
	const gchar * szColor = NULL;
	UT_sint32 ndxColor;

// Background

	szColor = PP_evalProperty("background-color",pTableAP,NULL,NULL,m_pDocument,true);

	if (szColor && g_ascii_strcasecmp (szColor, "transparent") != 0)
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}

// bgcolor

	szColor = PP_evalProperty("bgcolor",pTableAP,NULL,NULL,m_pDocument,true);

	if (szColor && g_ascii_strcasecmp (szColor, "transparent") != 0)
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}

// Left

	szColor = PP_evalProperty("left-color",pTableAP,NULL,NULL,m_pDocument,true);

	if (szColor && (g_ascii_strcasecmp (szColor, "transparent") != 0) && (g_ascii_strcasecmp (szColor, "inherit") != 0) )
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}

// Right

	szColor = PP_evalProperty("right-color",pTableAP,NULL,NULL,m_pDocument,true);

	if (szColor && (g_ascii_strcasecmp (szColor, "transparent") != 0) && (g_ascii_strcasecmp (szColor, "inherit") != 0) )
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}

// Bottom

	szColor = PP_evalProperty("bot-color",pTableAP,NULL,NULL,m_pDocument,true);

	if (szColor && (g_ascii_strcasecmp (szColor, "transparent") != 0) && (g_ascii_strcasecmp (szColor, "inherit") != 0) )
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}

// top

	szColor = PP_evalProperty("top-color",pTableAP,NULL,NULL,m_pDocument,true);

	if (szColor && (g_ascii_strcasecmp (szColor, "transparent") != 0) && (g_ascii_strcasecmp (szColor, "inherit") != 0) )
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}

}


void s_RTF_ListenerGetProps::_searchCellAPI(PT_AttrPropIndex api)
{

	const PP_AttrProp * pCellAP = NULL;
	m_pDocument->getAttrProp(api,&pCellAP);
	const gchar * szColor = NULL;
	UT_sint32 ndxColor;

// top

	szColor = PP_evalProperty("top-color",pCellAP,NULL,NULL,m_pDocument,true);

	if (szColor && (g_ascii_strcasecmp (szColor, "transparent") != 0) && (g_ascii_strcasecmp (szColor, "inherit") != 0) )
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}

// left

	szColor = PP_evalProperty("left-color",pCellAP,NULL,NULL,m_pDocument,true);

	if (szColor && (g_ascii_strcasecmp (szColor, "transparent") != 0) && (g_ascii_strcasecmp (szColor, "inherit") != 0) )
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}

// right

	szColor = PP_evalProperty("right-color",pCellAP,NULL,NULL,m_pDocument,true);

	if (szColor && (g_ascii_strcasecmp (szColor, "transparent") != 0) && (g_ascii_strcasecmp (szColor, "inherit") != 0) )
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}

// Bottom

	szColor = PP_evalProperty("bot-color",pCellAP,NULL,NULL,m_pDocument,true);

	if (szColor && (g_ascii_strcasecmp (szColor, "transparent") != 0) && (g_ascii_strcasecmp (szColor, "inherit") != 0) )
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}

// Background

	szColor = PP_evalProperty("background-color",pCellAP,NULL,NULL,m_pDocument,true);

	if (szColor && (g_ascii_strcasecmp (szColor, "transparent") != 0) && (g_ascii_strcasecmp (szColor, "inherit") != 0) )
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}


}

void s_RTF_ListenerGetProps::_check_revs_for_color(const PP_AttrProp * pAP1,
												   const PP_AttrProp * pAP2,
												   const PP_AttrProp * pAP3)
{
	const PP_AttrProp * pAP = NULL;

	for(UT_uint32 i = 0; i < 3; ++i)
	{
		if(i == 0)
			pAP = pAP1;
		else if(i == 1)
			pAP = pAP2;
		else
			pAP = pAP3;
		
		if(!pAP)
			continue;

		const gchar * pRev;
		char *pDup  = NULL;

		if(pAP->getAttribute("revision", pRev))
		{
			char * p;
			pDup = p = g_strdup(pRev);

			do
			{
				char * p1 = strstr(p, "color");
				char * p2 = strstr(p, "bgcolor");

				if(p1 && p2)
				{
					p = UT_MIN(p1,p2);
				}
				else if(p1)
				{
					p = p1;
				}
				else
				{
					p = p2;
				}
			
				if(p)
				{
					char * s = strchr(p, ':');
					if(s)
						++s;
					
					while(s && *s == ' ')
						++s;
				
					if(s)
					{
						char * e1 = strchr(s, ';');
						char * e2 = strchr(s, '}');
						char * e;

						if(e1 && e2)
						{
							e = UT_MIN(e1,e2);
						}
						else if(e1)
						{
							e = e1;
						}
						else
						{
							e = e2;
						}

						
						if(e)
						{
							*e = 0;
							p = e+1;
						}
						else
						{
							p = NULL;
						}

						m_pie->_findOrAddColor(s);
					}
				}
			
			}while(p);
		}
		else
			return;

		if(pDup)
		{
			g_free(pDup);
			pDup = NULL;
		}
	}
}

void s_RTF_ListenerGetProps::_check_revs_for_font(const PP_AttrProp * pAP1,
												  const PP_AttrProp * pAP2,
												  const PP_AttrProp * pAP3)
{
	const PP_AttrProp * pAP = NULL;

	for(UT_uint32 i = 0; i < 3; ++i)
	{
		if(i == 0)
			pAP = pAP1;
		else if(i == 1)
			pAP = pAP2;
		else
			pAP = pAP3;
		
		if(!pAP)
			continue;

		const gchar * pRev;
		char *pDup  = NULL;

		if(pAP->getAttribute("revision", pRev))
		{
			char * p;
			pDup = p = g_strdup(pRev);

			do
			{
				char * p1 = strstr(p, "font-family");
				char * p2 = strstr(p, "field-font");

				if(p1 && p2)
				{
					p = UT_MIN(p1,p2);
				}
				else if(p1)
				{
					p = p1;
				}
				else
				{
					p = p2;
				}
			
				if(p)
				{
					char * s = strchr(p, ':');
					if(s)
						++s;
					
					while(s && *s == ' ')
						++s;
				
					if(s)
					{
						char * e1 = strchr(s, ';');
						char * e2 = strchr(s, '}');
						char * e;

						if(e1 && e2)
						{
							e = UT_MIN(e1,e2);
						}
						else if(e1)
						{
							e = e1;
						}
						else
						{
							e = e2;
						}

						
						if(e)
						{
							*e = 0;
							p = e+1;
						}
						else
						{
							p = NULL;
						}

						{
							_rtf_font_info fi;

							if (fi.init(s))
							{
								UT_sint32 ndxFont = m_pie->_findFont(&fi);
								if (ndxFont == -1)
									m_pie->_addFont(&fi);
							}
						}
					}
				}
			
			}while(p);
		}
		else
			return;

		if(pDup)
		{
			g_free(pDup);
			pDup = NULL;
		}
	}
}

void s_RTF_ListenerGetProps::_compute_span_properties(const PP_AttrProp * pSpanAP,
													  const PP_AttrProp * pBlockAP,
													  const PP_AttrProp * pSectionAP)
{
	// see if we have a previously unused color reference.
	
	const gchar * szColor = PP_evalProperty("color",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);
	UT_sint32 ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
	if (ndxColor == -1)
		m_pie->_addColor(static_cast<const char*>(szColor));
	
	szColor = PP_evalProperty("bgcolor",pSpanAP,pBlockAP,pSectionAP,m_pDocument,true);

	if (g_ascii_strcasecmp (szColor, "transparent") != 0)
	{
		ndxColor = m_pie->_findColor(static_cast<const char*>(szColor));
		if (ndxColor == -1)
			m_pie->_addColor(static_cast<const char*>(szColor));
	}

	// also check the revision attribute
	_check_revs_for_color(pSpanAP, pBlockAP, pSectionAP);
	
	// convert our font properties into an item for the rtf font table.
	// in this pass thru the document we are just collecting all the
	// info that we need to put into the rtf header, so we can't just
	// write it out now.  so, we build a vector of the stuff we want
	// to write (and make sure it's unique).

	UT_sint32 ndxFont;

	{
		_rtf_font_info fi;

		if (fi.init(s_RTF_AttrPropAdapter_AP(pSpanAP,pBlockAP,pSectionAP,m_pDocument))) {
			ndxFont = m_pie->_findFont(&fi);
			if (ndxFont == -1)
				m_pie->_addFont(&fi);
		}
	}

//
// Look in field-font too
//
	{
		_rtf_font_info fii;

		if (fii.init(s_RTF_AttrPropAdapter_AP(pSpanAP,pBlockAP,pSectionAP,m_pDocument),true)) {
			ndxFont = m_pie->_findFont(&fii);
			if (ndxFont == -1)
				m_pie->_addFont(&fii);
		}
	}

	// also check the revision attribute
	_check_revs_for_font(pSpanAP, pBlockAP, pSectionAP);
	
}


