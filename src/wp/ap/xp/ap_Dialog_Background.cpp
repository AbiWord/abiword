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

const XML_Char * AP_Dialog_Background::getColor (void) const
{
	return (const XML_Char *) m_pszColor;
}

void  AP_Dialog_Background::setColor (const XML_Char * pszColor)
{
	if(pszColor && strcmp(pszColor,"transparent") != 0)
	{
		UT_parseColor(pszColor,m_color);
		sprintf(m_pszColor,"%s",pszColor);
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
	sprintf(m_pszColor, "%02x%02x%02x",col.m_red,
				col.m_grn,col.m_blu);
}
