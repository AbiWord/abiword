/* AbiWord
 * Copyright (C) 2010 Martin Sevior
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


#ifndef IE_EXP_PDOCRANGELISTENER_H
#define  IE_EXP_PDOCRANGELISTENER_H

#include "ut_types.h"
#include "pt_Types.h"
#include "pd_Document.h"
class PX_ChangeRecord;

#ifdef __sgi
// <sys/signal.h> may #define signal, leaving PL_Listener::signal() pure
// virtual if subclasses don't include the same header files.
// Please keep the "/**/" to stop MSVC dependency generator complaining.
#include /**/ <sys/signal.h>
#endif

// PL_Listener -- A layout registers a listener with the
//                PD_Document in order to be notified of
//                any changes to the document as they
//                occur.  The document will notify each
//                registered listener (in an undefined
//                order).  When the listener registers,
//                it is provided an ID which may be used
//                later to refer to it.

class ABI_EXPORT IE_Exp_DocRangeListener: public PL_Listener
{
public:
  IE_Exp_DocRangeListener( PD_DocumentRange * pDocRange, PD_Document * pOutDoc);
  virtual ~IE_Exp_DocRangeListener(){}

  virtual bool		populate(fl_ContainerLayout* sfh,
				 const PX_ChangeRecord * pcr);

  virtual bool		populateStrux(pf_Frag_Strux* sdh,
				      const PX_ChangeRecord * pcr,
				      fl_ContainerLayout* * psfh);

  virtual bool		change(fl_ContainerLayout* /*sfh*/,
			       const PX_ChangeRecord * /*pcr*/)
		{return true;}

  virtual bool		insertStrux(fl_ContainerLayout* /*sfh*/,
				    const PX_ChangeRecord * /*pcr*/,
				    pf_Frag_Strux* /*sdhNew*/,
				    PL_ListenerId /*lid*/,
				    void (* /*pfnBindHandles*/)(pf_Frag_Strux* sdhNew,
								PL_ListenerId lid,
								fl_ContainerLayout* sfhNew))
  { return true;}

	virtual bool		signal(UT_uint32 /*iSignal*/)
		{ return true;}
	virtual PLListenerType getType() const
		{
			return PTL_UNKNOWN;
		}
	static void  assembleAtts(const PP_PropertyVector & inAtts, const PP_PropertyVector & inProps, PP_PropertyVector & outAtts);

private:
	PD_Document *     getDoc(void) const;
	PD_Document *     m_pOutDocument;
	bool              m_bFirstSection;
	bool              m_bFirstBlock;
	PD_Document *     m_pSourceDoc;
	PD_DocumentRange * m_pDocRange;
	PT_AttrPropIndex   m_iLastAP;
};
#endif  /* IE_EXP_PDOCRANGELISTENER_H */
