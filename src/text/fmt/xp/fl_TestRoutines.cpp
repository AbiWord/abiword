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

#if defined(PT_TEST) || defined(FMT_TEST) || defined(UT_TEST)

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include "ut_test.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "fp_Page.h"
#include "fp_Run.h"
#include "fp_Line.h"
#include "fl_BlockLayout.h"
#ifdef ENABLE_SPELL
#include "fl_Squiggles.h"
#endif
#include "fl_DocLayout.h"
#include "fl_SectionLayout.h"
#include "fl_TableLayout.h"
#include "pt_PieceTable.h"

FL_DocLayout* FL_DocLayout::m_pDocLayout = NULL;

/*!
  Dump AbiWord backend's internal structures
  This function is for use in GDB (and other debuggers allowing
  interaction with the program being debugged).
  Type this to print out the structures from the GDB command line:
   (gdb) p __dump()
*/
void __dump(void)
{
	FL_DocLayout::m_pDocLayout->__dump(stdout);
	FL_DocLayout::m_pDocLayout->getDocument()->__dump(stdout);
}

/*!
  Dump AbiWord backend's internal fmt structures
  \see __dump
*/
void __dump_fmt(void)
{
	FL_DocLayout::m_pDocLayout->__dump(stdout);
}

/*!
  Dump AbiWord backend's internal pt structures
  \see __dump

  \note This dumps the last created document. This will differ from
        the document pointed to by the layout in some situations
        (during import, for instance).  
*/
void __dump_pt(void)
{
	FL_DocLayout::m_pDocLayout->getDocument()->m_pDoc->__dump(stdout);
}

/*!
  Dump AbiWord backend's internal change history structures
  \see __dump
*/
void __dump_ch(void)
{
	FL_DocLayout::m_pDocLayout->getDocument()->getPieceTable()->getChangeHistory()->__dump(stdout);
}

/*!
  Dump AbiWord backend's squiggle structures
  \see __dump
*/
void __dump_sq(void)
{
#ifdef ENABLE_SPELL
	if (FL_DocLayout::m_pDocLayout->isPendingWordForSpell())
	{
		fl_PartOfBlock* pPOB = FL_DocLayout::m_pDocLayout->getPendingWordForSpell();
		fprintf(stdout, "Pending word: %p [%d:%d]\n", (void*)pPOB,
				pPOB->getOffset(), pPOB->getOffset()+pPOB->getPTLength());
	}
	else
	{
		fprintf(stdout, "No pending word.\n");
	}
#endif
	fprintf(stdout,"FL_DocLayout::__dump(%p) sections:\n",(void*)FL_DocLayout::m_pDocLayout);
	for (fl_SectionLayout * psl=FL_DocLayout::m_pDocLayout->getFirstSection(); (psl); psl=static_cast<fl_SectionLayout *>(psl->getNext()))
	{
		fprintf(stdout,"Section: %p [type %d]\n",(void*)psl,psl->getType());
#ifdef ENABLE_SPELL
		for (fl_BlockLayout * pBL=(fl_BlockLayout *) psl->getFirstLayout(); (pBL); pBL= (fl_BlockLayout *) pBL->getNext())
			pBL->getSpellSquiggles()->__dump(stdout);
#endif
	}
}

/*!
  Dump pages and sections contained in this FL_DocLayout
  \param fp File where the dump should be written to
*/
void FL_DocLayout::__dump(FILE * fp) const
{
	int count = m_vecPages.getItemCount();

	fprintf(fp,"FL_DocLayout::__dump(%p) contains %d pages.\n", 
			(void*)this, m_vecPages.getItemCount());
	for (int i=0; i<count; i++)
	{
		fp_Page* p = (fp_Page*) m_vecPages.getNthItem(i);
		p->__dump(fp);
	}

	fprintf(fp,"FL_DocLayout::__dump(%p) sections:\n",(void*)this);
	for (fl_SectionLayout * psl= (fl_SectionLayout *) getFirstSection(); (psl); psl= (fl_SectionLayout *) psl->getNext())
	{
		psl->__dump(fp);
	}
}

/*!
  Dump sections contained in this fl_SectionLayout
  \param fp File where the dump should be written to
*/
void fl_SectionLayout::__dump(FILE * fp) const
{
	fprintf(fp,"Section: %p [type %d]\n",(void*)this,getType());
	for (fl_BlockLayout * pBL=(fl_BlockLayout *) getFirstLayout(); (pBL); pBL= (fl_BlockLayout *) pBL->getNext())
		pBL->__dump(fp);
}


/*!
  Dump sections contained in this fl_SectionLayout
  \param fp File where the dump should be written to
*/
void fl_ContainerLayout::__dump(FILE * fp) const
{
	fprintf(fp,"ContainerLayout: %p [Containertype %d]\n",(void*)this,getContainerType());
	for (fl_ContainerLayout * pBL=(fl_ContainerLayout *) getFirstLayout(); (pBL); pBL= (fl_ContainerLayout *) pBL->getNext())
		pBL->__dump(fp);
}


/*!
  Dump sections contained in this fl_SectionLayout
  \param fp File where the dump should be written to
*/
void fl_CellLayout::__dump(FILE * fp) const
{
	fprintf(fp,"CellLayout: %p [Containertype %d]\n",(void*)this,getContainerType());
	for (fl_ContainerLayout * pBL=(fl_ContainerLayout *) getFirstLayout(); (pBL); pBL= (fl_ContainerLayout *) pBL->getNext())
		pBL->__dump(fp);
}


/*!
  Dump sections contained in this fl_SectionLayout
  \param fp File where the dump should be written to
*/
void fl_TableLayout::__dump(FILE * fp) const
{
	fprintf(fp,"TableLayout: %p [Containertype %d]\n",(void*)this,getContainerType());
	for (fl_ContainerLayout * pBL=(fl_ContainerLayout *) getFirstLayout(); (pBL); pBL= (fl_BlockLayout *) pBL->getNext())
		pBL->__dump(fp);
}

/*!
  Dump runs contained in this fl_BlockLayout
  \param fp File where the dump should be written to

  We use an asterix to mark the first line of a container, making it
  easier to cross reference with the page/column list.
*/
void fl_BlockLayout::__dump(FILE * fp) const
{
	fprintf(fp,"  Block: %p [sdh %p]\n",(void*)this,(void*)getStruxDocHandle());
	fp_Container* pContainer = (fp_Container*)-1;
	fp_Run* pRun;
	fp_Line* pLine;

	// Get last line of previous block and its container.
	fl_BlockLayout* pPrev = (fl_BlockLayout *) getPrev();
	if (pPrev)
	{
		pLine = (fp_Line *) pPrev->getLastContainer();
		if (pLine)
		{
			pContainer = pLine->getContainer();
		}
	}

	pRun = m_pFirstRun;
	pLine = (fp_Line*)-1;
	for (; (pRun); pRun=pRun->getNextRun())
	{
		if (pRun->getLine() != pLine)
		{
			pLine = pRun->getLine();
			if (pLine)
			{
				// Set a mark each time we change container
				if (pLine->getContainer() != pContainer)
				{
					pContainer = pLine->getContainer();
					fprintf(fp, "  *");
				}
				else
				{
					fprintf(fp, "   ");
				}

				pLine->__dump(fp);
			}
			else
			{
				fprintf(fp, "   Line: ** NULL **\n");
			}
		}
		pRun->__dump(fp);
	}
}

void fl_Squiggles::__dump(FILE * fp) const
{
	fprintf(fp," Squiggles: %p\n",(void*)this);

	UT_sint32 iSquiggles = _getCount();
	fl_PartOfBlock* pPOB;
	UT_sint32 j;
	for (j = 0; j < iSquiggles; j++)
	{
		pPOB = getNth(j);
		fprintf(fp, "  %d: [%d:%d]\n", j, pPOB->getOffset(),
				pPOB->getOffset() + pPOB->getPTLength());
	}
}
#endif
