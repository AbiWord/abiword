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
#include <stdlib.h>
#include "ut_string.h"
#include "ap_Dialog_PageNumbers.h"

AP_Dialog_PageNumbers::AP_Dialog_PageNumbers (XAP_DialogFactory * pDlgFactory,
					      XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id)
{
  m_answer  = a_OK;
  m_pFrame  = NULL;
  m_preview = NULL;
}

AP_Dialog_PageNumbers::~AP_Dialog_PageNumbers(void)
{
  DELETEP(m_preview);
}

AP_Dialog_PageNumbers::tAnswer AP_Dialog_PageNumbers::getAnswer(void)
{
  return m_answer;
}

AP_Dialog_PageNumbers::tAlign AP_Dialog_PageNumbers::getAlignment(void)
{
  return m_align;
}

UT_Bool AP_Dialog_PageNumbers::isHeader(void)
{
  return (m_control == id_HDR);
}

UT_Bool AP_Dialog_PageNumbers::isFooter(void)
{
  return (m_control == id_FTR);
}

void AP_Dialog_PageNumbers::_updatePreview(AP_Dialog_PageNumbers::tAlign align, 
					   AP_Dialog_PageNumbers::tControl ctrl)
{
  UT_ASSERT(m_preview);
  m_preview->setHdrFtr (ctrl);
  m_preview->setAlign (align);
  m_preview->draw ();
}

void AP_Dialog_PageNumbers::_createPreviewFromGC(GR_Graphics * gc,
						 UT_uint32 width,
						 UT_uint32 height)
{
  UT_ASSERT(gc);
  m_preview = new AP_Preview_PageNumbers (gc);
  UT_ASSERT (m_preview);

  m_preview->setWindowSize (width, height);  
}

/**************************************************************************/

AP_Preview_PageNumbers::AP_Preview_PageNumbers (GR_Graphics * gc)
  : XAP_Preview (gc)
{
  char fontString [10];
  sprintf(fontString, "%dpt", 8);

  GR_Font * found =  m_gc->findFont("Times New Roman", "normal", "", "normal", "", fontString);

  m_gc->setFont(found);

  UT_UCS_cloneString_char (&m_str, "1");
}

AP_Preview_PageNumbers::~AP_Preview_PageNumbers(void)
{
  FREEP(m_str);
}

void AP_Preview_PageNumbers::setHdrFtr(AP_Dialog_PageNumbers::tControl control)
{
  m_control = control;
}

void AP_Preview_PageNumbers::setAlign(AP_Dialog_PageNumbers::tAlign align)
{
  m_align = align;
}

void AP_Preview_PageNumbers::draw (void)
{
  int x = 0, y = 0;

  UT_ASSERT (m_gc);

  m_gc->clearArea (0, 0, getWindowWidth(), getWindowHeight());

  // TODO: make this right for headers, footers, and both
  // TODO: and put some text in the middle too

  switch (m_align)
    {
    case AP_Dialog_PageNumbers::id_RALIGN : x = getWindowWidth() - (2 * m_gc->measureUnRemappedChar(*m_str)); break;
    case AP_Dialog_PageNumbers::id_CALIGN : x = (int)(getWindowWidth()/2); break;
    case AP_Dialog_PageNumbers::id_LALIGN : x =  3 * m_gc->measureUnRemappedChar(*m_str); break;
    }

  switch (m_control)
    {
    case AP_Dialog_PageNumbers::id_HDR : y = m_gc->getFontHeight(); break;
    case AP_Dialog_PageNumbers::id_FTR : y = getWindowHeight() - (2 * m_gc->getFontHeight()); break;
    }

  m_gc->drawChars (m_str, 0, UT_UCS_strlen(m_str), x, y);
}
