/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz <cinamod@hotmail.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gr_Painter.h"
#include "gr_Graphics.h"

GR_Painter::GR_Painter (GR_Graphics * pGr)
  : m_pGr (pGr),
	m_pCaretDisabler(NULL)
{
	GR_ScreenGraphics *pSGC;

	UT_ASSERT (m_pGr);

	pSGC = dynamic_cast<GR_ScreenGraphics *>(m_pGr);
	m_pCaretDisabler = new GR_CaretDisabler(pSGC->getCaret());
	UT_sint32 i = 0;
	GR_Caret * pCaret = pSGC->getNthCaret(i);
	while(pCaret)
	{
	    GR_CaretDisabler * pCaretDisabler = new GR_CaretDisabler(pCaret);
	    m_vecDisablers.addItem(pCaretDisabler);
	    i++;
	    pCaret = pSGC->getNthCaret(i);
	}

	m_pGr->beginPaint ();
	m_pGr->setLineWidth(m_pGr->tlu(1));
}


GR_Painter::GR_Painter (GR_Graphics * pGr, bool bCaret)
  : m_pGr (pGr),
	m_pCaretDisabler(NULL)
{
	UT_ASSERT (m_pGr);

	if(bCaret)
	  m_pCaretDisabler = NULL;

	m_pGr->beginPaint ();
	m_pGr->setLineWidth(m_pGr->tlu(1));
}

GR_Painter::~GR_Painter ()
{
	m_pGr->endPaint ();
	DELETEP(m_pCaretDisabler);
	UT_VECTOR_PURGEALL(GR_CaretDisabler *, m_vecDisablers);
}

