/* AbiSuite
 * Copyright (C) Jordi Mas i Hernàndez
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

#include "ut_vector.h"
#include "ut_xml.h"
#include "ut_hash.h"

#define MAX_PATHNM 512

class Barbarisms : public UT_XML::Listener
{
public:	
		Barbarisms();
		~Barbarisms();
		
		bool 			load(const char *szHash);		
		
		bool		 	suggestWord(const UT_UCSChar *word32, size_t length, UT_Vector* pVecsugg);
		
		bool checkWord(const UT_UCSChar * word32, size_t length);
		
		/* 
			Implementation of UT_XML::Listener
		*/
		void			startElement(const XML_Char *name, const XML_Char **atts);
		void			endElement(const XML_Char *name){};
		void			charData(const XML_Char *s, int len){};		

private:

		bool			suggestExactWord(const UT_UCSChar *word32, size_t length,	UT_Vector* pVecsugg);			
		
		UT_StringPtrMap	m_map;
		UT_Vector*		m_pCurVector;	
		

};

