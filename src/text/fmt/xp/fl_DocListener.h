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

#ifndef FL_DOCLISTENER_H
#define FL_DOCLISTENER_H

#include "ut_types.h"
#include "pt_Types.h"

#include "pl_Listener.h"

class FL_DocLayout;
class PD_Document;
class fl_SectionLayout;

/*!
	The fl_DocListener class handles notifications from a PD_Document 
	to its associated FL_DocLayout. 
*/

class fl_DocListener : public PL_Listener
{
public:
	fl_DocListener(PD_Document* doc, FL_DocLayout *pLayout);
	virtual ~fl_DocListener();

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
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual UT_Bool		signal(UT_uint32 iSignal);

protected:
	//! Document which is client of this DocListener
	PD_Document*		m_pDoc;
	//! The Layout notified by this DocListener
	FL_DocLayout*		m_pLayout;
	//! Set when the document is drawn on screen
	UT_Bool				m_bScreen;
	//! Counter used to keep track of when to update the Layout. In
	//! case of multi-step changes, updating is suspended.
	UT_uint32			m_iGlobCounter;
	//! SectionLayout currently being constructed (multi-step change
	//! related?!?)
	fl_SectionLayout*	m_pCurrentSL;
};

#endif /* FL_DOCLISTENER_H */
