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
#include "xap_Frame.h"
#include "ap_Strings.h"
#include "ut_string_class.h"
#include "xap_UnixDialogHelper.h"

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
						     NULL, static_cast<gchar **>(data));

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
	const char *s = static_cast<const char *>(szString);

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

static int
fp_2_pos (UT_Dimension u)
{
   switch (u)
    {
    case DIM_CM : return 1;
    case DIM_MM : return 2;
    case DIM_IN :
    default :
      return 0;
    } 
}

/*********************************************************************************/

static UT_Dimension last_margin_unit = DIM_IN; 

/*********************************************************************************/

// a *huge* convenience macro
static char _ev_buf[256];
#ifdef _
#undef _
#endif

inline char * _0(const XAP_StringSet * pSS, XAP_String_Id id)
{
	UT_UTF8String _s;
	pSS->getValueUTF8 (id,_s);
	return _ev_convert (_ev_buf, _s.utf8_str());
} 

#define _(a, x) _0(pSS, a##_STRING_ID_##x)

// string tags to stuff stored in widget data
#define WIDGET_MENU_OPTION_PTR		"menuoptionptr"
#define WIDGET_MENU_VALUE_TAG		"value"

// convenience macro
#define CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(w, m, d, f)				\
        do {												\
                g_object_set_data (G_OBJECT (w), WIDGET_MENU_OPTION_PTR, static_cast<gpointer>(m));                \
                g_object_set_data (G_OBJECT (w), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER(d));                \
	        g_signal_connect (G_OBJECT (w), "activate",	\
                G_CALLBACK (f),		\
                static_cast<gpointer>(this));							\
        } while (0)

/*********************************************************************************/

// static event callbacks

static void s_menu_item_activate (GtkWidget * widget)
{
	GtkWidget *option_menu = static_cast<GtkWidget *>(g_object_get_data (G_OBJECT (widget),
								   WIDGET_MENU_OPTION_PTR));
	UT_ASSERT(option_menu && GTK_IS_OPTION_MENU (option_menu));

	gpointer p = g_object_get_data (G_OBJECT (widget),
					  WIDGET_MENU_VALUE_TAG);

	g_object_set_data (G_OBJECT (option_menu), WIDGET_MENU_VALUE_TAG, p);
}

static void s_Landscape_changed(GtkWidget * w,  AP_UnixDialog_PageSetup *dlg)
{
	UT_ASSERT(w);
	UT_ASSERT(dlg);
	dlg->event_LandscapeChanged();
}

static void s_page_size_changed (GtkWidget * w, GtkWidget * child, 
				 AP_UnixDialog_PageSetup *dlg)
{
  UT_ASSERT(w && dlg);

  fp_PageSize::Predefined pos = (fp_PageSize::Predefined)gtk_list_child_position (GTK_LIST(w), child);
  dlg->event_PageSizeChanged (pos);
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

static void s_entryPageWidth_changed(GtkWidget * widget, AP_UnixDialog_PageSetup *dlg)
{
	UT_ASSERT(widget && dlg);
 	dlg->doWidthEntry();
}

static void s_entryPageHeight_changed(GtkWidget * widget, AP_UnixDialog_PageSetup *dlg)
{
	UT_ASSERT(widget && dlg);
	dlg->doHeightEntry();
}

/*********************************************************************************/

void AP_UnixDialog_PageSetup::_setWidth(const char * buf)
{
	double width = static_cast<double>(atof(buf));
	if(m_PageSize.match(width,10.0))
	{
		return;
	}
	double height = static_cast<double>(m_PageSize.Height(getPageUnits()));
	if( width >= 0.00001)
	{
		if(m_PageSize.isPortrait())
		{
			m_PageSize.Set( width,
							height,
							getPageUnits() );
		}
		else
		{
			m_PageSize.Set( height,
							width,
							getPageUnits() );
		}
	}
}

void AP_UnixDialog_PageSetup::_setHeight(const char * buf)
{
	double height = static_cast<double>(atof(buf));
	if(m_PageSize.match(height,10.0))
	{
		return;
	}
	double width = static_cast<double>(m_PageSize.Width(getPageUnits()));
	if( height >= 0.00001)
	{
		if(m_PageSize.isPortrait())
		{
			m_PageSize.Set( width,
							height,
							getPageUnits() );
		}
		else
		{
			m_PageSize.Set( height,
							width,
							getPageUnits() );
		}
	}
}


void AP_UnixDialog_PageSetup::event_LandscapeChanged(void)
{
	UT_UTF8String sHeight = gtk_entry_get_text(GTK_ENTRY(m_entryPageHeight));
	UT_UTF8String sWidth = gtk_entry_get_text(GTK_ENTRY(m_entryPageWidth));

	_setWidth(sHeight.utf8_str());
	_setHeight(sWidth.utf8_str());
	g_signal_handler_block(G_OBJECT(m_entryPageWidth), m_iEntryPageWidthID);
	g_signal_handler_block(G_OBJECT(m_entryPageHeight), m_iEntryPageHeightID);
	gtk_entry_set_text( GTK_ENTRY(m_entryPageWidth),sHeight.utf8_str() );
	gtk_entry_set_text( GTK_ENTRY(m_entryPageHeight),sWidth.utf8_str() );
	g_signal_handler_unblock(G_OBJECT(m_entryPageWidth), m_iEntryPageWidthID);
	g_signal_handler_unblock(G_OBJECT(m_entryPageHeight), m_iEntryPageHeightID);

  	/* switch layout XPM image */
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioPageLandscape))) {
		gtk_widget_destroy(customPreview);
		customPreview = create_pixmap (m_PageHbox, orient_horizontal_xpm);
		gtk_widget_show (customPreview);
		gtk_box_pack_start (GTK_BOX (m_PageHbox), customPreview, FALSE, FALSE, 0);
		gtk_box_reorder_child (GTK_BOX (m_PageHbox), customPreview, 0);
	} else {
		gtk_widget_destroy(customPreview);
		customPreview = create_pixmap (m_PageHbox, orient_vertical_xpm);
		gtk_widget_show (customPreview);
		gtk_box_pack_start (GTK_BOX (m_PageHbox), customPreview, FALSE, FALSE, 0);
		gtk_box_reorder_child (GTK_BOX (m_PageHbox), customPreview, 0);
	}
}

void AP_UnixDialog_PageSetup::doWidthEntry(void)
{
	UT_UTF8String sAfter = gtk_entry_get_text(GTK_ENTRY(m_entryPageWidth));

	m_PageSize.Set(fp_PageSize::psCustom  , getPageUnits());
	_setWidth(sAfter.utf8_str());
	g_signal_handler_block(G_OBJECT(m_entryPageWidth), m_iEntryPageWidthID);
	int pos = gtk_editable_get_position(GTK_EDITABLE(m_entryPageWidth));
	gtk_entry_set_text( GTK_ENTRY(m_entryPageWidth),sAfter.utf8_str() );
	gtk_editable_set_position(GTK_EDITABLE(m_entryPageWidth), pos);
	g_signal_handler_unblock(G_OBJECT(m_entryPageWidth),m_iEntryPageWidthID);

	m_PageSize.Set(fp_PageSize::psCustom  , getPageUnits());
	_updatePageSizeList();

}

void AP_UnixDialog_PageSetup::doHeightEntry(void)
{
    UT_UTF8String sAfter = gtk_entry_get_text(GTK_ENTRY(m_entryPageHeight));

	m_PageSize.Set(fp_PageSize::psCustom  , getPageUnits());
	_setHeight(sAfter.utf8_str());

	g_signal_handler_block(G_OBJECT(m_entryPageHeight), m_iEntryPageHeightID);
	int pos = gtk_editable_get_position(GTK_EDITABLE(m_entryPageHeight));
	gtk_entry_set_text( GTK_ENTRY(m_entryPageHeight),sAfter.utf8_str() );
	gtk_editable_set_position(GTK_EDITABLE(m_entryPageHeight), pos);
	g_signal_handler_unblock(G_OBJECT(m_entryPageHeight),m_iEntryPageHeightID);
	_updatePageSizeList();
}

/* The paper size may have changed, update the Paper Size listbox */
void AP_UnixDialog_PageSetup::_updatePageSizeList(void)
{
	xxx_UT_DEBUGMSG(("_updatePageSize set to %s \n",m_PageSize.getPredefinedName()));
  gint last_page_size = static_cast<gint>(fp_PageSize::NameToPredefined 
	  (m_PageSize.getPredefinedName ()));

  GtkList * optionPageSizeList = GTK_LIST(GTK_COMBO(m_optionPageSize)->list);
  g_signal_handler_block(G_OBJECT(optionPageSizeList), m_iOptionPageSizeListID);
  gtk_list_select_item (optionPageSizeList, last_page_size);
  g_signal_handler_unblock(G_OBJECT(optionPageSizeList), m_iOptionPageSizeListID);
}

void AP_UnixDialog_PageSetup::event_OK (void)
{
	fp_PageSize fp = m_PageSize;

	if(fp.Width(DIM_IN) < 1.0 || fp.Height(DIM_IN) < 1.0)	
	{
		// "The margins selected are too large to fit on the page."
		// Not quite the right message, but it's pretty close, 
		// and I don't know how to add strings.
		m_pFrame->showMessageBox(AP_STRING_ID_DLG_PageSetup_ErrBigMargins, 
								 XAP_Dialog_MessageBox::b_O,
								 XAP_Dialog_MessageBox::a_OK);
		setAnswer(a_CANCEL);
		return;
	}
	
	setMarginUnits (last_margin_unit);
	setPageUnits (fp.getDims());
	setPageSize (fp);
	setPageOrientation (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioPagePortrait)) ? PORTRAIT : LANDSCAPE);
	setPageScale (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (m_spinPageScale)));

	setMarginTop (gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginTop)));
	setMarginBottom (gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginBottom)));
	setMarginLeft (gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginLeft)));
	setMarginRight (gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginRight)));
	setMarginHeader (gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginHeader)));
	setMarginFooter (gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginFooter)));

	// The window will only close (on an OK click) if the margins
	// fit inside the paper size.
	if ( validatePageSettings() ) {
		setAnswer (a_OK);		
	}
	else {
		// "The margins selected are too large to fit on the page."
		m_pFrame->showMessageBox(AP_STRING_ID_DLG_PageSetup_ErrBigMargins, 
								 XAP_Dialog_MessageBox::b_O,
								 XAP_Dialog_MessageBox::a_OK);
	}
}

void AP_UnixDialog_PageSetup::event_Cancel (void)
{
	setAnswer (a_CANCEL);
}

#define FMT_STRING "%0.2f"
void AP_UnixDialog_PageSetup::event_PageUnitsChanged (void)
{
  UT_Dimension pu = static_cast<UT_Dimension>(GPOINTER_TO_INT (g_object_get_data (G_OBJECT (m_optionPageUnits), 
										   WIDGET_MENU_VALUE_TAG)));

  double width, height;

  fp_PageSize ps = m_PageSize;
  
  // convert values  
  width  = static_cast<double>(ps.Width (pu));
  height = static_cast<double>(ps.Height (pu));

  m_PageSize.Set(width, height, pu);
  setPageUnits(pu);

  // set values
  gchar * val;

  val = g_strdup_printf (FMT_STRING, static_cast<float>(width));
  gtk_entry_set_text (GTK_ENTRY (m_entryPageWidth), val);
  g_free (val);

  val = g_strdup_printf (FMT_STRING, static_cast<float>(height));
  gtk_entry_set_text (GTK_ENTRY (m_entryPageHeight), val);
  g_free (val);
}

void AP_UnixDialog_PageSetup::event_PageSizeChanged (fp_PageSize::Predefined pd)
{
  fp_PageSize ps(pd);
  if( TRUE != gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioPagePortrait)))
  {
	  ps.setLandscape();
  }
  // hmm, we should g_free the old pagesize.
  m_PageSize = ps;
  setPageUnits(ps.getDims());

  // change the units in the dialog, too.
  UT_Dimension new_units = ps.getDims();
  gtk_option_menu_set_history (GTK_OPTION_MENU (m_optionPageUnits), fp_2_pos (new_units));

  float w, h;

  w = ps.Width (new_units);
  h = ps.Height (new_units);
//  if( !ps.isPortrait())
//  {
//	  h = ps.Width (new_units);
//	  w = ps.Height (new_units);
//  }

  if (true /* fp_PageSize::psCustom != pd */)
  {
	  // set entry values for a non-custom pagesize
	  gchar * val;

	  val = g_strdup_printf (FMT_STRING, w);
 	  _setWidth(val);
	  gtk_entry_set_text (GTK_ENTRY (m_entryPageWidth), val);
	  g_free (val);

	  val = g_strdup_printf (FMT_STRING, h);
	  _setHeight(val);
	  gtk_entry_set_text (GTK_ENTRY (m_entryPageHeight), val);
	  g_free (val);
  }
  else
  {
	  ps.Set(atof(gtk_entry_get_text(GTK_ENTRY(m_entryPageWidth))),
			 atof(gtk_entry_get_text(GTK_ENTRY(m_entryPageHeight))),
			 static_cast<UT_Dimension>(GPOINTER_TO_INT (g_object_get_data 
												  (G_OBJECT (m_optionPageUnits), 
						   WIDGET_MENU_VALUE_TAG))));
  }
}

void AP_UnixDialog_PageSetup::event_MarginUnitsChanged (void)
{
  UT_Dimension mu = static_cast<UT_Dimension>(GPOINTER_TO_INT (g_object_get_data (G_OBJECT (m_optionMarginUnits),
										   WIDGET_MENU_VALUE_TAG)));

  float top, bottom, left, right, header, footer;

  top    = gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginTop));
  bottom = gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginBottom));
  left   = gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginLeft));
  right  = gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginRight));
  header = gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginHeader));
  footer = gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_spinMarginFooter));

  top = UT_convertDimensions (top,    last_margin_unit, mu);
  bottom = UT_convertDimensions (bottom, last_margin_unit, mu);
  left = UT_convertDimensions (left,   last_margin_unit, mu);
  right = UT_convertDimensions (right,  last_margin_unit, mu);
  header = UT_convertDimensions (header, last_margin_unit, mu);
  footer = UT_convertDimensions (footer, last_margin_unit, mu);

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
	: AP_Dialog_PageSetup (pDlgFactory, id),
    m_PageSize(fp_PageSize::psLetter)
{
  // nada
}

AP_UnixDialog_PageSetup::~AP_UnixDialog_PageSetup (void)
{
  // nada
}

void AP_UnixDialog_PageSetup::runModal (XAP_Frame *pFrame)
{
	UT_return_if_fail(pFrame);
	
	// snarf the parent pagesize.
	m_PageSize = getPageSize();
	m_pFrame = pFrame;

	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);
	m_PageSize = getPageSize();
	_updatePageSizeList();
	switch(abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this,
							 BUTTON_CANCEL, false))
	{
		case BUTTON_OK:
			event_OK() ; break;
		default:
			event_Cancel() ; break ;
	}

	abiDestroyWidget ( mainWindow ) ;
}

void AP_UnixDialog_PageSetup::_connectSignals (void)
{
  	// the control buttons
 	m_iEntryPageWidthID = g_signal_connect(G_OBJECT(m_entryPageWidth),
 					   "changed",
 					  G_CALLBACK(s_entryPageWidth_changed),
 					   static_cast<gpointer>(this));

 	m_iEntryPageHeightID = g_signal_connect(G_OBJECT(m_entryPageHeight),
 					   "changed",
 					  G_CALLBACK(s_entryPageHeight_changed),
 					   static_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_radioPageLandscape),
 					   "toggled",
 					  G_CALLBACK(s_Landscape_changed),
 					   static_cast<gpointer>(this));

}

GtkWidget * AP_UnixDialog_PageSetup::_getWidget(const char * szNameBase, UT_sint32 iLevel)
{
	if(m_pXML == NULL)
	{
		return NULL;
	}
	UT_String sLocal = szNameBase;
	if(iLevel > 0)
	{
		UT_String sVal = UT_String_sprintf("%d",iLevel);
		sLocal += sVal;
	}
	return glade_xml_get_widget(m_pXML, sLocal.c_str());
}

void Markup(GtkWidget * widget, const XAP_StringSet * pSS, char *string)
{
	gchar * unixstr = NULL;	// used for conversions
	UT_XML_cloneNoAmpersands(unixstr, string);
	UT_String markupStr(UT_String_sprintf(gtk_label_get_label (GTK_LABEL(widget)), unixstr));
	gtk_label_set_markup (GTK_LABEL(widget), markupStr.c_str());
	FREEP(unixstr);	
}

GtkWidget * AP_UnixDialog_PageSetup::_constructWindow (void)
{  
  // get the path where our glade file is located
  XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
  UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
  glade_path += "/ap_UnixDialog_PageSetup.glade";

  // load the dialog from the glade file
  m_pXML = abiDialogNewFromXML( glade_path.c_str() );
  if (!m_pXML)
  	return NULL;

  const XAP_StringSet * pSS = m_pApp->getStringSet ();
  GList *glist;
  GtkLabel *orientation;

  m_window = _getWidget("ap_UnixDialog_PageSetup");
  m_wHelp = _getWidget("wHelp");

  m_optionPageSize = _getWidget("comboPageSize");
  m_entryPageWidth = _getWidget("wWidthSpin");
  m_entryPageHeight = _getWidget("wHeightSpin");
  m_optionPageUnits = _getWidget("optionPageUnits");
  m_radioPagePortrait = _getWidget("rbPortrait");
  m_radioPageLandscape = _getWidget("rbLandscape");
  m_spinPageScale = _getWidget("wPageScale");

  m_optionMarginUnits = _getWidget("optionMarginUnits");
  m_spinMarginTop = _getWidget("wTopSpin");
  m_spinMarginBottom = _getWidget("wBottomSpin");
  m_spinMarginLeft = _getWidget("wLeftSpin");
  m_spinMarginRight = _getWidget("wRightSpin");
  m_spinMarginHeader = _getWidget("wHeaderSpin");
  m_spinMarginFooter = _getWidget("wFooterSpin");

  m_MarginHbox = _getWidget("hbox15");
  m_PageHbox = _getWidget("hbox16");

  /* required for translations */
  gtk_label_set_text (GTK_LABEL (_getWidget("lbPage")), _(AP, DLG_PageSetup_Page));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbMargin")), _(AP, DLG_PageSetup_Margin));
  Markup (_getWidget("lbPaper"), pSS, _(AP, DLG_PageSetup_Paper));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbPaperSize")), _(AP, DLG_PageSetup_Paper_Size));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbPageUnits")), _(AP, DLG_PageSetup_Units));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbWidth")), _(AP, DLG_PageSetup_Width));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbHeight")), _(AP, DLG_PageSetup_Height));
  Markup (_getWidget("lbOrientation"), pSS, _(AP, DLG_PageSetup_Orient));
  
  /* radio button labels */
  glist = gtk_container_get_children (GTK_CONTAINER (m_radioPagePortrait));
  orientation = GTK_LABEL (g_list_nth_data (glist, 0));
  gtk_label_set_text (GTK_LABEL (orientation), _(AP, DLG_PageSetup_Portrait));
  
  glist = gtk_container_get_children (GTK_CONTAINER (m_radioPageLandscape));
  orientation = GTK_LABEL (g_list_nth_data (glist, 0));
  gtk_label_set_text (GTK_LABEL (orientation), _(AP, DLG_PageSetup_Landscape));
 
  Markup (_getWidget("lbScale"), pSS, _(AP, DLG_PageSetup_Scale));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbAdjust")), _(AP, DLG_PageSetup_Adjust));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbPercentNormalSize")), _(AP, DLG_PageSetup_Percent));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbMarginUnits")), _(AP, DLG_PageSetup_Units));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbTop")), _(AP, DLG_PageSetup_Top));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbRight")), _(AP, DLG_PageSetup_Right));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbLeft")), _(AP, DLG_PageSetup_Left));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbBottom")), _(AP, DLG_PageSetup_Bottom));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbHeader")), _(AP, DLG_PageSetup_Header));
  gtk_label_set_text (GTK_LABEL (_getWidget("lbFooter")), _(AP, DLG_PageSetup_Footer));
  /* end translation req */

  /* setup page width and height */
  if (!getPageOrientation () == PORTRAIT)
  {
	  m_PageSize.setLandscape();
  }
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_entryPageWidth), m_PageSize.Width (getPageUnits ()));
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_entryPageHeight), m_PageSize.Height (getPageUnits ()));
  
  /* setup margin numbers */
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginTop), getMarginTop ());
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginBottom), getMarginBottom ());
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginLeft), getMarginLeft ());
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginRight), getMarginRight ());
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginHeader), getMarginHeader ());
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinMarginFooter), getMarginFooter ());
  
  /* setup scale number */
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_spinPageScale), static_cast<float>(getPageScale ()));

  // create the drop-down menu with all of our supported page sizes
  GList *popdown_items = NULL;
  for (int i = static_cast<int>(fp_PageSize::_first_predefined_pagesize_); i < static_cast<int>(fp_PageSize::_last_predefined_pagesize_dont_use_); i++)
      popdown_items = g_list_append (popdown_items, const_cast<char *>(fp_PageSize::PredefinedToName ((fp_PageSize::Predefined)i)) );
  gtk_combo_set_popdown_strings (GTK_COMBO (m_optionPageSize), popdown_items);
  GtkList * optionPageSizeList = GTK_LIST(GTK_COMBO(m_optionPageSize)->list);
  m_iOptionPageSizeListID = g_signal_connect(G_OBJECT(optionPageSizeList), "select-child",  G_CALLBACK(s_page_size_changed), static_cast<gpointer>(this));

  /* setup page units menu */
  optionPageUnits_menu = gtk_menu_new ();

  glade_menuitem = gtk_menu_item_new_with_label (_(XAP, DLG_Unit_inch));
  CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (glade_menuitem, m_optionPageUnits, DIM_IN, s_page_units_changed);
  gtk_widget_show (glade_menuitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (optionPageUnits_menu), glade_menuitem);

  glade_menuitem = gtk_menu_item_new_with_label (_(XAP, DLG_Unit_cm));
  CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (glade_menuitem, m_optionPageUnits, DIM_CM, s_page_units_changed);
  gtk_widget_show (glade_menuitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (optionPageUnits_menu), glade_menuitem);

  glade_menuitem = gtk_menu_item_new_with_label (_(XAP, DLG_Unit_mm));
  CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (glade_menuitem, m_optionPageUnits, DIM_MM, s_page_units_changed);
  gtk_widget_show (glade_menuitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (optionPageUnits_menu), glade_menuitem);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (m_optionPageUnits), optionPageUnits_menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (m_optionPageUnits), fp_2_pos (getPageUnits()));

  /* setup margin units menu */
  optionMarginUnits_menu = gtk_menu_new ();
  
  glade_menuitem = gtk_menu_item_new_with_label (_(XAP, DLG_Unit_inch));
  CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (glade_menuitem, m_optionMarginUnits, DIM_IN, s_margin_units_changed);
  gtk_widget_show (glade_menuitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (optionMarginUnits_menu), glade_menuitem);

  glade_menuitem = gtk_menu_item_new_with_label (_(XAP, DLG_Unit_cm));
  CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (glade_menuitem, m_optionMarginUnits, DIM_CM, s_margin_units_changed);
  gtk_widget_show (glade_menuitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (optionMarginUnits_menu), glade_menuitem);

  glade_menuitem = gtk_menu_item_new_with_label (_(XAP, DLG_Unit_mm));
  CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (glade_menuitem, m_optionMarginUnits, DIM_MM, s_margin_units_changed);
  gtk_widget_show (glade_menuitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (optionMarginUnits_menu), glade_menuitem);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (m_optionMarginUnits), optionMarginUnits_menu);
  last_margin_unit = getMarginUnits ();
  gtk_option_menu_set_history (GTK_OPTION_MENU (m_optionMarginUnits), fp_2_pos(last_margin_unit));

  /* add margin XPM image to the margin window */
  customPreview = create_pixmap (m_MarginHbox, margin_xpm);
  gtk_widget_show (customPreview);
  gtk_box_pack_start (GTK_BOX (m_MarginHbox), customPreview, FALSE, FALSE, 0);
  
  /* add correct page XPM image to the page window */
  if (getPageOrientation () == PORTRAIT) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_radioPagePortrait), TRUE);

    customPreview = create_pixmap (m_PageHbox, orient_vertical_xpm);
    gtk_widget_show (customPreview);
    gtk_box_pack_start (GTK_BOX (m_PageHbox), customPreview, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (m_PageHbox), customPreview, 0);
  } else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_radioPageLandscape), TRUE);

    customPreview = create_pixmap (m_PageHbox, orient_horizontal_xpm);
    gtk_widget_show (customPreview);
    gtk_box_pack_start (GTK_BOX (m_PageHbox), customPreview, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (m_PageHbox), customPreview, 0);
  }

  abiAddStockButton(GTK_DIALOG(m_window), GTK_STOCK_CANCEL, BUTTON_CANCEL);
  abiAddStockButton(GTK_DIALOG(m_window), GTK_STOCK_OK, BUTTON_OK);
  _connectSignals ();

  return m_window;
}
