/* AbiWord
 * Copyright (C) 2003 Martin Sevior
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

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "ap_Dialog_FormatFootnotes.h"

AP_Dialog_FormatFootnotes::AP_Dialog_FormatFootnotes(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogformatfootnotes.html"), m_answer(a_CANCEL)
{
}

AP_Dialog_FormatFootnotes::~AP_Dialog_FormatFootnotes(void)
{
}

void AP_Dialog_FormatFootnotes::setAnswer(AP_Dialog_FormatFootnotes::tAnswer a)
{
  m_answer = a;
}

AP_Dialog_FormatFootnotes::tAnswer AP_Dialog_FormatFootnotes::getAnswer(void) const
{
  return m_answer;
}
