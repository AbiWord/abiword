/* AbiSource
 *
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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

#ifndef _ODi_STARTTAG_H_
#define _ODi_STARTTAG_H_

// AbiWord includes
#include <ut_stringbuf.h>
#include <ut_vector.h>

/**
 * This class contains the data of a start element tag.
 *
 * It's intended to be reused, so, its data is buffered.
 * Reusing this class is a good idea beacuse during the parsing of an
 * OpenDocument text stream there will be LOTS of creation and destruction
 * of start tags. Making "new" and "delete" operations for these events would
 * be a significant performance burden.
 */
class ODi_StartTag {
public:

   ODi_StartTag(UT_uint32 attributeGrowStep=10);
   ~ODi_StartTag();

   void set(const gchar* pName, const gchar** ppAtts);

    /**
     * @return An UTF-8 string.
     */
    inline const char* getName() const {return m_name.data();}

    inline UT_uint32 getAttributeCount() const {return m_attributeSize/2;}

    /**
     * @param rName An UTF-8 string, conataining the attribute name.
     * @return An UTF-8 string, containing its value.
     */
    const char* getAttributeValue(const char* rName ) const;



private:

   UT_UTF8Stringbuf m_name;

   // Even values are the attributes names and the odds are the values
   UT_UTF8Stringbuf* m_pAttributes;

   // Used array slots for m_pAttributes
   UT_uint32 m_attributeSize;

   // Allocated array slots for m_pAttributes
   UT_uint32 m_attributeMemSize;

   UT_uint32 m_attributeGrowStep;


   void _growAttributes();
};

#endif //_ODi_STARTTAG_H_
