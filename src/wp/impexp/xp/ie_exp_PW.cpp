/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz
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

#include "ie_exp_PW.h"

#if 0

// this will be used as a Saig PatheticWriter exporter

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

ABI_PLUGIN_DECLARE("PW")

// we use a reference-counted sniffer
static IE_Exp_Sniffer * m_sniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Exp_PW_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	mi->name = "Saig PW Exporter";
	mi->desc = "Export PatheticWriter Documents";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Exp::registerExporter (m_sniffer);
	return 1;
}

ABI_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_ASSERT (m_sniffer);

	IE_Exp::unregisterExporter (m_sniffer);
	if (!m_sniffer->unref())
	{
		m_sniffer = 0;
	}

	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
  return 1;
}

#endif

/*****************************************************************/
/*****************************************************************/

IE_Exp_PW::IE_Exp_PW(PD_Document * pDocument)
	: IE_Exp(pDocument), m_pListener(0)
{
}

IE_Exp_XSL_FO::~IE_Exp_PW()
{
}

/*****************************************************************/
/*****************************************************************/

bool IE_Exp_PW_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix,".pw"));
}

UT_Error IE_Exp_XSL_PW_Sniffer::constructExporter(PD_Document * pDocument,
						  IE_Exp ** ppie)
{
	IE_Exp_PW * p = new IE_Exp_PW(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_XSL_FO_Sniffer::getDlgLabels(const char ** pszDesc,
					 const char ** pszSuffixList,
					 IEFileType * ft)
{
	*pszDesc = "Pathetic Writer (.pw)";
	*pszSuffixList = "*.pw";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_PW::_writeDocument(void)
{
	m_pListener = new s_PW_Listener(getDoc(),this);
	if (!m_pListener)
		return UT_IE_NOMEMORY;
	if (!getDoc()->tellListener(static_cast<PL_Listener *>(m_pListener)))
		return UT_ERROR;
	delete m_pListener;

	m_pListener = NULL;
	
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}  

class s_PW_Listener : public PL_Listener
{
public:
	s_PW_Listener(PD_Document * pDocument,
		      IE_Exp_PW * pie);
	virtual ~s_PW_Listener();

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
  
private:
	PD_Document *		m_pDocument;
	IE_Exp_PW *	    m_pie;
};

#endif
