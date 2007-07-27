/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2007 Martin Sevior
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


#include "fp_Run.h"
#include "fl_BlockLayout.h"
#include "ut_debugmsg.h"
#include "pd_Document.h"
#include <string.h>

fp_AnnotationRun::fp_AnnotationRun(fl_BlockLayout* pBL,
				   UT_uint32 iOffsetFirst, 
				   UT_uint32 iLen ) : 
  fp_HyperlinkRun(pBL,iOffsetFirst,1),m_iPID(0)
{
    UT_ASSERT(pBL);
	_setLength(1);
	_setDirty(false);
	_setWidth(0);
	_setRecalcWidth(false);
	
	UT_ASSERT((pBL));
	_setDirection(UT_BIDI_WS);

	const PP_AttrProp * pAP = NULL;

	getSpanAP(pAP);
	
	const gchar * pTarget;
	const gchar * pName;
	bool bFound = false;
	UT_uint32 k = 0;

	while(pAP->getNthAttribute(k++, pName, pTarget))
	{
		bFound = (0 == g_ascii_strncasecmp(pName,"Annotation",10));
		if(bFound)
			break;
	}

	// we have got to keep a local copy, since the pointer we get
	// is to a potentially volatile location
	if(bFound)
	{
	        if(m_pTarget)
		{
		    delete [] m_pTarget;
		}
		UT_uint32 iTargetLen = strlen(pTarget);
		m_pTarget = new gchar [iTargetLen + 1];
		strncpy(m_pTarget, pTarget, iTargetLen + 1);
		m_bIsStart = true;
		//if this is a start of the Annotation, we set m_pHyperlink to this,
		//so that when a run gets inserted after this one, its m_pHyperlink is
		//set correctly
		_setHyperlink(this);
		m_iPID = atoi(m_pTarget);
	}
	else
	{
		m_bIsStart = false;
		m_pTarget = NULL;
		_setHyperlink(NULL);
		m_iPID =0;
	}

}


fp_AnnotationRun::~fp_AnnotationRun()
{
	if(m_pTarget)
		delete [] m_pTarget;
	m_pTarget = NULL;
}
