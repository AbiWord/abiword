 
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
** The Original Code is AbiSource Utilities.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#ifndef UT_STACK_H
#define UT_STACK_H

#include "ut_types.h"
#include "ut_vector.h"

class UT_Stack
{
public:
	UT_Bool			push(void * pVoid);
	UT_Bool			pop(void ** ppVoid);
	UT_Bool			viewTop(void ** ppVoid) const;
	UT_uint32		getDepth(void) const;

protected:
	UT_Vector		m_vecStack;
};

#endif /* UT_STACK_H */
