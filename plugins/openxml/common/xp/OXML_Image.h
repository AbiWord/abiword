/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2008 Firat Kiyak <firatkiyak@gmail.com>
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

#ifndef _OXML_IMAGE_H_
#define _OXML_IMAGE_H_

// Internal includes
#include <OXML_Types.h>
#include <OXML_ObjectWithAttrProp.h>

// AbiWord includes
#include <ut_types.h>
#include <pd_Document.h>

// External includes
#include <string>
#include <boost/shared_ptr.hpp>

class IE_Exp_OpenXML;

class OXML_Image : public OXML_ObjectWithAttrProp
{

public:
	OXML_Image();
	virtual ~OXML_Image();

	virtual void setId(const char* id);
	virtual void setMimeType(const char* mimeType);
	virtual void setData(const UT_ByteBuf* data);

	virtual const char* getId();

	UT_Error serialize(IE_Exp_OpenXML* exporter);
	UT_Error addToPT(PD_Document * pDocument);

private:
	std::string m_id;
	std::string m_mimeType;
	const UT_ByteBuf* m_data;
};

typedef boost::shared_ptr<OXML_Image> OXML_SharedImage;

#endif //_OXML_IMAGE_H_

