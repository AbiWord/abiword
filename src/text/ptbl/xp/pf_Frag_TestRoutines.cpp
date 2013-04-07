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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */


#ifdef PT_TEST

#include "ut_types.h"
#include "ut_test.h"
#include "pf_Frag.h"
#include "pf_Frag_FmtMark.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pt_PieceTable.h"


/*****************************************************************/
/*****************************************************************/

void pf_Frag::__dump(FILE * fp) const
{
	fprintf(fp,"        %sFragment %p type[%d]\n",
			((m_type==PFT_EndOfDoc) ? "EOD" : "Unk"),
			(void*)this,m_type);
}

void pf_Frag_FmtMark::__dump(FILE * fp) const
{
	fprintf(fp,"        FmtMrkFragment %p api[%08lx]\n",(void*)this,(long unsigned int)m_indexAP);
}

void pf_Frag_Strux_Block::__dump(FILE * fp) const
{
	fprintf(fp,"      Block %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}

void pf_Frag_Strux_Section::__dump(FILE * fp) const
{
	fprintf(fp,"    Section %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}


void pf_Frag_Strux_SectionHdrFtr::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionHdrFtr %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}

void pf_Frag_Strux_SectionTable::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionTable %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionCell::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionCell %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionFootnote::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionFootnote %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionAnnotation::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionAnnotation %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionEndnote::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionEndnote %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionMarginnote::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionMarginnote %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionFrame::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionFrame %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}


void pf_Frag_Strux_SectionEndTable::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionEndTable %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionEndCell::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionEndCell %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionEndFootnote::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionEndFootnote %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionEndAnnotation::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionEndAnnotion %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionEndMarginnote::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionEndMarginnote %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionEndFrame::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionEndFrame %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionEndEndnote::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionEndEndnote %p api[%08lx]\n",
			const_cast<void*>(static_cast<const void*>(this)),static_cast<long>(m_indexAP));
}

void pf_Frag_Strux_SectionTOC::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionTOC %p api[%08lx]\n",
			(void*)this,(long)m_indexAP);
}
void pf_Frag_Strux_SectionEndTOC::__dump(FILE * fp) const
{
	fprintf(fp,"    SectionEndTOC %p api[%08lx]\n",
			const_cast<void*>(static_cast<const void*>(this)),static_cast<long>(m_indexAP));
}

void pf_Frag_Text::__dump(FILE * fp) const
{
	fprintf(fp,"        TextFragment %p b[%08lx,%ld] api[%08lx]\n",
			const_cast<void*>(static_cast<const void*>(this)),static_cast<long>(m_bufIndex),static_cast<long>(m_length),static_cast<long>(m_indexAP));

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

void pf_Frag_Object::__dump(FILE * fp) const
{
	const char * sz = "";
	switch (m_objectType)
	{
	case PTO_Image:
		sz = "Image";
		break;
	case PTO_Field:
		sz = "Field";
		break;
	default:
		sz = "TODO";
		break;
	}
	
	fprintf(fp,"        Object %p t[%s] api[%08lx]\n",
			const_cast<void*>(static_cast<const void*>(this)),sz,static_cast<long>(m_indexAP));
}

#endif /* PT_TEST */
