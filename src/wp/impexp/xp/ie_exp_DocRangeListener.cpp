/* AbiWord
 * Copyright (C) 2010 Martin Sevior <msevior@physics.unimelb.edu.au>
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


#include "ie_imp_PasteListener.h"
#include "pp_AttrProp.h"
#include "pf_Frag_Strux.h"
#include "px_CR_FmtMark.h"
#include "px_CR_FmtMarkChange.h"
#include "px_CR_Object.h"
#include "px_CR_ObjectChange.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "ie_exp_DocRangeListener.h"
#include "pd_Style.h"
#include "ut_string_class.h"
#include <string.h>
#include <glib.h>

/*!
 * This nifty little class allows a docrange to be exported into blank
 * document.
 * We can then export the content of this document to any of our supported
 * formats. This allows us to export any selection into any document
 * format. We can then place this content on the clipbaord for other
 * application to read.
 */
IE_Exp_DocRangeListener::IE_Exp_DocRangeListener(PD_DocumentRange * pDocRange, PD_Document * pOutDoc) : 
  m_pOutDocument(pOutDoc),
  m_bFirstSection(false),
  m_bFirstBlock(false),
  m_pSourceDoc(pDocRange->m_pDoc),
  m_pDocRange(pDocRange),
  m_iLastAP(0)
{
  //
  // Start by exporting the data items to the document
  //
     PD_DataItemHandle pHandle = NULL;
     std::string mimeType;
     const char * szName= NULL;
     const UT_ByteBuf * pBuf = NULL;
     UT_sint32 k = 0;
     while(m_pSourceDoc->enumDataItems(k,&pHandle,&szName,&pBuf,&mimeType))
     {
          getDoc()->createDataItem(szName,false,pBuf,mimeType,&pHandle);
	  k++;
     }
  //
  // Next export all the styles
  //
     UT_GenericVector<PD_Style*> VecStyles;
     m_pSourceDoc->getAllUsedStyles(&VecStyles);
     UT_sint32 i = 0;
     for(i=0; i< VecStyles.getItemCount();i++)
     {
          PD_Style * pStyle = VecStyles.getNthItem(i);
          PT_AttrPropIndex iAP = pStyle->getIndexAP();
          const char ** atts = NULL;
          const PP_AttrProp* pAP = NULL;
          if (m_pSourceDoc->getAttrProp(iAP, &pAP) && pAP)
          {
		atts = pAP->getAttributes();
	  } 
	  getDoc()->appendStyle(atts);
     }
}

/*!
 * outAtts must be freed after use.
 */	
void  IE_Exp_DocRangeListener::assembleAtts(const char ** inAtts, const char ** inProps, const char **& sAtts)
{
  const char * szKey = NULL;
  const char * szVal = NULL;
  UT_sint32 i= 0;
  UT_UTF8String sAllProps;
  UT_UTF8String sProp;
  UT_UTF8String sVal;
  bool bHasProps = false;
  UT_GenericVector<const char *> vecAtts;
  while(inAtts && inAtts[i])
  {
    szKey = inAtts[i++];
    szVal = inAtts[i++];
    xxx_UT_DEBUGMSG((" attribute %d key %s val %s \n",i-2,szKey,szVal));
    vecAtts.addItem(szKey);
    vecAtts.addItem(szVal);
    if(g_strcmp0(szKey,"props") == 0)
    {
	bHasProps = true;
    }
  }
  UT_sint32 attsCount = i;
  UT_sint32 propsCount = 0;
  if(!bHasProps)
  {
    i= 0;
    while(inProps && inProps[i])
    {
        xxx_UT_DEBUGMSG((" Prip %d prop %s val %s \n",i,inProps[i],inProps[i+1]));
	sProp = inProps[i++];
	sVal = inProps[i++];
	UT_UTF8String_setProperty(sAllProps,sProp,sVal);
    }	
    propsCount = i;
  }
  //UT_sint32 iSpace = 0;
  if(bHasProps || (propsCount == 0))
  {
    sAtts = new const char*[attsCount+1];
    //iSpace = attsCount+1;
  }
  else if(propsCount > 0)
  {
     sAtts = new const char*[attsCount+3];
     //iSpace = attsCount+1;
  }
  else if((attsCount == 0) && (propsCount == 0))
  {
    sAtts = NULL;
    return;
  }

  //UT_DEBUGMSG(("iSpace count %d \n",iSpace));
  for(i=0; i<vecAtts.getItemCount();i++)
  {
    szVal = vecAtts.getNthItem(i);
    xxx_UT_DEBUGMSG((" attribute %d val %s \n",i,szVal));
     sAtts[i] = g_strdup(szVal);
  }
  if(bHasProps ||(propsCount == 0) )
  {
      sAtts[i] = NULL;
      return;
  }
  sAtts[i++] = g_strdup("props");
  sAtts[i++] = g_strdup(sAllProps.utf8_str());
  sAtts[i++] = NULL;
  return;
}
 
void  IE_Exp_DocRangeListener::freeAtts(const char *** allAtts)
{
  const gchar ** sAtts = *allAtts;
  if(sAtts == NULL)
    return;
  UT_sint32 i=0;
  while(sAtts[i])
  {
      g_free(const_cast<char*>(sAtts[i]));
      i++;
  }
  delete [] sAtts;
}

bool  IE_Exp_DocRangeListener::populate(fl_ContainerLayout* /* sfh */,
					 const PX_ChangeRecord * pcr)
{
	if(!m_bFirstSection)
	{
	     getDoc()->appendStrux(PTX_Section,NULL);
	     m_bFirstSection = true;
	}
	if(!m_bFirstBlock)
	{
	     getDoc()->appendStrux(PTX_Block,NULL);
	     m_bFirstBlock = true;
	}
	PT_AttrPropIndex indexAP = pcr->getIndexAP();
	const PP_AttrProp* pAP = NULL;
	xxx_UT_DEBUGMSG(("SEVIOR: Doing Populate in PasteListener indexAP %d \n",indexAP));
	const char ** atts = NULL;
	const char ** props = NULL;
	if (m_pSourceDoc->getAttrProp(indexAP, &pAP) && pAP)
	{
		atts = pAP->getAttributes();
		props = pAP->getProperties();
	}
	else
	{
		return false;
	}
	const char ** allAtts= NULL;
	assembleAtts(atts, props, allAtts);
	bool bAppendFmt = (m_iLastAP != indexAP);
	m_iLastAP = indexAP;
    
	UT_DEBUGMSG(("MIQ: Doing Populate in PasteListener indexAP %d pcr.type:%d \n",
                 indexAP, pcr->getType() ));
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
	{
		const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);
		UT_uint32 len = pcrs->getLength();
		xxx_UT_DEBUGMSG(("Insert span index AP %d \n",indexAP));
		PT_BufIndex bi = pcrs->getBufIndex();
		const UT_UCSChar* pChars = 	m_pSourceDoc->getPointer(bi);
		if(bAppendFmt)
		{
		    getDoc()->appendFmt(allAtts);
		}
		getDoc()->appendSpan(pChars,len);
		freeAtts(&allAtts);
		return true;
	}

	case PX_ChangeRecord::PXT_InsertObject:
	{
		const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);
		getDoc()->appendObject(pcro->getObjectType(),allAtts);
		freeAtts(&allAtts);
		return true;
	}

	case PX_ChangeRecord::PXT_InsertFmtMark:
	{
		xxx_UT_DEBUGMSG(("Insert FmtMark index AP %d \n",indexAP));
	        getDoc()->appendFmt(allAtts);
		freeAtts(&allAtts);
		return true;
	}
	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	return true;
}

bool  IE_Exp_DocRangeListener::populateStrux(pf_Frag_Strux* /*sdh*/,
					     const PX_ChangeRecord * pcr,
					     fl_ContainerLayout* * /* psfh */)
{
//
// TODO graphics in struxes
// TODO UID stuff
//
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	PT_AttrPropIndex indexAP = pcr->getIndexAP();
	const PP_AttrProp* pAP = NULL;
	xxx_UT_DEBUGMSG(("SEVIOR: Doing Populate Strux in PasteListener \n"));
	const char ** atts = NULL;
	const char ** props = NULL;
	if (m_pSourceDoc->getAttrProp(indexAP, &pAP) && pAP)
	{
		atts = pAP->getAttributes();
		props = pAP->getProperties();
	}
	else
	{
		return false;
	}
	const char ** allAtts= NULL;
	assembleAtts(atts, props, allAtts);
	if((pcrx->getStruxType()== PTX_Section) && !m_bFirstSection)
	{
	    m_bFirstSection = true;
	}
	if((pcrx->getStruxType()== PTX_Block) && !m_bFirstBlock)
	{
	    m_bFirstBlock = true;
	}
	if(!m_bFirstSection && pcrx->getStruxType() != PTX_Section)
	{
	     getDoc()->appendStrux(PTX_Section,NULL);
	     m_bFirstSection = true;
	}
	if(!m_bFirstBlock && (pcrx->getStruxType() != PTX_Section) && (pcrx->getStruxType() != PTX_Block))
	{
	     getDoc()->appendStrux(PTX_Block,NULL);
	     m_bFirstBlock = true;
	}
	getDoc()->appendStrux(pcrx->getStruxType(),allAtts);
	freeAtts(&allAtts);
	return true;
}

PD_Document * IE_Exp_DocRangeListener::getDoc(void) const
{
	return m_pOutDocument;
}

