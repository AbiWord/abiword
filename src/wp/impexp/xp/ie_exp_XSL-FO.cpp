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

#include <locale.h>

#include "ut_string.h"
#include "ut_types.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "ie_exp_XSL-FO.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_App.h"
#include "ap_Prefs.h"
#include "pd_Style.h"
#include "fd_Field.h"
#include "xap_EncodingManager.h"
#include "fl_AutoNum.h"
#include "fp_PageSize.h"

/*****************************************************************/
/*****************************************************************/

IE_Exp_XSL_FO::IE_Exp_XSL_FO(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
}

IE_Exp_XSL_FO::~IE_Exp_XSL_FO()
{
	// nada
}

/*****************************************************************/
/*****************************************************************/

bool IE_Exp_XSL_FO::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".fo") == 0);
}

UT_Error IE_Exp_XSL_FO::StaticConstructor(PD_Document * pDocument,
										  IE_Exp ** ppie)
{
	IE_Exp_XSL_FO * p = new IE_Exp_XSL_FO(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Exp_XSL_FO::GetDlgLabels(const char ** pszDesc,
									const char ** pszSuffixList,
									IEFileType * ft)
{
	*pszDesc = "XSL-FO (.fo)";
	*pszSuffixList = "*.fo";
	*ft = IEFT_XSL_FO;
	return true;
}

bool IE_Exp_XSL_FO::SupportsFileType(IEFileType ft)
{
	return (IEFT_XSL_FO == ft);
}

UT_Error IE_Exp_XSL_FO::_writeDocument(void)
{
	// todo
	return UT_OK;
}  

/*****************************************************************/
/*****************************************************************/

class s_XSL_FO_Listener : public PL_Listener
{
public:
	s_XSL_FO_Listener(PD_Document * pDocument,
					  IE_Exp_XSL_FO * pie);
	virtual ~s_XSL_FO_Listener();

	virtual bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual bool		signal(UT_uint32 iSignal);
protected:
	
	PD_Document *		m_pDocument;
	IE_Exp_XSL_FO *	    m_pie;
};

s_XSL_FO_Listener::s_XSL_FO_Listener(PD_Document * pDocument,
									 IE_Exp_XSL_FO * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	// TODO
}

s_XSL_FO_Listener::~s_XSL_FO_Listener()
{
	// TODO
}

bool s_XSL_FO_Listener::populate(PL_StruxFmtHandle /*sfh*/,
								 const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			PT_BufIndex bi = pcrs->getBufIndex();
			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();
			switch (pcro->getObjectType())
			{
			case PTO_Image:
				return true;

			case PTO_Field:
                {
                    return true;
                }
			default:
				UT_ASSERT(0);
				return false;
			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;
		
	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_XSL_FO_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
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
			return true;
		}

	case PTX_SectionHdrFtr:
		{
			return true;
		}

	case PTX_Block:
		{
			return true;
		}

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_XSL_FO_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}

bool s_XSL_FO_Listener::change(PL_StruxFmtHandle /*sfh*/,
							   const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_XSL_FO_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/,
									PL_StruxDocHandle /*sdh*/,
									PL_ListenerId /* lid */,
									void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																PL_ListenerId /* lid */,
																PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}
