 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/


#ifndef FL_DOCLISTENER_H
#define FL_DOCLISTENER_H

#include "ut_types.h"
#include "pt_Types.h"

#include "pl_Listener.h"

class FL_DocLayout;
class PD_Document;

/*
	The fl_DocListener class notifications from a PD_Document to its 
	associated FL_DocLayout. 
*/

class fl_DocListener : public PL_Listener
{
public:
	fl_DocListener(PD_Document* doc, FL_DocLayout *pLayout);
	~fl_DocListener();

	virtual UT_Bool		populate(PL_StruxFmtHandle sfh,
								 PX_ChangeRecord * pcr);

	virtual UT_Bool		populateStrux(PL_StruxDocHandle sdh,
									  PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual UT_Bool		change(PL_StruxFmtHandle sfh,
							   PX_ChangeRecord * pcr);

	virtual UT_Bool		insertStrux(PL_StruxFmtHandle sfh,
									PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_StruxFmtHandle * psfh);

protected:
	PD_Document*		m_pDoc;
	FL_DocLayout*		m_pLayout;
};

#endif /* FL_DOCLISTENER_H */
