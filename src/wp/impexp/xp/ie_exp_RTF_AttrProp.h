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


#ifndef IE_EXP_RTF_ATTRPROP_H
#define IE_EXP_RTF_ATTRPROP_H

#include "pt_Types.h"
class PD_Document;
class PD_Style;
class PP_AttrProp;

/*!
 * This is an adapter interface. Some of the RTF export functions
 * are used for exporting both style sheets and document contents.
 * Styles and document elements use different ways of looking
 * up property values, but we don't care about that.
 * 
 * s_RTF_AttrPropAdapter is an abstract class for accessing properties.
 * The function getProperty takes a property name as argument
 * and returns the value, or NULL if that property isn't defined.
 */

class s_RTF_AttrPropAdapter
{
public:
    // TODO: remove 'getAttribute'. I don't think it's being used and it's
    //       not even implemented for the _AP version.
    virtual const XML_Char * getAttribute(const XML_Char * szName) const = 0;

    virtual const XML_Char * getProperty(const XML_Char * szName) const = 0;
};

class s_RTF_AttrPropAdapter_Style : public s_RTF_AttrPropAdapter
{
private:
    const PD_Style * pStyle;

public:
    s_RTF_AttrPropAdapter_Style(const PD_Style * pStyle) : pStyle(pStyle) {}

    virtual const XML_Char * getAttribute(const XML_Char * szName) const;
    virtual const XML_Char * getProperty(const XML_Char * szName) const;
};

class s_RTF_AttrPropAdapter_AP : public s_RTF_AttrPropAdapter
{
private:
    const PP_AttrProp * pSpanAP;
    const PP_AttrProp * pBlockAP;
    const PP_AttrProp * pSectionAP;
    PD_Document * pDoc;

public:
    s_RTF_AttrPropAdapter_AP(const PP_AttrProp * pSpanAP,
			     const PP_AttrProp * pBlockAP,
			     const PP_AttrProp * pSectionAP,
			     PD_Document * pDoc) : 
	pSpanAP(pSpanAP), pBlockAP(pBlockAP), pSectionAP(pSectionAP),
	pDoc(pDoc) {}

    virtual const XML_Char * getAttribute(const XML_Char * szName) const;
    virtual const XML_Char * getProperty(const XML_Char * szName) const;
};

#endif
