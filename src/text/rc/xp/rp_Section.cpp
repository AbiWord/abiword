/* AbiWord
 * Copyright (C) 2004 Marc Maurer
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

#include "pt_Types.h"
#include "rp_Object.h"
#include "rp_Section.h"
#include "rl_DocLayout.h"
#include "fv_View.h"

rp_Section::rp_Section(rl_Layout* pLayout) :
	rp_ContainerObject(pLayout)
{
	const PP_AttrProp* pAP = NULL;
	PP_RevisionAttr* pRevisions = NULL;
	bool bShowRevisions = false;
	UT_uint32 iId = static_cast<FV_View*>(pLayout->getDocLayout()->getAvView())->getRevisionLevel();
	bool bHiddenRevision = false;
	
	pLayout->getAttrProp(&pAP, pRevisions, bShowRevisions, iId, bHiddenRevision);
	DELETEP(pRevisions); // we don't need it
	UT_return_if_fail(pAP);

	UT_UCS4String* tag = new UT_UCS4String("Section");
	
	// add the attributes to the tag
	const XML_Char ** attribs = pAP->getAttributes();
	if (attribs)
	{
		UT_uint32 i = 0;
		while (attribs[i])
		{
			UT_UCS4String name(static_cast<const char *>(attribs[i]));
			UT_UCS4String value(static_cast<const char *>(attribs[i+1]));
			*tag += UT_UCS4String(" ");
			*tag += name;
			*tag += UT_UCS4String("=");
			*tag += value;
			i+=2;
		}
	}
	
	// add the props to the tag
	const XML_Char ** props = pAP->getProperties();
	if (props)
	{
		UT_uint32 i = 0;
		while (props[i])
		{
			UT_UCS4String name(static_cast<const char *>(props[i]));
			
			UT_UCS4String value(static_cast<const char *>(props[i+1]));
			*tag += UT_UCS4String(" ");
			*tag += name;
			*tag += UT_UCS4String("=");
			*tag += value;
			i+=2;
		}
	}
	
	setText(tag);	
	
	tag = new UT_UCS4String("Section");
	_setCloseTagText(tag);
}
