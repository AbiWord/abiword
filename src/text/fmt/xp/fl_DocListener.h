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

#ifndef FL_DOCLISTENER_H
#define FL_DOCLISTENER_H

#include "ut_types.h"
#include "pt_Types.h"
#include "ut_stack.h"
#include "xav_Listener.h"
#include "pl_Listener.h"

class FL_DocLayout;
class PD_Document;
class fl_SectionLayout;
class UT_Stack;
class fl_ContainerLayout;
class AP_StatusBar;

/*!
	The fl_DocListener class handles notifications from a PD_Document
	to its associated FL_DocLayout.
*/

class ABI_EXPORT fl_DocListener : public PL_Listener
{
public:
	fl_DocListener(PD_Document* doc, FL_DocLayout *pLayout);
	virtual ~fl_DocListener();

	virtual bool		populate(fl_ContainerLayout* sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(pf_Frag_Strux* sdh,
									  const PX_ChangeRecord * pcr,
									  fl_ContainerLayout* * psfh);

	virtual bool		change(fl_ContainerLayout* sfh,
							   const PX_ChangeRecord * pcr);


	virtual void		deferNotifications(void);
	virtual void		processDeferredNotifications(void);

	virtual bool		insertStrux(fl_ContainerLayout* sfh,
									const PX_ChangeRecord * pcr,
									pf_Frag_Strux* sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
															PL_ListenerId lid,
															fl_ContainerLayout* sfhNew));
	void                setHoldTableLayout(bool bHold) {m_bHoldTableLayout = bHold;}
	bool                holdTableLayout(void) { return m_bHoldTableLayout;}
	virtual bool		signal(UT_uint32 iSignal);
	virtual PLListenerType getType() const {return PTL_DocLayout;}

	const FL_DocLayout* getLayout() const {return (const FL_DocLayout*) m_pLayout;}

private:
	fl_ContainerLayout *   popContainerLayout(void);
	void 		           pushContainerLayout(fl_ContainerLayout * pCL);
	fl_ContainerLayout *   getTopContainerLayout(void);

	//! Document which is client of this DocListener
	PD_Document*		   m_pDoc;
	//! The Layout notified by this DocListener
	FL_DocLayout*		   m_pLayout;
	//! Set when the document is drawn on screen
	bool				   m_bScreen;
	bool                   m_bHoldTableLayout;
	//! Counter used to keep track of when to update the Layout. In
	//! case of multi-step changes, updating is suspended.
	UT_uint32			   m_iGlobCounter;
	//! SectionLayout currently being constructed (multi-step change
	//! related?!?)
	fl_SectionLayout*	   m_pCurrentSL;
	UT_Stack               m_sLastContainerLayout;
	bool                   m_bFootnoteInProgress;
	bool                   m_bEndFootnoteProcessedInBlock;
	AV_ChangeMask          m_chgMaskCached;
	bool                   m_bCacheChanges;
	AP_StatusBar *         m_pStatusBar;
	UT_sint32              m_iFilled;
};

#endif /* FL_DOCLISTENER_H */


