/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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
#include "pd_Document.h"

/*!
 * This class is used to store and manipulate collections of Attributes
 * /Properties.
 *
 * Amongst it purposes this class:
 * 1. Provides a integer index to every unique Attribute/Property collection.
 * 2. (addFmt) Merges additional Attributes and properties into an existing
 * collection of attributes/properties.
 * 3. (removeFmt) Removes attributes/properties from an existing collection
 * of Attributes properties.
 */ 
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

	if (!m_tableAttrProp[0].createAP(NULL)
		|| !m_tableAttrProp[1].createAP(NULL))
		return false;

	m_bInitialized = true;
	return true;
}

void pt_VarSet::setPieceTableState(PTState pts)
{
	if (pts == PTS_Editing)
		m_currentVarSet = 1;
}

bool pt_VarSet::appendBuf(const UT_UCSChar * pBuf,
						  UT_uint32 length,
						  PT_BufIndex * pbi)
{
	UT_uint32 bufOffset = m_buffer[m_currentVarSet].getLength();
	if (m_buffer[m_currentVarSet].ins(bufOffset,
									  (UT_GrowBufElement*)pBuf,
									  length))
	{
		*pbi = _makeBufIndex(m_currentVarSet,bufOffset);
		return true;
	}

	UT_DEBUGMSG(("could not appendBuf\n"));
	return false;
}

bool pt_VarSet::overwriteBuf(UT_UCSChar * pBuf,
							 UT_uint32 length,
							 PT_BufIndex * pbi)
{
	if (m_buffer[_varsetFromBufIndex(*pbi)]
        .overwrite(_subscriptFromBufIndex(*pbi),
                   (UT_GrowBufElement*)pBuf,
                   length))
	{
		return true;
	}

	UT_DEBUGMSG(("could not overwriteBuf\n"));
	return false;
}



bool pt_VarSet::storeAP(const PT_AttributePair* attrs, PT_AttrPropIndex * papi)
{
	if (!m_bInitialized)
		if (!_finishConstruction())
			return false;

	// create an AP for this set of attributes -- iff unique.
	// return the index for the new one (or the one we found).

	if (!attrs)
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

	if (pTemp->setAttributes(attrs))
	{
		pTemp->markReadOnly();
		return addIfUniqueAP(pTemp,papi);
	}
	
	delete pTemp;
	return false;
}

bool pt_VarSet::storeAP(const PT_AttributeVector & vAttrs,
						PT_AttrPropIndex * papi)
{
	if (!m_bInitialized)
		if (!_finishConstruction())
			return false;

	// create an AP for this set of attributes -- iff unique.
	// return the index for the new one (or the one we found).
	
	if (vAttrs.getItemCount() == 0)
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
	
	if (pTemp->setAttributes(vAttrs))
	{
		pTemp->markReadOnly();
		return addIfUniqueAP(pTemp,papi);
	}

	delete pTemp;
	return false;
}

bool pt_VarSet::isContiguous(PT_BufIndex bi,
							 UT_uint32 length,
							 PT_BufIndex bi2) const
{
	return ((getPointer(bi)+length) == getPointer(bi2));
}

bool pt_VarSet::mergeAP(PTChangeFmt ptc,
						PT_AttrPropIndex apiOld,
						const PT_AttributePair * attrs,
						const PT_PropertyPair  * props,
						PT_AttrPropIndex * papiNew,
						PD_Document * pDoc)
{
	// merge the given attr/props with set referenced by apiOld
	// under the operator ptc giving a new set to be returned in
	// papiNew.  if the resulting merger is the same as the set
	// referenced in apiOld, just return it.
	// return false only if we had an error.

	const PP_AttrProp * papOld = getAP(apiOld);

	if (!papOld)
	  return false;

	switch (ptc)
	{
	case PTC_AddFmt:
		{
			if (papOld->areAlreadyPresent(attrs,props))
			{
				*papiNew = apiOld;
				return true;
			}

			// create a new AP that is the merger of the existing
			// one and the a/p given in the args.  we then use it
			// to find an existing match or use the new one.
			
			PP_AttrProp * pNew = papOld->cloneWithReplacements(attrs,props,
															   false);
			if (!pNew)
				return false;

			pNew->markReadOnly();
			return addIfUniqueAP(pNew,papiNew);
		}
	case PTC_SetFmt:
		{
			if (papOld->isEquivalent(attrs,props))
			{
				*papiNew = apiOld;
				return true;
			}

			// create a new AP that is exactly given by the atts/props
			// presented.
			
			PP_AttrProp * pNew = papOld->cloneWithReplacements(attrs,props,
															   true);
			if (!pNew)
				return false;

			pNew->markReadOnly();
			return addIfUniqueAP(pNew,papiNew);
		}
	case PTC_SetExactly:
		{
			if (papOld->isEquivalent(attrs,props))
			{
				*papiNew = apiOld;
				return true;
			}

			// create a new AP that is exactly given by the atts/props
			// presented.
			
			PP_AttrProp * pNew = papOld->createExactly(attrs, props);
			if (!pNew)
				return false;

			pNew->markReadOnly();
			return addIfUniqueAP(pNew,papiNew);
		}

	case PTC_AddStyle:
		{
			if (!papOld->hasProperties() && 
				papOld->areAlreadyPresent(attrs,props))
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
		GQuark value;
		bool bFound = papOld->getAttribute (PT_STYLE_ATTRIBUTE_NAME, value);

		PP_AttrProp * pNew1  = NULL;
		PD_Style    * pStyle = NULL;

        if(bFound && value && value != PP_commonValueQuark (PP_COMMON_None))
        {
	        UT_DEBUGMSG(("current style [%s]\n",g_quark_to_string (value)));
			pDoc->getStyle(value,&pStyle);
		}

		if (bFound && !pStyle)
		{
			UT_DEBUGMSG(("tried to change from a nonexistent style [%s]!\n",
						 g_quark_to_string (value)));
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}

        if(bFound && value &&
		   value != PP_commonValueQuark (PP_COMMON_None) &&
		   pStyle)
		{
			// first of all deal with list-attributes if the new style is not
			// a list style and the old style is a list style
			if(pStyle->isList())
			{
				UT_DEBUGMSG(("old style is a list style\n"));
				// OK, old style is a list, is the new style?
				// (the following function cares not whether we are dealing
				//  with attributes or properties)
				GQuark newStyle;

				if(properties)
					newStyle = UT_getProperty(abi_list_style, props);
				
				// we do not care about the value, just about whether it is there
				if(!newStyle)
				{
					UT_DEBUGMSG(("new style is not a list style\n"));

					PT_AttributePair listAttrs[4] = {
						{ PT_LISTID_ATTRIBUTE_NAME,   0},
						{ PT_PARENTID_ATTRIBUTE_NAME, 0},
						{ PT_LEVEL_ATTRIBUTE_NAME,    0},
						{ 0,    0},
					}
					
					// we also need to explicitely clear the list formating
					// properties, since their values are not necessarily part
					// of the style definition, so that cloneWithEliminationIfEqual
					// which we call later will not get rid off them
					const PT_PropertyPair * listProps[10] = {
						{ abi_start_value, 0},
						{ abi_list_style , 0},
						{ abi_margin-left, 0},
						{ abi_text_indent, 0},
						{ abi_field_color, 0},
						{ abi_list_delim, 0},
						{ abi_field_font, 0},
						{ abi_list_decimal, 0},
						{ abi_list_tag, 0},
						{ 0, 0},
					};
					
					pNew1 = papOld->cloneWithElimination(&listAttrs,
														 &listProps);
				}
			}

			/* TODO profile this to see if it would be worth providing
			 * clone methods taking vector input
			 */
			PT_PropertyVector  vProps;
			PT_AttributeVector vAttribs;
		
			pStyle->getAllProperties(&vProps,0);
		
			const XML_Char ** sProps = NULL;
			UT_uint32 countp = vProps.getItemCount();
			sProps = new const XML_Char*[countp + 1];
			UT_uint32 i;
			for(i=0; i<countp; i++)
			{
				sProps[i] = vProps.getNthItem(i);
			}
			sProps[i] = NULL;
		
			
			pStyle->getAllAttributes(&vAttribs,0);
			
			const XML_Char ** sAttribs = NULL;
			countp = vAttribs.getItemCount();
			sAttribs = new const XML_Char*[countp + 1];
			
			for(i=0; i<countp; i++)
			{
				sAttribs[i] = vAttribs.getNthItem(i);
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

			pNew1 = pNew0->cloneWithReplacements(attrs,NULL, false);
			delete pNew0;
			if (!pNew1)
				return false;
		}
		else
		{
			pNew1 = papOld->cloneWithReplacements(attrs,NULL, false);
			if (!pNew1)
				return false;
		
		}
		
		PP_AttrProp * pNew = pNew1->cloneWithElimination(NULL,props);
		delete pNew1;
		if (!pNew)
			return false;
					
#else
			PP_AttrProp * pNew1 = papOld->cloneWithReplacements(attrs,NULL, false);
			if (!pNew1)
				return false;
			PP_AttrProp * pNew = pNew1->cloneWithElimination(NULL,propers);
			delete pNew1;
			if (!pNew)
				return false;
#endif
			pNew->markReadOnly();
			return addIfUniqueAP(pNew,papiNew);
		}

	case PTC_RemoveFmt:
		{
			if (!papOld->areAnyOfTheseNamesPresent(attrs,props))
			{
				// none of the given attributes/properties are present in this set.
				// the remove has no effect.
				*papiNew = apiOld;
				return true;
			}

			// create a new AP that is the existing set minus the
			// ones given in the args.  we then use it to find an
			// existing match or use the new one.

			PP_AttrProp * pNew = papOld->cloneWithElimination(attrs,props);
			if (!pNew)
				return false;

			pNew->markReadOnly();
			return addIfUniqueAP(pNew,papiNew);
		}
		
	default:
		UT_ASSERT_HARMLESS(0);
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

	UT_return_val_if_fail (pAP && papi, false);
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

	// we did not find a match or we're loading a document , so we store our new one.

	if (m_tableAttrProp[m_currentVarSet].addAP(pAP,&subscript))
	{
		*papi = _makeAPIndex(m_currentVarSet,subscript);
		return true;
	}
	
	// memory error of some kind.
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	delete pAP;
	return false;
}
