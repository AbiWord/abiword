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


#include <stdio.h>
#include <malloc.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_imp_Text.h"
#include "pd_Document.h"
#include "ut_growbuf.h"

/*****************************************************************/
/*****************************************************************/

IEStatus IE_Imp_Text::importFile(const char * szFilename)
{
	FILE *fp = NULL;

	IEStatus iestatus;
	const char *attr[] = {"type", "Box", "left", "0pt", "top", "0pt", "width", "*", "height", "*", NULL};
	UT_GrowBuf gbBlock;
	int idx;
	unsigned char c;

	fp = fopen(szFilename, "r");
	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		iestatus = IES_FileNotFound;
		goto Cleanup;
	}
	
	m_pDocument->appendStrux(PTX_Section, NULL);
	m_pDocument->appendStrux(PTX_ColumnSet, NULL);
	m_pDocument->appendStrux(PTX_Column, attr);
	m_pDocument->appendStrux(PTX_Block, NULL);

	idx=0;
	while (fread(&c, 1, sizeof(c), fp) > 0)
	{

		if (c == '\n')
		{
			// Put current text in span, then create new Block
			m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength());
			m_pDocument->appendStrux(PTX_Block, NULL);
			gbBlock.truncate(0);
			idx = 0;
		} else {
			// Prepare character in span

			// HACK: this cast is bogus
			UT_UCSChar uc = (UT_UCSChar) c;
			gbBlock.ins(idx, 1);
			gbBlock.overwrite(idx, &uc, 1);
			idx++;
		}

	} 

	m_pDocument->appendSpan(gbBlock.getPointer(0), gbBlock.getLength());



	iestatus = IES_OK;

Cleanup:
	if (fp)
		fclose(fp);
	return iestatus;
}

IE_Imp_Text::~IE_Imp_Text()
{
}

IE_Imp_Text::IE_Imp_Text(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
}

