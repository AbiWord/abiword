/* AbiWord
 * Copyright (C) 2001 Tomas Frydrych
 * Copyright (C) 2011 Ben Martin
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

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "ap_Dialog_GetStringCommon.h"

AP_Dialog_GetStringCommon::AP_Dialog_GetStringCommon( XAP_DialogFactory * pDlgFactory,
                                                      XAP_Dialog_Id id,
                                                      const char* dialogfile )
  : XAP_Dialog_NonPersistent(pDlgFactory,id, dialogfile)
  , m_string("")
  , m_answer(a_CANCEL)
{
}

AP_Dialog_GetStringCommon::~AP_Dialog_GetStringCommon(void)
{
}

void
AP_Dialog_GetStringCommon::setAnswer(AP_Dialog_GetStringCommon::tAnswer a)
{
    m_answer = a;
}

AP_Dialog_GetStringCommon::tAnswer
AP_Dialog_GetStringCommon::getAnswer() const
{
    return m_answer;
}

const std::string &
AP_Dialog_GetStringCommon::getString() const
{
  return m_string;
}

void AP_Dialog_GetStringCommon::setString( const std::string& s )
{
    m_string = s.substr( 0, getStringSizeLimit() );
}

void AP_Dialog_GetStringCommon::setDoc(FV_View * pView)
{
	m_pDoc = pView->getDocument();
}
