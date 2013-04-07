/* AbiWord
 * Copyright (C) 2005 Dom Lachowicz
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

#include "ie_TOC.h"
#include "pd_Document.h"
#include "ut_string.h"
#include "ut_misc.h"

#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "pp_AttrProp.h"
#include "pd_Style.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"

/*******************************************************************************/
/*******************************************************************************/

class ABI_EXPORT TOC_Listener : public PL_Listener
{
public:
  TOC_Listener(PD_Document * pDocument,
	       IE_TOCHelper * toc);
  
  virtual ~TOC_Listener();
  
  virtual bool		populate(fl_ContainerLayout* sfh,
				 const PX_ChangeRecord * pcr);
  
  virtual bool		populateStrux(pf_Frag_Strux* sdh,
				      const PX_ChangeRecord * pcr,
				      fl_ContainerLayout* * psfh);
  
  virtual bool		change(fl_ContainerLayout* sfh,
			       const PX_ChangeRecord * pcr);
  
  virtual bool		insertStrux(fl_ContainerLayout* sfh,
				    const PX_ChangeRecord * pcr,
				    pf_Frag_Strux* sdh,
				    PL_ListenerId lid,
				    void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
							    PL_ListenerId lid,
							    fl_ContainerLayout* sfhNew));
  
  virtual bool		signal(UT_uint32 iSignal);
  
  
private:

  void _saveTOCData(const UT_UCSChar * data, UT_uint32 length);
  void _commitTOCData();

  bool mInHeading;
  UT_UTF8String mHeadingText;
  int mHeadingLevel;
  PT_DocPosition mHeadingPos;

  PD_Document * mDocument;
  IE_TOCHelper * mTOC;
};

/*******************************************************************************/
/*******************************************************************************/

void TOC_Listener::_saveTOCData(const UT_UCSChar * data, UT_uint32 length)
{
  const UT_UCSChar * pData;
  
  UT_return_if_fail(sizeof(UT_Byte) == sizeof(char));
  
  for (pData=data; (pData<data+length); pData++)
    {
      mHeadingText.appendUCS4 (pData, 1);
    }
}

void TOC_Listener::_commitTOCData()
{
  if (mInHeading) {
    mTOC->_defineTOC(mHeadingText, mHeadingLevel, mHeadingPos);
  }

  mInHeading = false;
  mHeadingText.clear();
  mHeadingLevel = 0;
  mHeadingPos = 0;
}

TOC_Listener::TOC_Listener(PD_Document * pDocument,
			   IE_TOCHelper * toc)
  : mInHeading(0), 
    mHeadingText(""), 
    mHeadingLevel(0),
    mHeadingPos(0),
    mDocument(pDocument), 
    mTOC(toc)
{
}
  
TOC_Listener::~TOC_Listener()
{
  _commitTOCData();
}

bool TOC_Listener::populate(fl_ContainerLayout* /*sfh*/,
			    const PX_ChangeRecord * pcr)
{
  switch (pcr->getType())
    {
    case PX_ChangeRecord::PXT_InsertSpan:
      {
	if (mInHeading) {
	  const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);
	  PT_BufIndex bi = pcrs->getBufIndex();
	  _saveTOCData(mDocument->getPointer(bi),pcrs->getLength());
	}

	return true;
      }

    default:
      return true;
    }
}

bool TOC_Listener::populateStrux(pf_Frag_Strux* /*sdh*/,
				 const PX_ChangeRecord * pcr,
				 fl_ContainerLayout* * psfh)
{
  UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);
  const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
  *psfh = 0;
  
  // resets all TOC foo
  _commitTOCData();

  switch (pcrx->getStruxType())
    {
    case PTX_SectionTOC:
      {
	mTOC->mDocHasTOC = true;
	return true;
      }
    case PTX_Block:
      {
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = mDocument->getAttrProp (pcr->getIndexAP(), &pAP);

	if (bHaveProp) {
	  const gchar * szValue = 0;
	  bool bHaveStyle  = pAP->getAttribute (PT_STYLE_ATTRIBUTE_NAME,  szValue);
	  
	  if (bHaveStyle) {
	    if (mTOC->isTOCStyle (szValue, &mHeadingLevel)) {
	      mInHeading = true;
	      mHeadingPos = pcr->getPosition();
	    }
	  }
	}

	return true;
      }
      
    default:
      return true;
    }
}

bool TOC_Listener::change(fl_ContainerLayout* /*sfh*/,
			  const PX_ChangeRecord * /*pcr*/)
{
  UT_ASSERT_NOT_REACHED();
  return false;
}

bool TOC_Listener::insertStrux(fl_ContainerLayout* /*sfh*/,
			       const PX_ChangeRecord * /*pcr*/,
			       pf_Frag_Strux* /*sdh*/,
			       PL_ListenerId /* lid */,
			       void (* /*pfnBindHandles*/)(pf_Frag_Strux* /* sdhNew */,
							   PL_ListenerId /* lid */,
							   fl_ContainerLayout* /* sfhNew */))
{
  UT_ASSERT_NOT_REACHED();						// this function is not used.
  return false;
}

bool TOC_Listener::signal(UT_uint32 /* iSignal */)
{
  UT_ASSERT_NOT_REACHED();
  return false;
}

/*******************************************************************************/
/*******************************************************************************/

IE_TOCHelper::IE_TOCHelper(PD_Document * pDoc)
  : mHasTOC(false), mDocHasTOC(false), mDoc(pDoc)
{
  TOC_Listener listener(pDoc,this);

  pDoc->tellListener(&listener);
}

IE_TOCHelper::~IE_TOCHelper()
{
  UT_VECTOR_PURGEALL(UT_UTF8String *, mTOCStrings);
}

bool IE_TOCHelper::hasTOC() const
{
  return mHasTOC;
}

bool IE_TOCHelper::docHasTOC() const
{
  return mDocHasTOC;
}

bool IE_TOCHelper::_tocNameLevelHelper(const UT_UTF8String & style_name,
				       const char * base_name) const
{
  PD_Style * style = 0;
  mDoc->getStyle (style_name.utf8_str(), &style);
  UT_sint32 iLoop = 0;
  
  while (style && (iLoop < 10))
    {
      if (g_ascii_strcasecmp (base_name, style->getName ()) == 0)
	return true;
      
      style = style->getBasedOn ();
      iLoop++;
    }

  return false;
}

bool IE_TOCHelper::isTOCStyle(const UT_UTF8String & styleName, int * out_level) const
{
  // TODO: support user-defined toc-styles...
  // !pSectionAP->getProperty("toc-source-style1",pszTOCSRC))

  if (_tocNameLevelHelper (styleName, "Heading 1")) {
    if (out_level) 
      *out_level = 1;
    return true;
  } else if (_tocNameLevelHelper (styleName, "Heading 2")) {
    if (out_level) 
      *out_level = 2;
    return true;
  } else if (_tocNameLevelHelper (styleName, "Heading 3")) {
    if (out_level) 
      *out_level = 3;
    return true;
  } else if (_tocNameLevelHelper (styleName, "Heading 4")) {
    if (out_level) 
      *out_level = 4;
    return true;
  }

  return false;
}

bool IE_TOCHelper::isTOCStyle(const gchar * styleName, int * out_level) const
{
  return isTOCStyle(UT_UTF8String(styleName), out_level);
}

int IE_TOCHelper::getNumTOCEntries() const
{
  return mTOCStrings.size ();
}

UT_UTF8String IE_TOCHelper::getNthTOCEntry(int nth, int * out_level) const
{
  UT_return_val_if_fail(nth < getNumTOCEntries(), "");

  if (out_level != NULL)
    *out_level = mTOCLevels[nth];

  return *mTOCStrings.getNthItem(nth);
}

bool IE_TOCHelper::getNthTOCEntryPos(int nth, PT_DocPosition &pos) const
{
	UT_return_val_if_fail(nth < getNumTOCEntries(), false);

	pos = mTOCPositions[nth];
	return true;
}

void IE_TOCHelper::_defineTOC(const UT_UTF8String & toc_text, int level, PT_DocPosition pos)
{
  if(toc_text.length() > 0) {
    mHasTOC = true;
    
    mTOCStrings.push_back(new UT_UTF8String(toc_text));
    mTOCLevels.push_back(level);
    mTOCPositions.push_back(pos);
  }
}
