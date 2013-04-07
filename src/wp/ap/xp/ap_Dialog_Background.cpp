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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"

#include "ap_Dialog_Background.h"

AP_Dialog_Background::AP_Dialog_Background(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogbackgroundcolor"),
	m_color(0xff,0xff,0xff),
	m_answer(a_OK),
	m_bDoForeground(false),
	m_bDoHighlight(false)
{
	sprintf(m_pszColor,"%s","transparent");
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

void AP_Dialog_Background::setForeground(void)
{
	m_bDoForeground = true;
}


void AP_Dialog_Background::setHighlight(void)
{
	m_bDoHighlight = true;
}

const gchar * AP_Dialog_Background::getColor (void) const
{
	return static_cast<const gchar *>(m_pszColor);
}

void  AP_Dialog_Background::setColor (const gchar * pszColor)
{
	if(pszColor && strcmp(pszColor,"transparent") != 0)
	{
		UT_parseColor(pszColor,m_color);
		sprintf(m_pszColor, "%02x%02x%02x", m_color.m_red, m_color.m_grn, m_color.m_blu);
	}
	else
	{
		UT_setColor(m_color, 255, 255, 255);
		sprintf(m_pszColor,"%s","transparent");
	}
}


void  AP_Dialog_Background::setColor (UT_RGBColor & col)
{
	UT_setColor(m_color, col.m_red, col.m_grn, col.m_blu);
	sprintf(m_pszColor, "%02x%02x%02x", m_color.m_red, m_color.m_grn, m_color.m_blu);
}
