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

#include "ut_assert.h"
#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "ap_Strings.h"
#include "ut_dialogHelper.h"

#include "ap_UnixDialog_PageSetup.h"

#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

// include the 3 pixmaps that we use in this dialog
#include "orient-vertical.xpm"
#include "orient-horizontal.xpm"
#include "margin.xpm"

#include "ut_debugmsg.h"

/*********************************************************************************/

// static helper functions

static GtkWidget *
create_pixmap (GtkWidget *w, char **data)
{
  GtkWidget *pixmap;
  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;

  colormap = gtk_widget_get_colormap (w);
  gdkpixmap = gdk_pixmap_colormap_create_from_xpm_d (NULL, colormap, &mask,
						     NULL, (gchar **)data);

  pixmap = gtk_pixmap_new (gdkpixmap, mask);
  gdk_pixmap_unref (gdkpixmap);
  gdk_bitmap_unref (mask);
  return pixmap;
}

static char *
_ev_convert (char * bufResult,
	     const char * szString)
{
	UT_ASSERT (szString && bufResult);
	
	char *pl = bufResult;
	char *s = (char *)szString;

	int len = strlen (szString);
	int i;

	for (i = 0; i < len; i++)
	  {
	    if (*s == '&')
	      s++;
	    else
	      *pl++ = *s++;
	  }

	*pl = 0;
	return bufResult;
}

static UT_Dimension 
fp_2_dim (fp_PageSize::Unit u)
{
  switch (u)
    {
    case fp_PageSize::cm   : return DIM_CM;
    case fp_PageSize::mm   : return DIM_MM;
    case fp_PageSize::inch :
    default :
      return DIM_IN;
    }
}

static int
fp_2_pos (fp_PageSize::Unit u)
{
   switch (u)
    {
    case fp_PageSize::cm   : return 1;
    case fp_PageSize::mm   : return 2;
    case fp_PageSize::inch :
    default :
      return 0;
    } 
}

#define FMT_STRING "%0.2f"
static GtkWidget *
create_entry (float v)
{
  gchar * val;
  GtkWidget * e = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (e), 4);
  val = g_strdup_printf (FMT_STRING, v);
  gtk_entry_set_text (GTK_ENTRY (e), val);
  gtk_entry_set_editable (GTK_ENTRY (e), FALSE);
  gtk_widget_set_usize (e, gdk_string_measure (e->style->font, val) + 15, 0);
  g_free (val);

  return e;
}

/*********************************************************************************/

// some static variables
static fp_PageSize::Predefined last_page_size = fp_PageSize::Custom;
static fp_PageSize::Unit last_page_unit = fp_PageSize::inch;
static fp_PageSize::Unit last_margin_unit = fp_PageSize::inch;

/*********************************************************************************/

// a *huge* convenience macro
static char _ev_buf[256];
#define _(a, x) _ev_convert (_ev_buf, pSS->getValue (a##_STRING_ID_##x))

// string tags to stuff stored in widget data
#define WIDGET_MENU_OPTION_PTR		"menuoptionptr"
#define WIDGET_MENU_VALUE_TAG		"value"

// convenience macro
#define CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(w, m, d, f)				\
        do {												\
                gtk_object_set_data (GTK_OBJECT (w), WIDGET_MENU_OPTION_PTR, (gpointer)m);                \
                gtk_object_set_data (GTK_OBJECT (w), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER(d));                \
	        gtk_signal_connect (GTK_OBJECT (w), "activate",	\
                GTK_SIGNAL_FUNC (f),		\
                (gpointer)this);							\
        } while (0)

#define CONVERT_DIMENSIONS(v, d1, d2) v = UT_convertInchesToDimension (UT_convertDimToInches (v, fp_2_dim (d1)), fp_2_dim (d2))

/*********************************************************************************/

// static event callbacks

static void s_ok_clicked (GtkWidget * w, AP_UnixDialog_PageSetup *dlg)
{
  UT_ASSERT (dlg);
  dlg->event_OK ();
}

static void s_cancel_clicked (GtkWidget * w, AP_UnixDialog_PageSetup *dlg)
{
  UT_ASSERT (dlg);
  dlg->event_Cancel ();
}

static void s_delete_clicked (GtkWidget * w,
			      gpointer data,
			      AP_UnixDialog_PageSetup * dlg)
{
  UT_ASSERT (dlg);
  dlg->event_WindowDelete ();
}

static void s_menu_item_activate (GtkWidget * widget)
{
	GtkWidget *option_menu = (GtkWidget *)gtk_object_get_data (GTK_OBJECT (widget),
								   WIDGET_MENU_OPTION_PTR);
	UT_ASSERT(option_menu && GTK_IS_OPTION_MENU (option_menu));

	gpointer p = gtk_object_get_data (GTK_OBJECT (widget),
					  WIDGET_MENU_VALUE_TAG);

	gtk_object_set_data (GTK_OBJECT (option_menu), WIDGET_MENU_VALUE_TAG, p);
}

static void s_page_size_changed (GtkWidget * w, AP_UnixDialog_PageSetup *dlg)
{
  UT_ASSERT(w && dlg);

  s_menu_item_activate (w);
  dlg->event_PageSizeChanged ();
}

static void s_page_units_changed (GtkWidget * w, AP_UnixDialog_PageSetup *dlg)
{
  UT_ASSERT(w && dlg);
  s_menu_item_activate (w);
  dlg->event_PageUnitsChanged ();
}

static void s_margin_units_changed (GtkWidget * w, AP_UnixDialog_PageSetup *dlg)
{
  UT_ASSERT(w && dlg);

  s_menu_item_activate (w);
  dlg->event_MarginUnitsChanged ();
}

/*********************************************************************************/

void AP_UnixDialog_PageSetup::event_OK (void)
{
	setAnswer (a_OK);

	fp_PageSize fp (last_page_size);

	setPageSize (fp);
	setMarginUnits (last_margin_unit);
	setPageUnits (last_page_unit);
	setPageOrientation (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioPagePortrait)) ? PORTRAIT : LANDSCAPE);
	setPageScale (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (m_spinPageScale)));

	setMarginTop (gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginTop)));
	setMarginBottom (gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginBottom)));
	setMarginLeft (gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginLeft)));
	setMarginRight (gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginRight)));
	setMarginHeader (gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginHeader)));
	setMarginFooter (gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginFooter)));

	gtk_main_quit();
}

void AP_UnixDialog_PageSetup::event_Cancel (void)
{
	setAnswer (a_CANCEL);
	gtk_main_quit();
}

void AP_UnixDialog_PageSetup::event_WindowDelete (void)
{
        event_Cancel();
}

void AP_UnixDialog_PageSetup::event_PageUnitsChanged (void)
{
  fp_PageSize::Unit pu = (fp_PageSize::Unit) GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (m_optionPageUnits), 
										   WIDGET_MENU_VALUE_TAG));

  float width, height;

  fp_PageSize ps(last_page_size);
  
  // get values  
  width  = (float)ps.Width (pu);
  height = (float)ps.Height (pu);

  last_page_unit = pu;
  
  // set values
  gchar * val;

  val = g_strdup_printf (FMT_STRING, width);
  gtk_entry_set_text (GTK_ENTRY (m_entryPageWidth), val);
  g_free (val);

  val = g_strdup_printf (FMT_STRING, height);
  gtk_entry_set_text (GTK_ENTRY (m_entryPageHeight), val);
  g_free (val);
}

void AP_UnixDialog_PageSetup::event_PageSizeChanged (void)
{

  fp_PageSize::Predefined pd = (fp_PageSize::Predefined) GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (m_optionPageSize),
											       WIDGET_MENU_VALUE_TAG));
  fp_PageSize ps(pd);

  float w, h;

  w = ps.Width (fp_PageSize::inch);
  h = ps.Height (fp_PageSize::inch);

  CONVERT_DIMENSIONS (w, fp_PageSize::inch, last_page_unit);
  CONVERT_DIMENSIONS (h, fp_PageSize::inch, last_page_unit);

  // set values
  gchar * val;

  val = g_strdup_printf (FMT_STRING, w);
  gtk_entry_set_text (GTK_ENTRY (m_entryPageWidth), val);
  g_free (val);

  val = g_strdup_printf (FMT_STRING, h);
  gtk_entry_set_text (GTK_ENTRY (m_entryPageHeight), val);
  g_free (val);

  last_page_size = pd;
}

void AP_UnixDialog_PageSetup::event_MarginUnitsChanged (void)
{
  fp_PageSize::Unit mu = (fp_PageSize::Unit) GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (m_optionMarginUnits),
										   WIDGET_MENU_VALUE_TAG));

  float top, bottom, left, right, header, footer;

  top    = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginTop));
  bottom = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginBottom));
  left   = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginLeft));
  right  = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginRight));
  header = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginHeader));
  footer = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (m_spinMarginFooter));

  CONVERT_DIMENSIONS (top,    last_margin_unit, mu);
  CONVERT_DIMENSIONS (bottom, last_margin_unit, mu);
  CONVERT_DIMENSIONS (left,   last_margin_unit, mu);
  CONVERT_DIMENSIONS (right,  last_margin_unit, mu);
  CONVERT_DIMENSIONS (header, last_margin_unit, mu);
  CONVERT_DIMENSIONS (footer, last_margin_unit, mu);

  last_margin_unit = mu;

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginTop), top);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginBottom), bottom);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginLeft), left);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginRight), right);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginHeader), header);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginFooter), footer);
}

/*********************************************************************************/

XAP_Dialog *
AP_UnixDialog_PageSetup::static_constructor(XAP_DialogFactory * pFactory,
					    XAP_Dialog_Id id)
{
    AP_UnixDialog_PageSetup * p = new AP_UnixDialog_PageSetup(pFactory,id);
    return p;
}

AP_UnixDialog_PageSetup::AP_UnixDialog_PageSetup (XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id) 
  : AP_Dialog_PageSetup (pDlgFactory, id)
{
  // nada
}

AP_UnixDialog_PageSetup::~AP_UnixDialog_PageSetup (void)
{
  // nada
}

void AP_UnixDialog_PageSetup::runModal (XAP_Frame *pFrame)
{
    // Build the window's widgets and arrange them
    GtkWidget * mainWindow = _constructWindow();
    UT_ASSERT(mainWindow);

    connectFocus(GTK_WIDGET(mainWindow), pFrame);

    // To center the dialog, we need the frame of its parent.
    XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
    UT_ASSERT(pUnixFrame);
    
    // Get the GtkWindow of the parent frame
    GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
    UT_ASSERT(parentWindow);
    
    // Center our new dialog in its parent and make it a transient
    // so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);

    // Show the top level dialog,
    gtk_widget_show(mainWindow);

    // Make it modal, and stick it up top
    gtk_grab_add(mainWindow);

    // Run into the GTK event loop for this window.
    gtk_main();
    gtk_widget_destroy(mainWindow);
}

void AP_UnixDialog_PageSetup::_connectSignals (void)
{
  	// the control buttons
	gtk_signal_connect(GTK_OBJECT(m_buttonOK),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_ok_clicked),
			   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(m_buttonCancel),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_cancel_clicked),
			   (gpointer) this);

	// the catch-alls
	
	gtk_signal_connect_after(GTK_OBJECT(m_window),
				 "delete_event",
				 GTK_SIGNAL_FUNC(s_delete_clicked),
				 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(m_window),
				 "destroy",
				 NULL,
				 NULL);

}

GtkWidget * AP_UnixDialog_PageSetup::_constructWindow (void)
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();

  m_window = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (m_window), _(AP, DLG_PageSetup_Title));

  _constructWindowContents (GTK_DIALOG(m_window)->vbox);

  m_buttonOK = gtk_button_new_with_label (_(XAP, DLG_OK));
  gtk_widget_show (m_buttonOK);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(m_window)->action_area), 
		     m_buttonOK);

  m_buttonCancel = gtk_button_new_with_label (_(XAP, DLG_Cancel));
  gtk_widget_show (m_buttonCancel);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(m_window)->action_area), m_buttonCancel);

  _connectSignals ();
  return m_window;
}

// this code is glade generated
void AP_UnixDialog_PageSetup::_constructWindowContents (GtkWidget *container)
{
  GtkWidget *notebook;
  GtkWidget *packerPage;
  GtkWidget *framePaper;
  GtkWidget *tablePaper;
  GtkWidget *entryPageWidth;
  GtkWidget *entryPageHeight;
  GtkWidget *labelWidth;
  GtkWidget *labelHeight;
  GtkWidget *labelPaperSize;
  GtkWidget *optionPageSize;
  GtkWidget *optionPageSize_menu;
  GtkWidget *glade_menuitem;
  GtkWidget *labelPageUnits;
  GtkWidget *optionPageUnits;
  GtkWidget *optionPageUnits_menu;
  GtkWidget *frameOrientation;
  GtkWidget *tableOrientation;
  GSList *tableOrientation_group = NULL;
  GtkWidget *radioPageLandscape;
  GtkWidget *radioPagePortrait;
  GtkWidget *pixmapPortrait;
  GtkWidget *pixmap2;
  GtkWidget *frameScale;
  GtkWidget *table1;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkObject *spinPageScale_adj;
  GtkWidget *spinPageScale;
  GtkWidget *labelPage;
  GtkWidget *tableMargin;
  GtkObject *spinMarginBottom_adj;
  GtkWidget *spinMarginBottom;
  GtkObject *spinMarginFooter_adj;
  GtkWidget *spinMarginFooter;
  GtkWidget *labelMarginUnits;
  GtkWidget *labelTop;
  GtkWidget *labelHeader;
  GtkWidget *labelFooter;
  GtkWidget *labelBottom;
  GtkWidget *customPreview;
  GtkObject *spinMarginTop_adj;
  GtkWidget *spinMarginTop;
  GtkObject *spinMarginHeader_adj;
  GtkWidget *spinMarginHeader;
  GtkWidget *vbox3;
  GtkWidget *labelRight;
  GtkObject *spinMarginRight_adj;
  GtkWidget *spinMarginRight;
  GtkWidget *vbox2;
  GtkWidget *labelLeft;
  GtkObject *spinMarginLeft_adj;
  GtkWidget *spinMarginLeft;
  GtkWidget *optionMarginUnits;
  GtkWidget *optionMarginUnits_menu;
  GtkWidget *labelMargin;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);
  gtk_box_pack_start (GTK_BOX (container), notebook, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 7);

  packerPage = gtk_packer_new ();
  gtk_packer_set_default_border_width (GTK_PACKER (packerPage), 2);
  gtk_packer_set_default_pad (GTK_PACKER (packerPage), 8, 8);
  gtk_packer_set_default_ipad (GTK_PACKER (packerPage), 2, 2);
  gtk_widget_show (packerPage);
  gtk_container_add (GTK_CONTAINER (notebook), packerPage);
  gtk_container_set_border_width (GTK_CONTAINER (packerPage), 2);

  framePaper = gtk_frame_new (_(AP, DLG_PageSetup_Paper));
  gtk_widget_show (framePaper);
  gtk_packer_add_defaults (GTK_PACKER (packerPage), framePaper, GTK_SIDE_TOP,
                           GTK_ANCHOR_CENTER, (GtkPackerOptions) (GTK_FILL_X));

  tablePaper = gtk_table_new (3, 4, TRUE);
  gtk_widget_show (tablePaper);
  gtk_container_add (GTK_CONTAINER (framePaper), tablePaper);
  gtk_table_set_row_spacings (GTK_TABLE (tablePaper), 4);
  gtk_table_set_col_spacings (GTK_TABLE (tablePaper), 4);

  entryPageWidth = create_entry (getPageSize().Width (getPageUnits ()));
  gtk_widget_show (entryPageWidth);
  gtk_table_attach (GTK_TABLE (tablePaper), entryPageWidth, 3, 4, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
 
  entryPageHeight = create_entry (getPageSize().Height (getPageUnits ()));
  gtk_widget_show (entryPageHeight);
  gtk_table_attach (GTK_TABLE (tablePaper), entryPageHeight, 3, 4, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  labelWidth = gtk_label_new (_(AP, DLG_PageSetup_Width));
  gtk_widget_show (labelWidth);
  gtk_table_attach (GTK_TABLE (tablePaper), labelWidth, 2, 3, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  labelHeight = gtk_label_new (_(AP, DLG_PageSetup_Height));
  gtk_widget_show (labelHeight);
  gtk_table_attach (GTK_TABLE (tablePaper), labelHeight, 2, 3, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  labelPaperSize = gtk_label_new (_(AP, DLG_PageSetup_Paper_Size));
  gtk_widget_show (labelPaperSize);
  gtk_table_attach (GTK_TABLE (tablePaper), labelPaperSize, 0, 1, 0, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  optionPageSize = gtk_option_menu_new ();
  gtk_widget_show (optionPageSize);
  gtk_table_attach (GTK_TABLE (tablePaper), optionPageSize, 1, 2, 0, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  optionPageSize_menu = gtk_menu_new ();

  // create the drop-down menu with all of our supported page sizes
  for (int i = (int)fp_PageSize::A0; i < (int)fp_PageSize::_last_predefined_pagesize_dont_use_; i++)
    {
      glade_menuitem = gtk_menu_item_new_with_label (fp_PageSize::PredefinedToName ((fp_PageSize::Predefined)i));
      CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (glade_menuitem, optionPageSize, i, s_page_size_changed);
      gtk_widget_show (glade_menuitem);
      gtk_menu_append (GTK_MENU (optionPageSize_menu), glade_menuitem);
    }
  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionPageSize), optionPageSize_menu);
  last_page_size = fp_PageSize::NameToPredefined (getPageSize ().getPredefinedName ());
  gtk_option_menu_set_history (GTK_OPTION_MENU (optionPageSize), (int) last_page_size);

  labelPageUnits = gtk_label_new (_(AP, DLG_PageSetup_Units));
  gtk_widget_show (labelPageUnits);
  gtk_table_attach (GTK_TABLE (tablePaper), labelPageUnits, 2, 3, 2, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  optionPageUnits = gtk_option_menu_new ();
  gtk_widget_show (optionPageUnits);
  gtk_table_attach (GTK_TABLE (tablePaper), optionPageUnits, 3, 4, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  optionPageUnits_menu = gtk_menu_new ();

  glade_menuitem = gtk_menu_item_new_with_label (_(XAP, DLG_Unit_inch));
  CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (glade_menuitem, optionPageUnits, fp_PageSize::inch, s_page_units_changed);
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionPageUnits_menu), glade_menuitem);

  glade_menuitem = gtk_menu_item_new_with_label (_(XAP, DLG_Unit_cm));
  CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (glade_menuitem, optionPageUnits, fp_PageSize::cm, s_page_units_changed);
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionPageUnits_menu), glade_menuitem);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionPageUnits), optionPageUnits_menu);
  last_page_unit = getPageUnits ();
  gtk_option_menu_set_history (GTK_OPTION_MENU (optionPageUnits), fp_2_pos (last_page_unit));

  frameOrientation = gtk_frame_new (_(AP, DLG_PageSetup_Orient));
  gtk_widget_show (frameOrientation);
  gtk_packer_add_defaults (GTK_PACKER (packerPage), frameOrientation, GTK_SIDE_TOP,
                           GTK_ANCHOR_CENTER, (GtkPackerOptions) (GTK_FILL_X));

  tableOrientation = gtk_table_new (2, 2, TRUE);
  gtk_widget_show (tableOrientation);
  gtk_container_add (GTK_CONTAINER (frameOrientation), tableOrientation);
  gtk_table_set_row_spacings (GTK_TABLE (tableOrientation), 1);

  radioPageLandscape = gtk_radio_button_new_with_label (tableOrientation_group, _(AP, DLG_PageSetup_Landscape));
  tableOrientation_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radioPageLandscape));
  gtk_widget_show (radioPageLandscape);
  gtk_table_attach (GTK_TABLE (tableOrientation), radioPageLandscape, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);

  radioPagePortrait = gtk_radio_button_new_with_label (tableOrientation_group, _(AP, DLG_PageSetup_Portrait));
  tableOrientation_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radioPagePortrait));
  gtk_widget_show (radioPagePortrait);
  gtk_table_attach (GTK_TABLE (tableOrientation), radioPagePortrait, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);

  pixmapPortrait = create_pixmap (container, orient_vertical_xpm);
  gtk_widget_show (pixmapPortrait);
  gtk_table_attach (GTK_TABLE (tableOrientation), pixmapPortrait, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  pixmap2 = create_pixmap (container, orient_horizontal_xpm);
  gtk_widget_show (pixmap2);
  gtk_table_attach (GTK_TABLE (tableOrientation), pixmap2, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  if (getPageOrientation () == PORTRAIT)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radioPagePortrait), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radioPageLandscape), TRUE);;

  frameScale = gtk_frame_new (_(AP, DLG_PageSetup_Scale));
  gtk_widget_show (frameScale);
  gtk_packer_add_defaults (GTK_PACKER (packerPage), frameScale, GTK_SIDE_TOP,
                           GTK_ANCHOR_CENTER, (GtkPackerOptions) (GTK_FILL_X));

  table1 = gtk_table_new (1, 4, TRUE);
  gtk_widget_show (table1);
  gtk_container_add (GTK_CONTAINER (frameScale), table1);

  label5 = gtk_label_new (_(AP, DLG_PageSetup_Adjust));
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  label6 = gtk_label_new (_(AP, DLG_PageSetup_Percent));
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table1), label6, 2, 4, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_FILL);
  gtk_misc_set_alignment (GTK_MISC (label6), 7.45058e-09, 0.5);
  gtk_misc_set_padding (GTK_MISC (label6), 8, 0);

  spinPageScale_adj = gtk_adjustment_new (100, 1, 1000, 1, 25, 25);
  spinPageScale = gtk_spin_button_new (GTK_ADJUSTMENT (spinPageScale_adj), 1, 0);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinPageScale), (float)getPageScale ());
  gtk_widget_show (spinPageScale);
  gtk_table_attach (GTK_TABLE (table1), spinPageScale, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  labelPage = gtk_label_new (_(AP, DLG_PageSetup_Page));
  gtk_widget_show (labelPage);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 0), labelPage);

  tableMargin = gtk_table_new (7, 3, FALSE);
  gtk_widget_show (tableMargin);
  gtk_container_add (GTK_CONTAINER (notebook), tableMargin);
  gtk_container_set_border_width (GTK_CONTAINER (tableMargin), 8);

  spinMarginBottom_adj = gtk_adjustment_new (99, 0, 100, 1, 10, 10);
  spinMarginBottom = gtk_spin_button_new (GTK_ADJUSTMENT (spinMarginBottom_adj), 1, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinMarginBottom), getMarginBottom ());
  gtk_widget_show (spinMarginBottom);
  gtk_table_attach (GTK_TABLE (tableMargin), spinMarginBottom, 1, 2, 6, 7,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  spinMarginFooter_adj = gtk_adjustment_new (99, 0, 100, 1, 10, 10);
  spinMarginFooter = gtk_spin_button_new (GTK_ADJUSTMENT (spinMarginFooter_adj), 1, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinMarginFooter), getMarginFooter ());
  gtk_widget_show (spinMarginFooter);
  gtk_table_attach (GTK_TABLE (tableMargin), spinMarginFooter, 2, 3, 6, 7,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  labelMarginUnits = gtk_label_new (_(AP, DLG_PageSetup_Units));
  gtk_widget_show (labelMarginUnits);
  gtk_table_attach (GTK_TABLE (tableMargin), labelMarginUnits, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  labelTop = gtk_label_new (_(AP, DLG_PageSetup_Top));
  gtk_widget_show (labelTop);
  gtk_table_attach (GTK_TABLE (tableMargin), labelTop, 1, 2, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  labelHeader = gtk_label_new (_(AP, DLG_PageSetup_Header));
  gtk_widget_show (labelHeader);
  gtk_table_attach (GTK_TABLE (tableMargin), labelHeader, 2, 3, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  labelFooter = gtk_label_new (_(AP, DLG_PageSetup_Footer));
  gtk_widget_show (labelFooter);
  gtk_table_attach (GTK_TABLE (tableMargin), labelFooter, 2, 3, 5, 6,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  labelBottom = gtk_label_new (_(AP, DLG_PageSetup_Bottom));
  gtk_widget_show (labelBottom);
  gtk_table_attach (GTK_TABLE (tableMargin), labelBottom, 1, 2, 5, 6,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  customPreview = create_pixmap (container, margin_xpm);
  gtk_widget_show (customPreview);
  gtk_table_attach (GTK_TABLE (tableMargin), customPreview, 1, 2, 2, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  spinMarginTop_adj = gtk_adjustment_new (99, 0, 100, 1, 10, 10);
  spinMarginTop = gtk_spin_button_new (GTK_ADJUSTMENT (spinMarginTop_adj), 1, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinMarginTop), getMarginTop ());
  gtk_widget_show (spinMarginTop);
  gtk_table_attach (GTK_TABLE (tableMargin), spinMarginTop, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);

  spinMarginHeader_adj = gtk_adjustment_new (99, 0, 100, 1, 10, 10);
  spinMarginHeader = gtk_spin_button_new (GTK_ADJUSTMENT (spinMarginHeader_adj), 1, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinMarginHeader), getMarginHeader ());
  gtk_widget_show (spinMarginHeader);
  gtk_table_attach (GTK_TABLE (tableMargin), spinMarginHeader, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox3);
  gtk_table_attach (GTK_TABLE (tableMargin), vbox3, 2, 3, 3, 4,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  labelRight = gtk_label_new (_(AP, DLG_PageSetup_Right));
  gtk_widget_show (labelRight);
  gtk_box_pack_start (GTK_BOX (vbox3), labelRight, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (labelRight), 0.5, 1);

  spinMarginRight_adj = gtk_adjustment_new (99, 0, 100, 1, 10, 10);
  spinMarginRight = gtk_spin_button_new (GTK_ADJUSTMENT (spinMarginRight_adj), 1, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinMarginRight), getMarginRight ());
  gtk_widget_show (spinMarginRight);
  gtk_box_pack_start (GTK_BOX (vbox3), spinMarginRight, FALSE, FALSE, 0);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_table_attach (GTK_TABLE (tableMargin), vbox2, 0, 1, 3, 4,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  labelLeft = gtk_label_new (_(AP, DLG_PageSetup_Left));
  gtk_widget_show (labelLeft);
  gtk_box_pack_start (GTK_BOX (vbox2), labelLeft, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (labelLeft), 0.5, 1);

  spinMarginLeft_adj = gtk_adjustment_new (99, 0, 100, 1, 10, 10);
  spinMarginLeft = gtk_spin_button_new (GTK_ADJUSTMENT (spinMarginLeft_adj), 1, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinMarginLeft), getMarginLeft ());
  gtk_widget_show (spinMarginLeft);
  gtk_box_pack_start (GTK_BOX (vbox2), spinMarginLeft, FALSE, FALSE, 0);

  optionMarginUnits = gtk_option_menu_new ();
  gtk_widget_show (optionMarginUnits);
  gtk_table_attach (GTK_TABLE (tableMargin), optionMarginUnits, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);

  optionMarginUnits_menu = gtk_menu_new ();

  glade_menuitem = gtk_menu_item_new_with_label (_(XAP, DLG_Unit_inch));
  CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (glade_menuitem, optionMarginUnits, fp_PageSize::inch, s_margin_units_changed);
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionMarginUnits_menu), glade_menuitem);

  glade_menuitem = gtk_menu_item_new_with_label (_(XAP, DLG_Unit_cm));
  CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (glade_menuitem, optionMarginUnits, fp_PageSize::cm, s_margin_units_changed);
  gtk_widget_show (glade_menuitem);
  gtk_menu_append (GTK_MENU (optionMarginUnits_menu), glade_menuitem);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (optionMarginUnits), optionMarginUnits_menu);
  last_margin_unit = getMarginUnits ();
  gtk_option_menu_set_history (GTK_OPTION_MENU (optionMarginUnits), fp_2_pos (last_margin_unit));

  labelMargin = gtk_label_new (_(AP, DLG_PageSetup_Margin));
  gtk_widget_show (labelMargin);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 1), labelMargin);

  // now set our instance data equal to these widgets
  // so that we can query them later
  m_optionPageSize 		= optionPageSize;
  m_entryPageWidth 		= entryPageWidth;
  m_entryPageHeight		= entryPageHeight;
  m_optionPageUnits		= optionPageUnits;
  m_radioPagePortrait		= radioPagePortrait;
  m_radioPageLandscape	        = radioPageLandscape;
  m_spinPageScale		= spinPageScale;

  m_optionMarginUnits		= optionMarginUnits;
  m_spinMarginTop	      	= spinMarginTop;
  m_spinMarginBottom		= spinMarginBottom;
  m_spinMarginLeft		= spinMarginLeft;
  m_spinMarginRight		= spinMarginRight;
  m_spinMarginHeader		= spinMarginHeader;
  m_spinMarginFooter		= spinMarginFooter;
}
