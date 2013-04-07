/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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

// Class definition include
#include <OXML_Element.h>

// Internal includes
#include <OXML_Types.h>

// AbiWord includes
#include <ut_types.h>
#include <pd_Document.h>
#include <pt_Types.h>

// External includes
#include <string>

OXML_Element::OXML_Element(const std::string & id, OXML_ElementTag tag, OXML_ElementType type) : 
	OXML_ObjectWithAttrProp(),
	TARGET(0), 
	m_id(id), 
	m_tag(tag), 
	m_type(type)
{
}

OXML_Element::~OXML_Element()
{
	this->clearChildren();
}

bool OXML_Element::operator ==(const std::string & id)
{
	return this->m_id.compare(id) == 0;
}

OXML_SharedElement OXML_Element::getElement(const std::string & id) const
{
	OXML_ElementVector::const_iterator it;
	it = std::find(m_children.begin(), m_children.end(), id);
	return ( it != m_children.end() ) ? (*it) : OXML_SharedElement() ;
}

UT_Error OXML_Element::appendElement(const OXML_SharedElement & obj)
{
	UT_return_val_if_fail(obj.get() != NULL, UT_ERROR);

	try {
		m_children.push_back(obj);
	} catch(...) {
		UT_DEBUGMSG(("Bad alloc!\n"));
		return UT_OUTOFMEM;
	}

	obj->setTarget(TARGET); //propagate the target

	return UT_OK;
}

UT_Error OXML_Element::clearChildren()
{
	m_children.clear();
	return m_children.size() == 0 ? UT_OK : UT_ERROR;
}

UT_Error OXML_Element::serializeChildren(IE_Exp_OpenXML* exporter)
{
	UT_Error ret = UT_OK;

	OXML_ElementVector::size_type i;
	for (i = 0; i < m_children.size(); i++)
	{
		ret = m_children[i]->serialize(exporter);
		if(ret != UT_OK)
			return ret;
	}

	return ret;
}

UT_Error OXML_Element::serialize(IE_Exp_OpenXML* exporter)
{
	UT_Error ret = UT_OK;
	//Do something here when export filter is implemented

	if (ret != UT_OK)
		return ret;	

	return serializeChildren(exporter);
}

UT_Error OXML_Element::addChildrenToPT(PD_Document * pDocument)
{
	UT_Error ret(UT_OK), temp(UT_OK);

	OXML_ElementVector::size_type i;
	for (i = 0; i < m_children.size(); i++)
	{
		temp = m_children[i]->addToPT(pDocument);
		if (temp != UT_OK)
			ret = temp;
	}
	return ret;
}

UT_Error OXML_Element::addToPT(PD_Document * pDocument)
{
	UT_Error ret = UT_OK;

	if (pDocument == NULL)
		return UT_ERROR;

	//	const gchar ** atts = getAttributesWithProps();

	switch (m_tag) {
	case PG_BREAK:
	{
		UT_UCSChar ucs = UCS_FF;
		ret = pDocument->appendSpan(&ucs, 1) ? UT_OK : UT_ERROR;
		UT_return_val_if_fail(ret == UT_OK, ret);
	}
		break;
	case CL_BREAK:
	{
		UT_UCSChar ucs = UCS_VTAB;
		ret = pDocument->appendSpan(&ucs, 1) ? UT_OK : UT_ERROR;
		UT_return_val_if_fail(ret == UT_OK, ret);
	}
		break;
	
	case LN_BREAK:
	{
		UT_UCSChar ucs = UCS_LF;
		ret = pDocument->appendSpan(&ucs, 1) ? UT_OK : UT_ERROR;
		UT_return_val_if_fail(ret == UT_OK, ret);
	}
		break;

	case P_TAG: //fall through to default
	case R_TAG: //fall through to default
	case T_TAG: //fall through to default
	default:
		UT_ASSERT_NOT_REACHED(); //We really shouldn't get here.
		break;
	}

	ret = addChildrenToPT(pDocument);
	return ret;
}

void OXML_Element::setTarget(int target)
{
	TARGET = target;
}
