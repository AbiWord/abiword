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
#include "fp_TextRun.h"
#include "fp_Page.h"

/*!
  Dump fp_Page information
  \param fp File where the dump should be written to
*/
void fp_Page::__dump(FILE * fp) const
{
	fprintf(fp,"\tPage: 0x%p\n",(void*)this);
}

/*!
  Dump fp_Run information
  \param fp File where the dump should be written to
*/
void fp_Run::__dump(FILE * fp) const
{
	static const char * s_names[] = { "Text", "Image", "Tab", "LineBreak", "ColBreak", "PageBreak", "Field", "FmtMark", "FieldStart", "FieldEnd" };
	UT_ASSERT(NrElements(s_names)==(FPRUN__LAST__-FPRUN__FIRST__));
	const char * szName = (((m_iType >= FPRUN__FIRST__) && (m_iType <= FPRUN__LAST__)) ? s_names[m_iType-1] : "Unknown");

	fprintf(fp,"    Run: %p T=%s Off=%d Len=%d D=%c Line=%p [x %d y %d w %d h %d]\n",
			(void*)this, szName, m_iOffsetFirst, m_iLen, 
			((m_bDirty) ? 'y' : 'n'), (void*)m_pLine,
			m_iX, m_iY, m_iWidth, m_iHeight);
}

/*!
  Dump fp_TextRun information
  \param fp File where the dump should be written to
*/
void fp_TextRun::__dump(FILE * fp) const
{
	fp_Run::__dump(fp);

	if (m_iLen == 0)
		return;

	fprintf(fp,"      [");
	{
		const UT_UCSChar* pSpan = NULL;
		UT_uint32 lenSpan = 0;

		UT_uint32 koff=m_iOffsetFirst;
		UT_uint32 klen=m_iLen;
		
		while (m_pBL->getSpanPtr(koff, &pSpan, &lenSpan) && (klen > 0))
		{
			UT_uint32 kdraw = MyMin(klen,lenSpan);
			for (UT_uint32 k=0; k<kdraw; k++)
			{
				// a cheap unicode to ascii hack...
				unsigned char c = (unsigned char)(pSpan[k] & 0x00ff);
				fprintf(fp,"%c",c);
			}

			klen -= kdraw;
			koff += lenSpan;
		}
	}
	fprintf(fp,"]\n");
		
}

#endif
