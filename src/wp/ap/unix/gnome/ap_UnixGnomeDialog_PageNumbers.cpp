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

#include <gnome.h>
#include "ap_UnixGnomeDialog_PageNumbers.h"
#include "ap_Strings.h"
#include "ut_debugmsg.h"

AP_UnixGnomeDialog_PageNumbers::AP_UnixGnomeDialog_PageNumbers(XAP_DialogFactory * pDlgFactory, 
			       XAP_Dialog_Id id) :
  AP_UnixDialog_PageNumbers (pDlgFactory, id)
{
}

AP_UnixGnomeDialog_PageNumbers::~AP_UnixGnomeDialog_PageNumbers(void)
{
}

XAP_Dialog * AP_UnixGnomeDialog_PageNumbers::static_constructor(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
{
  AP_UnixGnomeDialog_PageNumbers * dlg = new AP_UnixGnomeDialog_PageNumbers (pDlgFactory, id);
  return dlg;
}

GtkWidget * AP_UnixGnomeDialog_PageNumbers::_constructWindow (void)
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();
  m_window = gnome_dialog_new (pSS->getValue(AP_STRING_ID_DLG_PageNumbers_Title), NULL);

  _constructWindowContents (GNOME_DIALOG(m_window)->vbox);

  gnome_dialog_append_button (GNOME_DIALOG (m_window), GNOME_STOCK_BUTTON_OK);
  m_buttonOK = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_window)->buttons)->data);

  gnome_dialog_append_button (GNOME_DIALOG (m_window), GNOME_STOCK_BUTTON_CANCEL);
  m_buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_window)->buttons)->data);

  _connectSignals ();
  return m_window;
}
