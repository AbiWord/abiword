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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef PT_AUTHOR_H
#define PT_AUTHOR_H

#include "ut_types.h"
#include "pp_AttrProp.h"

class PD_Document;

class ABI_EXPORT pp_Author
{
public:
  pp_Author(UT_sint32 iID);
  virtual ~pp_Author();

  PP_AttrProp *      getAttrProp(void);
  const PP_AttrProp *      getAttrProp(void) const;

  bool      getProperty(const gchar * szName, const gchar *& szValue) const;
  UT_sint32          getAuthorInt(void) const;

private:
  UT_sint32         m_iAuthorInt;
  PP_AttrProp       m_AP;
};

#endif // #ifndef PT_AUTHOR_H
