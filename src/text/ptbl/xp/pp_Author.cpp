/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 2009 Martin Sevior <msevior@gmail.com>
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

#include "pp_Author.h"
#include "pp_AttrProp.h"
#include "pd_Style.h"
#include "pd_Document.h"
#include "ut_debugmsg.h"
#include "ut_misc.h"

pp_Author::pp_Author(PD_Document * pDoc, UT_sint32 iID):
  m_pDoc(pDoc), m_iAuthorInt(iID) 
{
}

pp_Author::~pp_Author(void)
{
}

PP_AttrProp *      pp_Author::getAttrProp(void)
{ 
    return & m_AP;
}

const PP_AttrProp * pp_Author::getAttrProp(void) const
{
  return & m_AP;
}
  
bool pp_Author::getProperty(const gchar * szName, const gchar *& szValue) const
{ 
  return  m_AP.getProperty(szName,szValue);
}

UT_sint32 pp_Author::getAuthorInt(void) const
{ 
  return m_iAuthorInt;
}
