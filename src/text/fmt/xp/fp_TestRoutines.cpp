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

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)

#include <stdio.h>
#include <string.h>
#include "ut_test.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "fl_BlockLayout.h"
#include "fp_Run.h"
#include "fp_Column.h"
#include "fp_Line.h"
#include "fp_TextRun.h"
#include "fp_Page.h"
#include "fp_Column.h"

#include "pd_Iterator.h"

/*!
  Dump fp_Column information
  \param fp File where the dump should be written to
*/
void fp_Column::__dump(FILE * fp) const
{
	fprintf(fp, "Col: %p X=%d Y=%d W=%d H=%d E=%s\n",
			static_cast<const void*>(this), getX(), getY(), getWidth(), getHeight(),
			(isEmpty() ? "Yes" : "No"));
}

/*!
  Dump fp_Page information
  \param fp File where the dump should be written to

  The column leader is identified by a plus sign.
*/
void fp_Page::__dump(FILE * fp) const
{
	int i, count;

	fprintf(fp,"  Page: %p\n",static_cast<const void*>(this));
	
	for (count = countColumnLeaders(), i = 0; i < count; i++)
	{
		fp_Column* pCol = getNthColumnLeader(i);

		if (pCol)
		{
			fprintf(fp, "  +");
			pCol->__dump(fp);

			while ((pCol = pCol->getFollower()))
			{
				fprintf(fp, "   ");
				pCol->__dump(fp);
			}
		}
		else
		{
			fprintf(fp, "   Col: ** NULL **\n");
		}

	}
}

/*!
  Dump fp_Run information
  \param fp File where the dump should be written to

  Prefix non-text runs with + to make them stand out in the dump.
*/
void fp_Run::__dump(FILE * fp) const
{
	static const char * s_names[] = { "Text", "Image", "Tab", 
									  "LineBreak", "ColBreak", "PageBreak",
									  "Field", "FmtMark", "FieldStart",
									  "FieldEnd", "EOP", "Bookmark",
									  "Hyperlink", "DirectionMarker" };
	UT_ASSERT(G_N_ELEMENTS(s_names)==(FPRUN__LAST__-FPRUN__FIRST__));
	const char * szName = 
		(((m_iType >= FPRUN__FIRST__) && (m_iType <= FPRUN__LAST__)) 
		 ? s_names[m_iType-1] : "Unknown");

	if (m_iType != FPRUN_TEXT)
		fprintf(fp,"   +");
	else
		fprintf(fp,"    ");

	fprintf(fp,"Run: %p T=%s Off=%d Len=%d D=%c [x %d y %d w %d h %d]\n",
			static_cast<const void*>(this), szName, m_iOffsetFirst, m_iLen, 
			((m_bDirty) ? 'y' : 'n'),
			m_iX, m_iY, m_iWidth, m_iHeight);

	if (m_iType != FPRUN_TEXT)
		fprintf(fp, "         [<%s>]\n", szName);
}

/*!
  Dump fp_TextRun information
  \param fp File where the dump should be written to
*/
void fp_TextRun::__dump(FILE * fp) const
{
	fp_Run::__dump(fp);

	fprintf(fp,"         [");
	if (getLength() != 0)
	{
		UT_uint32 koff=getBlockOffset();
		UT_uint32 klen=getLength();

		PD_StruxIterator text(getBlock()->getStruxDocHandle(),
							  koff + fl_BLOCK_STRUX_OFFSET);

		for(UT_uint32 k = 0; k < klen; k++)
		{
			unsigned char c = static_cast<unsigned char>(text[k+fl_BLOCK_STRUX_OFFSET] & 0x00ff);
			UT_return_if_fail(text.getStatus() == UTIter_OK);
			fprintf(fp,"%c",c);
		}
	}
	fprintf(fp,"]\n");
		
}

/*!
  Dump fp_Line information
  \param fp File where the dump should be written to
*/
void fp_Line::__dump(FILE * fp) const
{
	fprintf(fp,"Line: %p Col=%p X=%d Y=%d H=%d W=%d\n",
			static_cast<const void*>(this), static_cast<void*>(getContainer()),
			getX(), getY(), getHeight(), getMaxWidth());
}

#endif
