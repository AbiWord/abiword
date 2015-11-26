/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere
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


#include "ut_assert.h"
#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "ap_Strings.h"
#include "ut_string_class.h"
#include "xap_UnixDialogHelper.h"
#include "xap_GtkSignalBlocker.h"
#include "xap_GtkComboBoxHelpers.h"
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
create_pixmap (const char **data)
{
	GtkWidget *pixmap;
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data(data);
	pixmap = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref (pixbuf);
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
	std::string _s;
	pSS->getValueUTF8 (id,_s);
	return _ev_convert (_ev_buf, _s.c_str());
} 

#define _(a, x) _0(pSS, a##_STRING_ID_##x)


/*********************************************************************************/

// static event callbacks

static void s_Landscape_changed(GtkWidget * w,  AP_UnixDialog_PageSetup *dlg)
{
	UT_return_if_fail(w && dlg);
	dlg->event_LandscapeChanged();
}

static void s_page_size_changed (GtkWidget * w, AP_UnixDialog_PageSetup *dlg)
{
	UT_return_if_fail(w && dlg);
	fp_PageSize::Predefined pos = (fp_PageSize::Predefined)gtk_combo_box_get_active(GTK_COMBO_BOX(w));
	dlg->event_PageSizeChanged (pos);
}

static void s_page_units_changed (GtkWidget * w, AP_UnixDialog_PageSetup *dlg)
{
	UT_return_if_fail(w && dlg);
	dlg->event_PageUnitsChanged ();
}

static void s_margin_units_changed (GtkWidget * w, AP_UnixDialog_PageSetup *dlg)
{
	UT_return_if_fail(w && dlg);
	dlg->event_MarginUnitsChanged ();
}

static void s_entryPageWidth_changed(GtkWidget * w, AP_UnixDialog_PageSetup *dlg)
{
	UT_return_if_fail(w && dlg);
 	dlg->doWidthEntry();
}

static void s_entryPageHeight_changed(GtkWidget * w, AP_UnixDialog_PageSetup *dlg)
{
	UT_return_if_fail(w && dlg);
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
	std::string sHeight = gtk_entry_get_text(GTK_ENTRY(m_entryPageHeight));
	std::string sWidth = gtk_entry_get_text(GTK_ENTRY(m_entryPageWidth));

	_setWidth(sHeight.c_str());
	_setHeight(sWidth.c_str());
	g_signal_handler_block(G_OBJECT(m_entryPageWidth), m_iEntryPageWidthID);
	g_signal_handler_block(G_OBJECT(m_entryPageHeight), m_iEntryPageHeightID);
	gtk_entry_set_text( GTK_ENTRY(m_entryPageWidth),sHeight.c_str() );
	gtk_entry_set_text( GTK_ENTRY(m_entryPageHeight),sWidth.c_str() );
	g_signal_handler_unblock(G_OBJECT(m_entryPageWidth), m_iEntryPageWidthID);
	g_signal_handler_unblock(G_OBJECT(m_entryPageHeight), m_iEntryPageHeightID);

  	/* switch layout XPM image */
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioPageLandscape)))
	{
		gtk_widget_destroy(customPreview);
		customPreview = create_pixmap (orient_horizontal_xpm);
		gtk_widget_show (customPreview);
		gtk_box_pack_start (GTK_BOX (m_PageHbox), customPreview, FALSE, FALSE, 0);
		gtk_box_reorder_child (GTK_BOX (m_PageHbox), customPreview, 0);
	}
	else
	{
		gtk_widget_destroy(customPreview);
		customPreview = create_pixmap (orient_vertical_xpm);
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
	{
		XAP_GtkSignalBlocker b(G_OBJECT(m_entryPageWidth), m_iEntryPageWidthID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_entryPageWidth));
		gtk_entry_set_text( GTK_ENTRY(m_entryPageWidth),sAfter.utf8_str() );
		gtk_editable_set_position(GTK_EDITABLE(m_entryPageWidth), pos);
	}
	m_PageSize.Set(fp_PageSize::psCustom  , getPageUnits());
	_updatePageSizeList();
}

void AP_UnixDialog_PageSetup::doHeightEntry(void)
{
    UT_UTF8String sAfter = gtk_entry_get_text(GTK_ENTRY(m_entryPageHeight));

	m_PageSize.Set(fp_PageSize::psCustom  , getPageUnits());
	_setHeight(sAfter.utf8_str());

	{
		XAP_GtkSignalBlocker b(G_OBJECT(m_entryPageHeight), m_iEntryPageHeightID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_entryPageHeight));
		gtk_entry_set_text( GTK_ENTRY(m_entryPageHeight),sAfter.utf8_str() );
		gtk_editable_set_position(GTK_EDITABLE(m_entryPageHeight), pos);
	}
	_updatePageSizeList();
}

/* The paper size may have changed, update the Paper Size listbox */
void AP_UnixDialog_PageSetup::_updatePageSizeList(void)
{
	UT_DEBUGMSG(("AP_UnixDialog_PageSetup::_updatePageSize set to %s \n", m_PageSize.getPredefinedName()));
	gint page_index = static_cast<gint>(fp_PageSize::NameToPredefined 
						(m_PageSize.getPredefinedName ()));

	XAP_GtkSignalBlocker b(G_OBJECT(m_comboPageSize), m_iComboPageSizeListID);
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_comboPageSize), page_index);
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
	else
	{
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
	UT_Dimension pu = static_cast<UT_Dimension>(XAP_comboBoxGetActiveInt(
													GTK_COMBO_BOX(m_optionPageUnits)));

	double width, height;

	fp_PageSize ps = m_PageSize;

	// convert values  
	width  = static_cast<double>(ps.Width (pu));
	height = static_cast<double>(ps.Height (pu));

	if(m_PageSize.isPortrait())
	{
	    m_PageSize.Set(width, height, pu);
	}
	else
	{
	    m_PageSize.Set(height,width, pu);
	}
	// set values
	gchar * val;
	{
	  XAP_GtkSignalBlocker b(G_OBJECT(m_entryPageWidth), m_iEntryPageWidthID);
	  val = g_strdup_printf (FMT_STRING, static_cast<float>(width));
	  gtk_entry_set_text (GTK_ENTRY (m_entryPageWidth), val);
	  g_free (val);
	}
	{
	  XAP_GtkSignalBlocker C(G_OBJECT(m_entryPageHeight), m_iEntryPageHeightID);
	  val = g_strdup_printf (FMT_STRING, static_cast<float>(height));
	  gtk_entry_set_text (GTK_ENTRY (m_entryPageHeight), val);
	  g_free (val);
	}
	setPageUnits(pu);
}

void AP_UnixDialog_PageSetup::event_PageSizeChanged (fp_PageSize::Predefined pd)
{
	fp_PageSize ps(pd);
	if( TRUE != gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (m_radioPagePortrait)))
	{
		ps.setLandscape();
	}
	m_PageSize = ps;

	// change the units in the dialog, too.
	UT_Dimension new_units = ps.getDims();
	setPageUnits(new_units);
	XAP_comboBoxSetActiveFromIntCol(GTK_COMBO_BOX (m_optionPageUnits), 1, new_units);

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
	  XAP_GtkSignalBlocker b(G_OBJECT(m_entryPageWidth), m_iEntryPageWidthID);
	  XAP_GtkSignalBlocker c(G_OBJECT(m_entryPageHeight), m_iEntryPageHeightID);
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
	  UT_Dimension dim = (UT_Dimension)XAP_comboBoxGetActiveInt(GTK_COMBO_BOX(m_optionPageUnits));
	  ps.Set(atof(gtk_entry_get_text(GTK_ENTRY(m_entryPageWidth))),
			 atof(gtk_entry_get_text(GTK_ENTRY(m_entryPageHeight))),
			 dim);
  }
}

void AP_UnixDialog_PageSetup::event_MarginUnitsChanged (void)
{
	UT_Dimension mu =  (UT_Dimension)XAP_comboBoxGetActiveInt(GTK_COMBO_BOX(m_optionMarginUnits));

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
    m_pBuilder(NULL),
    m_PageSize(fp_PageSize::psLetter)
{
	// nada
}

AP_UnixDialog_PageSetup::~AP_UnixDialog_PageSetup (void)
{
  if (m_pBuilder)
    g_object_unref(G_OBJECT(m_pBuilder));
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

	g_signal_connect(G_OBJECT(m_optionPageUnits),
					 "changed",
					 G_CALLBACK(s_page_units_changed), this);
	g_signal_connect(G_OBJECT(m_optionMarginUnits),
					 "changed",
					 G_CALLBACK(s_margin_units_changed), this);
}

GtkWidget * AP_UnixDialog_PageSetup::_getWidget(const char * szNameBase, UT_sint32 iLevel)
{
	UT_return_val_if_fail(m_pBuilder, NULL);

	UT_String sLocal = szNameBase;
	if(iLevel > 0)
	{
		UT_String sVal = UT_String_sprintf("%d",iLevel);
		sLocal += sVal;
	}
	return GTK_WIDGET(gtk_builder_get_object(m_pBuilder, sLocal.c_str()));
}

void Markup(GtkWidget * widget, const XAP_StringSet * /*pSS*/, char *string)
{
	gchar * unixstr = NULL;	// used for conversions
	UT_XML_cloneNoAmpersands(unixstr, string);
	UT_String markupStr(UT_String_sprintf(gtk_label_get_label (GTK_LABEL(widget)), unixstr));
	gtk_label_set_markup (GTK_LABEL(widget), markupStr.c_str());
	FREEP(unixstr);	
}

GtkWidget * AP_UnixDialog_PageSetup::_constructWindow (void)
{  
	// load the dialog from the UI file
	m_pBuilder = newDialogBuilder("ap_UnixDialog_PageSetup.ui");

	const XAP_StringSet * pSS = m_pApp->getStringSet ();
	GList *glist;
	GtkLabel *orientation;

	m_window = _getWidget("ap_UnixDialog_PageSetup");
	m_wHelp = _getWidget("wHelp");

	m_comboPageSize = _getWidget("comboPageSize");
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

	m_MarginHbox = _getWidget("grMargin");
	m_PageHbox = _getWidget("hbox16");

	/* required for translations */
	abiDialogSetTitle(m_window, "%s", _(AP, DLG_PageSetup_Title));
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

	// fill the combobox all of our supported page sizes
	GtkListStore* pagesize_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER);
	GtkTreeIter pagesize_iter;
	for (UT_uint32 i = fp_PageSize::_first_predefined_pagesize_; i < fp_PageSize::_last_predefined_pagesize_dont_use_; i++)
	{
		gtk_list_store_append(pagesize_store, &pagesize_iter);
		gtk_list_store_set(pagesize_store, &pagesize_iter,
					0, pSS->getValue(fp_PageSize::PredefinedToLocalName((fp_PageSize::Predefined) i)),
					1, this,
					-1);
	}
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_comboPageSize), GTK_TREE_MODEL(pagesize_store));
	m_iComboPageSizeListID = g_signal_connect(G_OBJECT(m_comboPageSize),
							"changed",
							G_CALLBACK(s_page_size_changed),
							static_cast<gpointer>(this));

	/* setup page units menu */
	GtkComboBox *combo = GTK_COMBO_BOX(m_optionPageUnits);
	XAP_makeGtkComboBoxText(combo, G_TYPE_INT);
	XAP_appendComboBoxTextAndInt(combo, _(XAP, DLG_Unit_inch), DIM_IN);
	XAP_appendComboBoxTextAndInt(combo, _(XAP, DLG_Unit_cm), DIM_CM);
	XAP_appendComboBoxTextAndInt(combo, _(XAP, DLG_Unit_mm), DIM_MM);
    XAP_comboBoxSetActiveFromIntCol(combo, 1, getPageUnits ());

	/* setup margin units menu */
	combo = GTK_COMBO_BOX(m_optionMarginUnits);
	XAP_makeGtkComboBoxText(combo, G_TYPE_INT);
	XAP_appendComboBoxTextAndInt(combo, _(XAP, DLG_Unit_inch), DIM_IN);
	XAP_appendComboBoxTextAndInt(combo, _(XAP, DLG_Unit_cm), DIM_CM);
	XAP_appendComboBoxTextAndInt(combo, _(XAP, DLG_Unit_mm), DIM_MM);
	last_margin_unit = getMarginUnits ();
    XAP_comboBoxSetActiveFromIntCol(combo, 1, last_margin_unit);

	/* add margin XPM image to the margin window */
	customPreview = create_pixmap (margin_xpm);
	gtk_widget_show (customPreview);
	gtk_grid_attach (GTK_GRID (m_MarginHbox), customPreview, 2, 0, 1, 8);

	/* add correct page XPM image to the page window */
	if (getPageOrientation () == PORTRAIT)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_radioPagePortrait), TRUE);

		customPreview = create_pixmap (orient_vertical_xpm);
		gtk_widget_show (customPreview);
		gtk_box_pack_start (GTK_BOX (m_PageHbox), customPreview, FALSE, FALSE, 0);
		gtk_box_reorder_child (GTK_BOX (m_PageHbox), customPreview, 0);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_radioPageLandscape), TRUE);

		customPreview = create_pixmap (orient_horizontal_xpm);
		gtk_widget_show (customPreview);
		gtk_box_pack_start (GTK_BOX (m_PageHbox), customPreview, FALSE, FALSE, 0);
		gtk_box_reorder_child (GTK_BOX (m_PageHbox), customPreview, 0);
	}

	std::string s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel, s);
	abiAddButton(GTK_DIALOG(m_window), s, BUTTON_CANCEL);
	pSS->getValueUTF8(XAP_STRING_ID_DLG_OK, s);
	abiAddButton(GTK_DIALOG(m_window), s, BUTTON_OK);
	_connectSignals ();

	return m_window;
}
