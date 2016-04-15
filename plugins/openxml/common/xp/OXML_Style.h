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

#ifndef _OXML_STYLE_H_
#define _OXML_STYLE_H_

// Internal includes
#include <OXML_Types.h>
#include <OXML_ObjectWithAttrProp.h>

// AbiWord includes
#include <ut_types.h>
#include <pd_Document.h>

// External includes
#include <memory>
#include <string>

class IE_Exp_OpenXML;

/* \class OXML_Style
 * \brief This class represents a single style in the OpenXML data model.
*/
class OXML_Style : public OXML_ObjectWithAttrProp
{
public:
	/*!
	    \param id The unique identifier for a valid OpenXML style.
	    \param name The unique identifier for a style in the AbiWord piecetable.
	 */
	OXML_Style(const std::string & id, const std::string & name);
	virtual ~OXML_Style();

	void setId(const std::string & id)
		{ m_id = id; }
	const std::string & getId() const
		{ return m_id; }
	void setName(const std::string & name)
		{ m_name = name; setAttribute(PT_NAME_ATTRIBUTE_NAME, name.c_str()); };
	const std::string & getName() const
		{ return m_name; }
	void setBasedOn(const std::string & basedOn)
		{ m_basedon = basedOn; }
	void setFollowedBy(const std::string & followedBy)
		{ m_followedby = followedBy; }

	UT_Error serialize(IE_Exp_OpenXML* exporter);
	UT_Error addToPT(PD_Document * pDocument);

private:
	std::string m_id;
	std::string m_name;
	std::string m_basedon;
	std::string m_followedby;
};

typedef std::shared_ptr<OXML_Style> OXML_SharedStyle;

#endif //_OXML_STYLE_H_

