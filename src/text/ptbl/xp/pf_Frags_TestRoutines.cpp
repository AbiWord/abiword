 
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

#ifdef PT_TEST

#include "ut_types.h"
#include "ut_test.h"
#include "ut_assert.h"
#include "pf_Fragments.h"
#include "pf_Frag.h"

/*****************************************************************/
/*****************************************************************/

void pf_Fragments::__dump(FILE * fp) const
{
	pf_Frag * p;

	for (p=m_pFirst; (p); p=p->getNext())
		p->__dump(fp);
}

#endif /* PT_TEST */
