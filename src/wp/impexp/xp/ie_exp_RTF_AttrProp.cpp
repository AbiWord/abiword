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

#include "pd_Style.h"
#include "pp_Property.h"
#include "ie_exp_RTF_AttrProp.h"

const XML_Char * s_RTF_AttrPropAdapter_Style::getAttribute(const XML_Char * szName) const 
{
    const XML_Char * szValue = 0;
    pStyle->getAttribute(szName, szValue);
    return szValue;
}

const XML_Char * s_RTF_AttrPropAdapter_Style::getProperty(const XML_Char * szName) const 
{
    const XML_Char * szValue = 0;
    pStyle->getProperty(szName, szValue);
    return szValue;
}

const XML_Char * s_RTF_AttrPropAdapter_AP::getAttribute(const XML_Char * szName) const
{
    return 0;
}

const XML_Char * s_RTF_AttrPropAdapter_AP::getProperty(const XML_Char * szName) const 
{
    return PP_evalProperty(szName, pSpanAP, pBlockAP, pSectionAP, 
			   pDoc, true);
}

