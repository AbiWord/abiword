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
#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_dialogHelper.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Spell.h"
#include "ap_UnixGnomeDialog_Spell.h"

/*****************************************************************/
XAP_Dialog * AP_UnixGnomeDialog_Spell::static_constructor(XAP_DialogFactory * pFactory,
							  XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_Spell * p = new AP_UnixGnomeDialog_Spell(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_Spell::AP_UnixGnomeDialog_Spell(XAP_DialogFactory * pDlgFactory,
						  XAP_Dialog_Id id)
	: AP_UnixDialog_Spell(pDlgFactory, id)
{
}

AP_UnixGnomeDialog_Spell::~AP_UnixGnomeDialog_Spell(void)
{
}

GtkWidget * AP_UnixGnomeDialog_Spell::_constructWindow (void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	m_windowMain = gnome_dialog_new (pSS->getValue(AP_STRING_ID_DLG_Spell_SpellTitle), NULL);
	UT_ASSERT(m_windowMain);

	_constructWindowContents (GNOME_DIALOG (m_windowMain)->vbox);

	_connectSignals ();
	gtk_widget_show_all (m_windowMain);

	return (m_windowMain);
}

void AP_UnixGnomeDialog_Spell::_createButtons(void)
{
  GtkWidget *pixmap;

  const XAP_StringSet * pSS = m_pApp->getStringSet();
  XML_Char * unixstr = NULL;      // used for conversions

  pixmap = gnome_stock_pixmap_widget(m_windowMain, GNOME_STOCK_PIXMAP_CONVERT);
  UT_XML_cloneNoAmpersands(unixstr,
			   pSS->getValue(AP_STRING_ID_DLG_Spell_Change));
  //m_buttonChange = gtk_button_new_with_label(unixstr);
  m_buttonChange = gnome_pixmap_button(pixmap, unixstr);
  FREEP(unixstr);

  pixmap = gnome_stock_pixmap_widget(m_windowMain, GNOME_STOCK_PIXMAP_CONVERT);
  UT_XML_cloneNoAmpersands(unixstr, 
			   pSS->getValue(AP_STRING_ID_DLG_Spell_ChangeAll));
  //m_buttonChangeAll = gtk_button_new_with_label(unixstr);
  m_buttonChangeAll = gnome_pixmap_button(pixmap, unixstr);
  FREEP(unixstr);

  pixmap = gnome_stock_pixmap_widget(m_windowMain, GNOME_STOCK_PIXMAP_TRASH);
  UT_XML_cloneNoAmpersands(unixstr, 
			   pSS->getValue(AP_STRING_ID_DLG_Spell_Ignore));
  //m_buttonIgnore = gtk_button_new_with_label(unixstr);
  m_buttonIgnore = gnome_pixmap_button(pixmap, unixstr);
  FREEP(unixstr);

  pixmap = gnome_stock_pixmap_widget(m_windowMain, GNOME_STOCK_PIXMAP_TRASH_FULL);
  UT_XML_cloneNoAmpersands(unixstr, 
			   pSS->getValue(AP_STRING_ID_DLG_Spell_IgnoreAll));
  //m_buttonIgnoreAll = gtk_button_new_with_label(unixstr);
  m_buttonIgnoreAll = gnome_pixmap_button(pixmap, unixstr);
  FREEP(unixstr);

  pixmap = gnome_stock_pixmap_widget(m_windowMain, GNOME_STOCK_PIXMAP_ADD);
  UT_XML_cloneNoAmpersands(unixstr, 
			   pSS->getValue(AP_STRING_ID_DLG_Spell_AddToDict));
  //m_buttonAddToDict = gtk_button_new_with_label(unixstr);
  m_buttonAddToDict = gnome_pixmap_button(pixmap, unixstr);
  FREEP(unixstr);

  m_buttonCancel = gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL);
}


