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

#ifndef _OXML_ELEMENT_H_
#define _OXML_ELEMENT_H_

// Internal includes
#include <OXML_Types.h>
#include <OXML_ObjectWithAttrProp.h>
#include "OXML_Style.h"
#include "OXML_List.h"
#include "OXML_Image.h"

// AbiWord includes
#include <ut_types.h>
#include <pp_AttrProp.h>
#include <pd_Document.h>

// External includes
#include <string>
#include <vector>
#include <memory>
#include <map>

class OXML_Element;
class IE_Exp_OpenXML;

typedef std::shared_ptr<OXML_Element> OXML_SharedElement;
typedef std::vector<OXML_SharedElement> OXML_ElementVector;

/* \class OXML_Element
 * \brief This class represents any element in the OpenXML data model.
 * OXML_Element can represent any tag, whether it is block-level or character-level.
 * The tags it can represent are listed in the ElementTag enum of OXML_Types.h.
 * If additional logic is required for a specific tag, this class can be derived
 * (see the documentation on the virtual methods in this case).
*/
class OXML_Element : public OXML_ObjectWithAttrProp
{
public:
	/*!
	 * \param id String representing the id (should be unique, although no check is made)
	 * \param tag The tag that this element represents.
	 * \param The category (BLOCK or SPAN) that this element belongs to.
	 */
	OXML_Element(const std::string & id, OXML_ElementTag tag, OXML_ElementType type);
	virtual ~OXML_Element();

	inline const std::string & getId() const { return m_id; }
	inline void setId(const std::string & id) { m_id = id; }
	inline OXML_ElementTag getTag() const { return m_tag; }
	inline OXML_ElementType getType() const { return m_type; }
	inline void setType(OXML_ElementType type) { m_type = type; }

	bool operator ==(const std::string & id);
	friend bool operator ==(const OXML_SharedElement& lhs, const std::string & id) { return (*lhs) == id; }

	OXML_SharedElement getElement(const std::string & id) const;
	UT_Error appendElement(const OXML_SharedElement & obj);
	inline const OXML_ElementVector & getChildren() const { return m_children; }
	UT_Error clearChildren();

	//! Writes the OpenXML element to a file on disk.
	/*! This method is used during the export process.
	 *  WARNING: If you derive OXML_Element, you should probably override this method.
	 *  If you do, make sure to call the method serializeChildren (if applicable).
		\param exporter the actual exporter which handles writing the files.
	*/
	virtual UT_Error serialize(IE_Exp_OpenXML* exporter);
	//! Appends this section and all its content to the Abiword Piecetable.
	/*! This method is used during the import process.
	 *  WARNING: If you derive OXML_Element, you should probably override this method.
	 *  If you do, make sure to call the method addChildrenToPT (if applicable).
		\param pDocument A valid reference to the PD_Document object.
	*/
	virtual UT_Error addToPT(PD_Document * pDocument);
	//! Calls the method addToPT() on all children.
	/*! WARNING: if you derive OXML_Element, you probably shouldn't redefine this method.
	 */
	UT_Error addChildrenToPT(PD_Document * pDocument);

	void setTarget(int target);

protected:
	//! Calls the method serialize() on all children.
	/*! WARNING: if you derive OXML_Element, you probably shouldn't redefine this method.
	 */
	UT_Error serializeChildren(IE_Exp_OpenXML* exporter);

	int TARGET;

private:
	std::string m_id;
	OXML_ElementTag m_tag;
	OXML_ElementType m_type;
	OXML_ElementVector m_children;

};

typedef std::map<std::string, OXML_SharedStyle > OXML_StyleMap;
typedef std::map<UT_uint32, OXML_SharedList > OXML_ListMap;
typedef std::map<std::string, OXML_SharedImage > OXML_ImageMap;

#endif //_OXML_ELEMENT_H_

