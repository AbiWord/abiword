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


#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "pt_VarSet.h"
#include "pd_Style.h"

pt_VarSet::pt_VarSet()
{
	m_currentVarSet = 0;
	m_bInitialized = false;
}

pt_VarSet::~pt_VarSet()
{
}

bool pt_VarSet::_finishConstruction(void)
{
	// finish the construction -- C++ doesn't let us return failures
	// in a constructor, so we do the malloc's here.
		
	// create a default A/P as entry zero in each AP table.

	PT_AttrPropIndex foo;
		
	if (   !m_tableAttrProp[0].createAP(&foo)
		|| !m_tableAttrProp[1].createAP(&foo))
		return false;
	((PP_AttrProp *)getAP(_makeAPIndex(0,0)))->markReadOnly();
	((PP_AttrProp *)getAP(_makeAPIndex(1,0)))->markReadOnly();
	
	m_bInitialized = true;
	return true;
}

void pt_VarSet::setPieceTableState(PTState pts)
{
	if (pts == PTS_Editing)
		m_currentVarSet = 1;
}

bool pt_VarSet::appendBuf(const UT_UCSChar * pBuf, UT_uint32 length, PT_BufIndex * pbi)
{
	UT_uint32 bufOffset = m_buffer[m_currentVarSet].getLength();
	if (m_buffer[m_currentVarSet].ins(bufOffset,pBuf,length))
	{
		*pbi = _makeBufIndex(m_currentVarSet,bufOffset);
		return true;
	}

	UT_DEBUGMSG(("could not appendBuf\n"));
	return false;
}

bool pt_VarSet::overwriteBuf(UT_UCSChar * pBuf, UT_uint32 length, PT_BufIndex * pbi)
{
	if (m_buffer[_varsetFromBufIndex(*pbi)]
        .overwrite(_subscriptFromBufIndex(*pbi),
                   pBuf,
                   length))
	{
		return true;
	}

	UT_DEBUGMSG(("could not overwriteBuf\n"));
	return false;
}



bool pt_VarSet::storeAP(const XML_Char ** attributes, PT_AttrPropIndex * papi)
{
	if (!m_bInitialized)
		if (!_finishConstruction())
			return false;

	// create an AP for this set of attributes -- iff unique.
	// return the index for the new one (or the one we found).

	if (!attributes || !*attributes)
	{
		// we preloaded the zeroth cell (of both tables)
		// with empty attributes.  return index back to
		// the first table.
		*papi = _makeAPIndex(0,0);
		return true;
	}

	// This is a bit expensive, but it can't be helped.
	// We synthesize a (possibly temporary) AP to organize
	// and store the values given in the args.  We then use
	// it to find one that matches it.  If we find it, we
	// destroy the one we just created.  Otherwise, we add
	// the new one to the correct table and return it.

	PP_AttrProp * pTemp = new PP_AttrProp();
	if (!pTemp)
		return false;

	if (pTemp->setAttributes(attributes))
	{
		pTemp->markReadOnly();
		return addIfUniqueAP(pTemp,papi);
	}
	
	delete pTemp;
	return false;
}

bool pt_VarSet::storeAP(const UT_Vector * pVecAttributes, PT_AttrPropIndex * papi)
{
	if (!m_bInitialized)
		if (!_finishConstruction())
			return false;

	// create an AP for this set of attributes -- iff unique.
	// return the index for the new one (or the one we found).
	
	if (!pVecAttributes || pVecAttributes->getItemCount()==0)
	{
		// we preloaded the zeroth cell (of both tables)
		// with empty attributes.  return index back to
		// the first table.
		*papi = _makeAPIndex(0,0);
		return true;
	}

	// This is a bit expensive, but it can't be helped.
	// We synthesize a (possibly temporary) AP to organize
	// and store the values given in the args.  We then use
	// it to find one that matches it.  If we find it, we
	// destroy the one we just created.  Otherwise, we add
	// the new one to the correct table and return it.

	PP_AttrProp * pTemp = new PP_AttrProp();
	if (!pTemp)
		return false;
	
	if (pTemp->setAttributes(pVecAttributes))
	{
		pTemp->markReadOnly();
		return addIfUniqueAP(pTemp,papi);
	}

	delete pTemp;
	return false;
}

bool pt_VarSet::isContiguous(PT_BufIndex bi, UT_uint32 length, PT_BufIndex bi2) const
{
	return ((getPointer(bi)+length) == getPointer(bi2));
}

bool pt_VarSet::mergeAP(PTChangeFmt ptc, PT_AttrPropIndex apiOld,
						   const XML_Char ** attributes, const XML_Char ** properties,
						   PT_AttrPropIndex * papiNew,PD_Document * pDoc)
{
	// merge the given attr/props with set referenced by apiOld
	// under the operator ptc giving a new set to be returned in
	// papiNew.  if the resulting merger is the same as the set
	// referenced in apiOld, just return it.
	// return false only if we had an error.

	const PP_AttrProp * papOld = getAP(apiOld);
	switch (ptc)
	{
	case PTC_AddFmt:
		{
			if (papOld->areAlreadyPresent(attributes,properties))
			{
				*papiNew = apiOld;
				return true;
			}

			// create a new AP that is the merger of the existing
			// one and the a/p given in the args.  we then use it
			// to find an existing match or use the new one.
			
			PP_AttrProp * pNew = papOld->cloneWithReplacements(attributes,properties, false);
			if (!pNew)
				return false;

			pNew->markReadOnly();
			return addIfUniqueAP(pNew,papiNew);
		}

	case PTC_AddStyle:
		{
			if (!papOld->hasProperties() && 
				papOld->areAlreadyPresent(attributes,properties))
			{
				*papiNew = apiOld;
				return true;
			}

			// create a new AP that is the merger of the existing
			// one (with its props cleared) and the a/p given in 
			// the args.  we then use it to find an existing match 
			// or use the new one.
//
// This is a bit tricky because we want to replace any attributes with one's
// from our style but we will remove any explict defined properties that are 
// contained in the style. So first clonewithreplace then cloneWithElmination
// This code assumes that the properties passed in the PTC_AddStyle are the
// properties from the fully expanded style.
//

// TODO this is not right; we first have to remove any properties that we got
// from the current style if any, only then we can proceed
#if 1
		const XML_Char * szStyle;
		const XML_Char * szSName;
		bool bFound = false;

		//there is something wrong with getAttribute. What? Who wrote this???  - Martin

		//      papOld->getAttribute(PT_STYLE_ATTRIBUTE_NAME, szStyle);
		for(UT_uint32 k = 0; k < papOld->getAttributeCount();k++)
		{
			papOld->getNthAttribute(k, szSName, szStyle);
			if(!UT_strcmp(PT_STYLE_ATTRIBUTE_NAME, szSName))
			{
				bFound = true;
				break;
			}
		}
		

		PP_AttrProp * pNew1 = NULL;
		PD_Style * pStyle = NULL;

        if(bFound && szStyle && UT_strcmp(szStyle, "None"))
        {
	        UT_DEBUGMSG(("current style [%s]\n",szStyle));
			pDoc->getStyle(szStyle,&pStyle);
		}

		if (!pStyle)
		{
			UT_DEBUGMSG(("oops! tried to change from a nonexistent style!\n"));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}

        if(bFound && szStyle && UT_strcmp(szStyle, "None") && pStyle)
		{
			// first of all deal with list-attributes if the new style is not
			// a list style and the old style is a list style
			if(pStyle->isList())
			{
				UT_DEBUGMSG(("old style is a list style\n"));
				// OK, old style is a list, is the new style?
				// (the following function cares not whether we are dealing
				//  with attributes or properties)
				const XML_Char * pNewStyle = UT_getAttribute("list-style", properties);
				
				// we do not care about the value, just about whether it is there
				if(!pNewStyle)
				{
					UT_DEBUGMSG(("new style is not a list style\n"));
					
					const XML_Char * pListAttrs[8];
					pListAttrs[0] = "listid";
					pListAttrs[1] = NULL;
					pListAttrs[2] = "parentid";
					pListAttrs[3] = NULL;
					pListAttrs[4] = "level";
					pListAttrs[5] = NULL;
					pListAttrs[6] = NULL;
					pListAttrs[7] = NULL;

					// we also need to explicitely clear the list formating
					// properties, since their values are not necessarily part
					// of the style definition, so that cloneWithEliminationIfEqual
					// which we call later will not get rid off them
					const XML_Char * pListProps[20];
					pListProps[0] =  "start-value";
					pListProps[1] =  NULL;
					pListProps[2] =  "list-style";
					pListProps[3] =  NULL;
					pListProps[4] =  "margin-left";
					pListProps[5] =  NULL;
					pListProps[6] =  "text-indent";
					pListProps[7] =  NULL;
					pListProps[8] =  "field-color";
					pListProps[9] =  NULL;
					pListProps[10]=  "list-delim";
					pListProps[11] =  NULL;
					pListProps[12]=  "field-font";
					pListProps[13] =  NULL;
					pListProps[14]=  "list-decimal";
					pListProps[15] =  NULL;
					pListProps[16] =  "list-tag";
					pListProps[17] =  NULL;
					pListProps[18] =  NULL;
					pListProps[19] =  NULL;
					
					pNew1 = papOld->cloneWithElimination((const XML_Char **)&pListAttrs, (const XML_Char **)&pListProps);
				}
			}
			
			UT_Vector vProps, vAttribs;
		
			pStyle->getAllProperties(&vProps,0);
		
			const XML_Char ** sProps = NULL;
			UT_uint32 countp = vProps.getItemCount() + 1;
			sProps = new const XML_Char*[countp];
			countp--;
			UT_uint32 i;
			for(i=0; i<countp; i++)
			{
				sProps[i] = (const XML_Char *) vProps.getNthItem(i);
			}
			sProps[i] = NULL;
		
			
			pStyle->getAllAttributes(&vAttribs,0);
			
			const XML_Char ** sAttribs = NULL;
			countp = vAttribs.getItemCount() + 1;
			sAttribs = new const XML_Char*[countp];
			countp--;
			
			for(i=0; i<countp; i++)
			{
				sAttribs[i] = (const XML_Char *) vAttribs.getNthItem(i);
			}
			sAttribs[i] = NULL;
		
			PP_AttrProp * pNew0;
			
			if(pNew1)
			{
				pNew0 = pNew1->cloneWithEliminationIfEqual(sAttribs,sProps);
				delete pNew1;
			}
			else
			 	pNew0 = papOld->cloneWithEliminationIfEqual(sAttribs,sProps);
			 	
			delete [] sProps;
			delete [] sAttribs;
			
			if (!pNew0)
				return false;

			pNew1 = pNew0->cloneWithReplacements(attributes,NULL, false);
			delete pNew0;
			if (!pNew1)
				return false;
		}
		else
		{
			pNew1 = papOld->cloneWithReplacements(attributes,NULL, false);
			if (!pNew1)
				return false;
		
		}
		
		PP_AttrProp * pNew = pNew1->cloneWithElimination(NULL,properties);
		delete pNew1;
		if (!pNew)
			return false;
					
#else
			PP_AttrProp * pNew1 = papOld->cloneWithReplacements(attributes,NULL, false);
			if (!pNew1)
				return false;
			PP_AttrProp * pNew = pNew1->cloneWithElimination(NULL,properties);
			delete pNew1;
			if (!pNew)
				return false;
#endif
			pNew->markReadOnly();
			return addIfUniqueAP(pNew,papiNew);
		}

	case PTC_RemoveFmt:
		{
			if (!papOld->areAnyOfTheseNamesPresent(attributes,properties))
			{
				// none of the given attributes/properties are present in this set.
				// the remove has no effect.
				*papiNew = apiOld;
				return true;
			}

			// create a new AP that is the existing set minus the
			// ones given in the args.  we then use it to find an
			// existing match or use the new one.

			PP_AttrProp * pNew = papOld->cloneWithElimination(attributes,properties);
			if (!pNew)
				return false;

			pNew->markReadOnly();
			return addIfUniqueAP(pNew,papiNew);
		}
		
	default:
		UT_ASSERT(0);
		return false;
	}
}

bool pt_VarSet::addIfUniqueAP(PP_AttrProp * pAP, PT_AttrPropIndex * papi)
{
	// Add the AP to our tables iff it is unique.
	// If not unique, delete it and return the index
	// of the one that matches.  If it is unique, add
	// it and return the index where we added it.
	// return false if we have any errors.

	UT_ASSERT(pAP && papi);
	UT_uint32 subscript = 0;
	UT_uint32 table = 0;
	
	for (table=0; table<2; table++)
		if (m_tableAttrProp[table].findMatch(pAP,&subscript))
		{
			// use the one that we already have in the table.
			delete pAP;
			*papi = _makeAPIndex(table,subscript);
			return true;
		}

	// we did not find a match, so we store our new one.
	
	if (m_tableAttrProp[m_currentVarSet].addAP(pAP,&subscript))
	{
		*papi = _makeAPIndex(m_currentVarSet,subscript);
		return true;
	}
	
	// memory error of some kind.
	
	delete pAP;
	return false;
}
