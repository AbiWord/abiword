/*
 * Copyright (C) 2005 by Martin Sevior
 * Copyright (C) 2006,2007 by Marc Maurer <uwog@uwog.net>
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

#ifndef ABICOLLABSERVICE_EXPORT_H
#define ABICOLLABSERVICE_EXPORT_H

#include "ut_types.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "ut_stack.h"
#include "stdio.h"
#include "xav_Listener.h"
#include "pl_Listener.h"
#include "ut_string_class.h"

class FL_DocLayout;
class PD_Document;
class UT_Stack;
class PX_ChangeRecord;
class ChangeAdjust;
class ServiceAccountHandler;

class AbiCollabService_Export :  public PL_DocChangeListener
{
public:
  AbiCollabService_Export(PD_Document* pDoc, ServiceAccountHandler* pService );
	virtual ~AbiCollabService_Export(void);

	virtual bool		populate(fl_ContainerLayout* sfh,
								 const PX_ChangeRecord* pcr);

	virtual bool		populateStrux(pf_Frag_Strux* sdh,
									  const PX_ChangeRecord* pcr,
									  fl_ContainerLayout** psfh);

	virtual bool		change(fl_ContainerLayout* sfh,
							   const PX_ChangeRecord* pcr);


	virtual void		deferNotifications(void);
	virtual void		processDeferredNotifications(void);

	virtual bool		insertStrux(fl_ContainerLayout* sfh,
									const PX_ChangeRecord* pcr,
									pf_Frag_Strux* sdh,
									PL_ListenerId lid,
									void (*pfnBindHandles)(pf_Frag_Strux* sdhNew,
															PL_ListenerId lid,
															fl_ContainerLayout* sfhNew));

	virtual bool		signal(UT_uint32 iSignal);

	virtual				PLListenerType getType() const { return PTL_CollabServiceExport; }
	virtual void		setNewDocument(PD_Document * pDoc);
	virtual void		removeDocument(void);
	PD_Document *           getDocument(void)
	{ return m_pDoc;}
private:
	PD_Document*		m_pDoc;
	ServiceAccountHandler * m_pService;
};

#endif /* ABICOLLABSERVICE_EXPORT_H */
