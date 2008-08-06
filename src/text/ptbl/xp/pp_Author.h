/* AbiWord
 * Copyright (C) 2008 Martin Sevior <msevior@gmail.com>
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

#ifndef PT_AUTHOR_H
#define PT_AUTHOR_H

#include "ut_types.h"
#include "ut_string_class.h"
#include "ut_vector.h"
#include "pp_AttrProp.h"

class PD_Document;

class ABI_EXPORT pp_Author
{
  public:
  pp_Author(PD_Document * pDoc, const gchar * szUUID, UT_sint32 iID):
  m_pDoc(pDoc), m_sUUID(szUUID), m_iAuthorInt(iID) {};
  virtual ~pp_Author(){};

  PP_AttrProp *      getAttrProp(void)
  { return & m_AP;}

  bool      getProperty(const gchar * szName, const gchar *& szValue)
  { return  m_AP.getProperty(szName,szValue);}

  const gchar *      getUUID(void)
  { return static_cast<const gchar *>(m_sUUID.utf8_str());}

  UT_sint32          getAuthorInt(void)
  { return m_iAuthorInt;}

  private:
  PD_Document *     m_pDoc;
  UT_UTF8String     m_sUUID;
  UT_sint32         m_iAuthorInt;
  PP_AttrProp       m_AP;
};

#endif // #ifndef PT_AUTHOR_H
