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


#ifdef PT_TEST

#include "ut_types.h"
#include "ut_test.h"
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Span.h"
#include "px_CR_Glob.h"

/*****************************************************************/
/*****************************************************************/

static const char * s_CRNames[] = {
	"GlobGlob",
	"InstSpan",
	"DeleSpan",
	"ChngSpan",
	"InstStrx",
	"DeleStrx",
	"ChngStrx",
	"InstObjt",
	"DeleObjt",
	"ChngObjt",
	"InstFMrk",
	"DeleFMrk",
	"ChngFMrk",
	"ChngPnt ",
	"UpdList ",
	"StopList",
	"DChngIP ",
	"AChngIP ",
	"UpdField",
	"RemList "
};

void
PX_ChangeRecord::__dump_type(FILE* fp) const
{
	UT_ASSERT(NrElements(s_CRNames)==(PXT__LAST__-PXT__FIRST__));
	const char * szName = 
		(((m_type >= PXT__FIRST__) && (m_type <= PXT__LAST__)) 
		 ? s_CRNames[m_type+1] : "????????");
	
	fprintf(fp, "  T[%s] api[%08lx] ", szName, (long)m_indexAP);
}

/*!
  Dump information about this change record
  \param fp File descripter used for output
  \fixme This function should be virtual, allowing subclasses to
         implement their own, thus make it possible to print out more
         information. The printed list of change records is not of
         much use since one cannot see how they relate to the stuff in
         the various buffers.
*/
void
PX_ChangeRecord::__dump(FILE* fp) const
{
	__dump_type(fp);
	fprintf(fp, "\n");
}

#if 0
void
PX_ChangeRecord::__dump_buf(FILE* fp) const
{
	__dump_type(fp);
	fprintf(fp, "b[%08lx,%ld@%08lx]\n", 
			(long)m_bufIndex, (long)m_length, (long)m_blockOffset);

	const UT_UCSChar * ptr = m_pPieceTable->getPointer(m_bufIndex);
	char c;
	UT_uint32 k;

	fprintf(fp,"\t[");
	for (k=0; k<m_length; k++)
	{
		// note: this is a cheap unicode to ascii conversion for
		// note: debugging purposes only.
		c = (  ((ptr[k] < 20) || (ptr[k] > 0x7f))
			   ? '@'
			   : (char)ptr[k]);
		fprintf(fp,"%c",c);
	}
	fprintf(fp,"]\n");
}
#endif

void
PX_ChangeRecord_SpanChange::__dump(FILE* fp) const
{
	__dump_type(fp);

	fprintf(fp, "b[%08lx,%ld@%08lx]\n",
			(long) m_bufIndex, (long)m_length, (long)m_blockOffset);
}

void
PX_ChangeRecord_Span::__dump(FILE* fp) const
{
	__dump_type(fp);

	fprintf(fp, "b[%08lx,%ld@%08lx]\n",
			(long) m_bufIndex, (long)m_length, (long)m_blockOffset);

#if 0
	const UT_UCSChar * ptr = m_pPieceTable->getPointer(m_bufIndex);
	char c;
	UT_uint32 k;

	fprintf(fp,"\t[");
	for (k=0; k<m_length; k++)
	{
		// note: this is a cheap unicode to ascii conversion for
		// note: debugging purposes only.
		c = (  ((ptr[k] < 20) || (ptr[k] > 0x7f))
			   ? '@'
			   : (char)ptr[k]);
		fprintf(fp,"%c",c);
	}
	fprintf(fp,"]\n");
#endif
}

void
PX_ChangeRecord_Glob::__dump(FILE* fp) const
{
	__dump_type(fp);

	const char* szFlag;
	switch (m_flags) 
	{
	case PXF_MultiStepStart:
		szFlag = "MSS";	break;
	case PXF_MultiStepEnd:
		szFlag = "MSE";	break;
	case PXF_UserAtomicStart:
		szFlag = "UAS";	break;
	case PXF_UserAtomicEnd:
		szFlag = "UAE";	break;
	default:
		szFlag = "???"; break;
	}

	fprintf(fp, "F[%s]\n", szFlag);

}

#endif /* PT_TEST */
