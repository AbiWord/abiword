/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_UnixGnomeDlg_ClipArt.h"
#include "xap_UnixDialogHelper.h"
#include "xap_Frame.h"
#include "xap_App.h"

// define this if you want the stock gnome icons in there too
//#define USE_STOCK_ICONS_TOO 1

// TODO: make this use the GnomeIconList widget instead

XAP_Dialog * XAP_UnixGnomeDialog_ClipArt::static_constructor(XAP_DialogFactory * pFactory,
							     XAP_Dialog_Id id)
{
  XAP_UnixGnomeDialog_ClipArt * p = new XAP_UnixGnomeDialog_ClipArt(pFactory,id);
  return p;
}

XAP_UnixGnomeDialog_ClipArt::XAP_UnixGnomeDialog_ClipArt(XAP_DialogFactory * pDlgFactory,
							 XAP_Dialog_Id id)
  : XAP_Dialog_ClipArt (pDlgFactory,id)
{
}

XAP_UnixGnomeDialog_ClipArt::~XAP_UnixGnomeDialog_ClipArt(void)
{
}

#if 1

// simple fallback dummy impl.

GtkWidget * XAP_UnixGnomeDialog_ClipArt::_constructPreviewPane ()
{
  GtkWidget * clipArt = gnome_icon_selection_new ();
  
#ifdef USE_STOCK_ICONS_TOO	
  gnome_icon_selection_add_defaults (GNOME_ICON_SELECTION(clipArt));
#endif
  gnome_icon_selection_add_directory (GNOME_ICON_SELECTION(clipArt), 
				      getInitialDir());
  
  return clipArt;
}

#else

// TODO: turn on this code someday

#define ICON_WIDTH 48

GtkWidget * XAP_UnixGnomeDialog_ClipArt::_constructPreviewPane ()
{
  GtkWidget * clipArt = gnome_icon_list_new_flags (ICON_WIDTH,
						   NULL,
						   0);  
  
  gnome_icon_list_set_selection_mode (GNOME_ICON_LIST (clipArt), 
				      GTK_SELECTION_SINGLE);
  
  
  gnome_icon_list_append (GNOME_ICON_LIST (clipArt),
			  getInitialDir(), getInitialDir());
  
  return clipArt;
}
#endif

void XAP_UnixGnomeDialog_ClipArt::runModal(XAP_Frame * pFrame)
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();
  
  GtkWidget * mainWindow = abiDialogNew ("clipart dialog", TRUE, pSS->getValue (XAP_STRING_ID_DLG_CLIPART_Title));
  UT_return_if_fail(mainWindow);
  
  GtkWidget * clipArt = _constructPreviewPane ();
  
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(mainWindow)->vbox), clipArt, 
		      TRUE, FALSE, 0);

  abiAddStockButton(GTK_DIALOG(mainWindow), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  abiAddStockButton(GTK_DIALOG(mainWindow), GTK_STOCK_OK, GTK_RESPONSE_OK);
  
  connectFocus(GTK_WIDGET(mainWindow), pFrame);
  
  // load after the show_all to give the impression that we're 
  // loading the icons
  gtk_widget_show_all (mainWindow);
  gnome_icon_selection_show_icons (GNOME_ICON_SELECTION(clipArt));

  const gchar * graphic = NULL;
  switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this, GTK_RESPONSE_CANCEL, false ) )
    {
    case GTK_RESPONSE_OK:
      graphic = gnome_icon_selection_get_icon(GNOME_ICON_SELECTION(clipArt), 
							    TRUE);
      if (graphic) {
	setGraphicName (graphic);
	setAnswer (XAP_Dialog_ClipArt::a_OK);
      }
      else {
	setAnswer (XAP_Dialog_ClipArt::a_CANCEL);
      }
      break;
    default:
      break;
    }
  
  abiDestroyWidget(mainWindow);
}

