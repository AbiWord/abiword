 
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

#ifndef PL_LISTENER_H
#define PL_LISTENER_H

#include "ut_types.h"
#include "pt_Types.h"
class PX_ChangeRecord;


// PL_Listener -- A layout registers a listener with the
//                PD_Document in order to be notified of
//                any changes to the document as they
//                occur.  The document will notify each
//                registered listener (in an undefined
//                order).  When the listener registers,
//                it is provided an ID which may be used
//                later to refer to it.

class PL_Listener
{
public:
	// when a listener is installed, the document calls the listener
	// for each fragment in the document.  this allows the layout to
	// fully populate its representation of the formatting/layout of
	// the document.
	//
	// when the document changes (due to editing), the document
	// will call each listener and notify them.  the (fmt) structure
	// handle passed represents the either the structure being
	// modified or the containing structure block.  if the change
	// is an insertStrux, the listener can return a handle for the
	// new structure.
	//
	// the change notification occurs after the document has
	// been updated and in a stable state.  (for an editing operation
	// that breaks down into multiple change records, the listeners
	// will be called after each step.)
	//
	// sdh represents a handle to either the structure or the
	// containing structure block (opaque document instance data).
	//
	// pcr contains a change record to indicate what was done to
	// the document (in the case of editing).  pcr contains a faked-up
	// change record describing a fragment of the document (when part
	// of the listener installation sequence).
	//
	// sfh,psfh refer to a layout handle (opaque layout instance data)
	// to correspond with sdh.
	//
	// insertStrux() is a special form of change() which allows
	// instance data to be exchanged for a new strux.  in this case,
	// sfh refers to the strux preceeding the new one.

	virtual UT_Bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr) = 0;

	virtual UT_Bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh) = 0;

	virtual UT_Bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr) = 0;

	virtual UT_Bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdhNew,
									PL_StruxFmtHandle * psfhNew) = 0;
};
	
#endif /* PL_LISTENER_H */
