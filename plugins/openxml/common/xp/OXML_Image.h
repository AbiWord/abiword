/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiSource
 *
 * Copyright (C) 2008 Firat Kiyak <firatkiyak@gmail.com>
 * Copyright (C) 2009 Hubert Figuiere
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

#ifndef _OXML_IMAGE_H_
#define _OXML_IMAGE_H_

#include <memory>
#include <string>

#include "ut_types.h"
#include "fg_Graphic.h"
#include "pd_Document.h"

#include "OXML_Types.h"
#include "OXML_ObjectWithAttrProp.h"


class IE_Exp_OpenXML;
class FG_Graphic;

class OXML_Image
    : public OXML_ObjectWithAttrProp
{

public:
	OXML_Image();
	virtual ~OXML_Image();

	void setId(const std::string & id);
	void setMimeType(const std::string & mimeType);
	void setData(const UT_ConstByteBufPtr & data);
	void setGraphic(FG_ConstGraphicPtr && fg);

	const std::string & getId() const
        {
            return m_id;
        }

	UT_Error serialize(IE_Exp_OpenXML* exporter);
	UT_Error addToPT(PD_Document * pDocument);

private:
	std::string  m_id;
	std::string m_mimeType;
	UT_ConstByteBufPtr m_data;
	FG_ConstGraphicPtr m_graphic;
};

typedef std::shared_ptr<OXML_Image> OXML_SharedImage;

#endif //_OXML_IMAGE_H_

