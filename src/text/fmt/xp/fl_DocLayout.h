 
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


#ifndef DOCLAYOUT_H
#define DOCLAYOUT_H

#include "ut_types.h"
#include "ut_vector.h"

#include "dg_DocMarker.h"
#include "pl_Listener.h"

class FP_Page;
class DG_Graphics;
class DG_Document;
class DG_DocBuffer;
class DG_LayoutView;

// ----------------------------------------------------------------
/*
	FL_DocLayout is a formatted representation of a specific DG_Document, 
	formatted for a specific DG_Graphics context.  

	A FL_DocLayout encapsulates two related hierarchies of objects.  

	The "logical" hierarchy corresponds to the logical structure of the 
	document, and consists of a list of FL_SectionLayout objects, which are 
	in turn composed of FL_BlockLayout objects. 

	The "physical" hierarchy, by contrast, encapsulates the subdivision 
	of physical space into objects of successively finer granularity.  Thus, 
	a FL_DocLayout is also a list of Pages, each of which was constructed 
	during the process of formatting the document.  In turn, 
	
		each FP_Page is a list of SectionSlices.  
		Each FP_SectionSlice is a list of Columns.  
		Each FP_Column is a list of BlockSlices.
		Each FP_BlockSlice is a list of Lines.  
		Each FP_Line is a list of Runs.  
		
	Finally, each FP_Run contains some fragment of content from the original 
	document, usually text.
*/

class FL_DocLayout : public PL_Listener
{
public:
	FL_DocLayout(DG_Document* doc, DG_Graphics* pG);
	~FL_DocLayout();

	void setLayoutView(DG_LayoutView*);

	DG_Graphics*	getGraphics();
	DG_Document*	getDocument() const;
	DG_DocBuffer*	getBuffer() const;
	UT_uint32		getHeight();

	FP_Page*	addNewPage();
	FP_Page*	getFirstPage();
	FP_Page*	getLastPage();
	FP_Page*	getNthPage(int n);
	int			countPages();

	int			formatAll();
	int			reformat();

	// Listener-related routines
	
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

	// Debug-related routined
	
	void dump();
	
protected:
	DG_Graphics*		m_pG;
	DG_Document*		m_pDoc;
	DG_DocBuffer*	   	m_pBuffer;
	DG_LayoutView*      m_pLayoutView;
	
	UT_Vector		m_vecPages;
	UT_Vector		m_vecSectionLayouts;
};

#endif /* DOCLAYOUT_H */
