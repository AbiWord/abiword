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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"

#include "ap_Dialog_Background.h"

AP_Dialog_Background::AP_Dialog_Background(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id), m_answer(a_OK)
{
  UT_setColor(m_color, 0xff, 0xff, 0xff);
}

AP_Dialog_Background::~AP_Dialog_Background(void)
{
}

AP_Dialog_Background::tAnswer AP_Dialog_Background::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_Background::setAnswer (AP_Dialog_Background::tAnswer answer)
{
  m_answer = answer;
}

const UT_RGBColor & AP_Dialog_Background::getColor (void) const
{
  return m_color;
}

void  AP_Dialog_Background::setColor (UT_RGBColor & clr)
{
  UT_setColor(m_color, clr.m_red, clr.m_grn, clr.m_blu);
}
