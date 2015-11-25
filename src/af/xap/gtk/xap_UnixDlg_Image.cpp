/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz
 * Copyright (C) 2002 Martin Sevior
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
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Image.h"
#include "xap_UnixDlg_Image.h"

/*****************************************************************/

void XAP_UnixDialog_Image::s_HeightSpin_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doHeightSpin();
}

void XAP_UnixDialog_Image::s_WidthSpin_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doWidthSpin();
}

void XAP_UnixDialog_Image::s_HeightEntry_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doHeightEntry();
}

gboolean XAP_UnixDialog_Image::s_HeightEntry_FocusOut(GtkWidget * widget, GdkEvent  * /*event*/, XAP_UnixDialog_Image *dlg)
{
  if(!(widget && dlg))
    {
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
      return(TRUE);
    }
  dlg->doHeightEntry();
  return(FALSE);
}

gboolean XAP_UnixDialog_Image::s_WidthEntry_FocusOut(GtkWidget * widget, GdkEvent  * /*event*/, XAP_UnixDialog_Image *dlg)
{
  if(!(widget && dlg))
    {
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
      return(TRUE);
    }
  dlg->doWidthEntry();
  return(FALSE);
}

void XAP_UnixDialog_Image::s_WidthEntry_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doWidthEntry();
}

void XAP_UnixDialog_Image::s_aspect_clicked(GtkWidget * widget, XAP_UnixDialog_Image * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->aspectCheckbox();
}

void XAP_UnixDialog_Image::s_wrapping_changed(GtkWidget * widget, XAP_UnixDialog_Image * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->wrappingChanged();
}

void XAP_UnixDialog_Image::wrappingChanged(void)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wrbInLine)))
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbPlaceParagraph),TRUE);
		gtk_widget_set_sensitive(m_wPlaceTable,FALSE);
		gtk_widget_set_sensitive(m_wWrapTable,FALSE);
		gtk_widget_set_sensitive(m_wrbPlaceParagraph,FALSE);
		gtk_widget_set_sensitive(m_wrbPlaceColumn,FALSE);
		gtk_widget_set_sensitive(m_wrbPlacePage,FALSE);
		gtk_widget_set_sensitive(m_wrbSquareWrap,FALSE);
		gtk_widget_set_sensitive(m_wrbTightWrap,FALSE);

		return;
	}
	gtk_widget_set_sensitive(m_wPlaceTable,TRUE);
	gtk_widget_set_sensitive(m_wWrapTable,TRUE);
	gtk_widget_set_sensitive(m_wrbPlaceParagraph,TRUE);
	gtk_widget_set_sensitive(m_wrbPlaceColumn,TRUE);
	gtk_widget_set_sensitive(m_wrbPlacePage,TRUE);
	gtk_widget_set_sensitive(m_wrbSquareWrap,TRUE);
	gtk_widget_set_sensitive(m_wrbTightWrap,TRUE);
}

void XAP_UnixDialog_Image::event_Ok ()
{
	setAnswer(XAP_Dialog_Image::a_OK);
	setTitle (gtk_entry_get_text (GTK_ENTRY(m_wTitleEntry)));
	setDescription (gtk_entry_get_text (GTK_ENTRY(m_wDescriptionEntry)));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wrbInLine)))
	{
		setWrapping(WRAP_INLINE);
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wrbNone)))
	{
		setWrapping(WRAP_NONE);
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wrbWrappedRight)))
	{
		setWrapping(WRAP_TEXTRIGHT);
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wrbWrappedLeft)))
	{
		setWrapping(WRAP_TEXTLEFT);
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wrbWrappedBoth)))
	{
		setWrapping(WRAP_TEXTBOTH);
	}

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wrbPlaceParagraph)))
	{
		setPositionTo(POSITION_TO_PARAGRAPH);
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wrbPlaceColumn)))
	{
		setPositionTo(POSITION_TO_COLUMN);
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wrbPlacePage)))
	{
		setPositionTo(POSITION_TO_PAGE);
	}
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(m_wrbTightWrap)))
	{
	        setTightWrap(true);
	}
	else
	{
	        setTightWrap(false);
	}
}
void XAP_UnixDialog_Image::event_Cancel ()
{  
	setAnswer(XAP_Dialog_Image::a_Cancel);
}

void XAP_UnixDialog_Image::aspectCheckbox()
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON( m_wAspectCheck)) && (m_dHeightWidth > 0.0001))
		m_bAspect = true;
	else
		m_bAspect = false;
	setPreserveAspect( m_bAspect );
}

void XAP_UnixDialog_Image::doHeightSpin(void)
{
	bool bIncrement = true;
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wHeightSpin));
	if (val == m_iHeight)
		return;
	if(val < m_iHeight)
		bIncrement = false;

	m_iHeight = val;
	incrementHeight(bIncrement);
	adjustWidthForAspect();
	gtk_entry_set_text( GTK_ENTRY(m_wHeightEntry),getHeightString() );
}


void XAP_UnixDialog_Image::doWidthSpin(void)
{
	bool bIncrement = true;
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wWidthSpin));
	if (val == m_iWidth)
		return;	
	if(val < m_iWidth)
		bIncrement = false;
	m_iWidth = val;
	incrementWidth(bIncrement);
	adjustHeightForAspect();
	gtk_entry_set_text( GTK_ENTRY(m_wWidthEntry),getWidthString() );
}

void XAP_UnixDialog_Image::doHeightEntry(void)
{
	const char * szHeight = gtk_entry_get_text(GTK_ENTRY(m_wHeightEntry));
	if(UT_determineDimension(szHeight,DIM_none) != DIM_none)
	{
		setHeight(szHeight);

		g_signal_handler_block(G_OBJECT(m_wHeightEntry), m_iHeightID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_wHeightEntry));
		gtk_entry_set_text( GTK_ENTRY(m_wHeightEntry),getHeightString() );
		gtk_editable_set_position(GTK_EDITABLE(m_wHeightEntry), pos);
		g_signal_handler_unblock(G_OBJECT(m_wHeightEntry), m_iHeightID);
	}
	else
	  {
	    gtk_entry_set_text( GTK_ENTRY(m_wHeightEntry),getHeightString() ); 
	  }
	adjustWidthForAspect();
}

void XAP_UnixDialog_Image::setHeightEntry(void)
{
	g_signal_handler_block(G_OBJECT(m_wHeightEntry), m_iHeightID);
	int pos = gtk_editable_get_position(GTK_EDITABLE(m_wHeightEntry));
	gtk_entry_set_text( GTK_ENTRY(m_wHeightEntry),getHeightString() );
	gtk_editable_set_position(GTK_EDITABLE(m_wHeightEntry), pos);
	g_signal_handler_unblock(G_OBJECT(m_wHeightEntry), m_iHeightID);
}

void XAP_UnixDialog_Image::setWidthEntry(void)
{
	g_signal_handler_block(G_OBJECT(m_wWidthEntry), m_iWidthID);
	int pos = gtk_editable_get_position(GTK_EDITABLE(m_wWidthEntry));
	gtk_entry_set_text( GTK_ENTRY(m_wWidthEntry),getWidthString() );
	gtk_editable_set_position(GTK_EDITABLE(m_wWidthEntry), pos);
	g_signal_handler_unblock(G_OBJECT(m_wWidthEntry), m_iWidthID);
}

void XAP_UnixDialog_Image::doWidthEntry(void)
{
	const char * szWidth = gtk_entry_get_text(GTK_ENTRY(m_wWidthEntry));
	if(UT_determineDimension(szWidth,DIM_none) != DIM_none)
	{
		setWidth(szWidth);
		
		g_signal_handler_block(G_OBJECT(m_wWidthEntry), m_iWidthID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_wWidthEntry));
		gtk_entry_set_text( GTK_ENTRY(m_wWidthEntry),getWidthString() );
		gtk_editable_set_position(GTK_EDITABLE(m_wWidthEntry), pos);
		g_signal_handler_unblock(G_OBJECT(m_wWidthEntry), m_iWidthID);
	}
	else
	  {
	    gtk_entry_set_text( GTK_ENTRY(m_wWidthEntry),getWidthString() );
	  }
	adjustHeightForAspect();
}


void XAP_UnixDialog_Image::adjustHeightForAspect(void)
{
	if(m_bAspect)
		setHeightEntry();
}

void XAP_UnixDialog_Image::adjustWidthForAspect(void)
{
	if(m_bAspect)
		setWidthEntry();
}

/***********************************************************************/

XAP_Dialog * XAP_UnixDialog_Image::static_constructor(XAP_DialogFactory * pFactory,
													  XAP_Dialog_Id id)
{
	return new XAP_UnixDialog_Image(pFactory,id);
}

XAP_UnixDialog_Image::XAP_UnixDialog_Image(XAP_DialogFactory * pDlgFactory,
										   XAP_Dialog_Id id)
	: XAP_Dialog_Image(pDlgFactory,id)
{
}

XAP_UnixDialog_Image::~XAP_UnixDialog_Image(void)
{
}

void XAP_UnixDialog_Image::setWrappingGUI()
{
	if(isInHdrFtr() || (getWrapping() == WRAP_INLINE))
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbInLine),TRUE);
		gtk_widget_set_sensitive(m_wrbSquareWrap,FALSE);
		gtk_widget_set_sensitive(m_wrbTightWrap,FALSE);
	}
	else if(getWrapping() == WRAP_TEXTRIGHT)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbWrappedRight),TRUE);
		gtk_widget_set_sensitive(m_wrbSquareWrap,TRUE);
		gtk_widget_set_sensitive(m_wrbTightWrap,TRUE);
	}
	else if(getWrapping() == WRAP_NONE)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbNone),TRUE);
		gtk_widget_set_sensitive(m_wrbSquareWrap,FALSE);
		gtk_widget_set_sensitive(m_wrbTightWrap,FALSE);
	}
	else if(getWrapping() == WRAP_TEXTLEFT)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbWrappedLeft),TRUE);
		gtk_widget_set_sensitive(m_wrbSquareWrap,TRUE);
		gtk_widget_set_sensitive(m_wrbTightWrap,TRUE);
	}
	else if(getWrapping() == WRAP_TEXTBOTH)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbWrappedBoth),TRUE);
		gtk_widget_set_sensitive(m_wrbSquareWrap,TRUE);
		gtk_widget_set_sensitive(m_wrbTightWrap,TRUE);
	}
	if(isInHdrFtr())
	{
	  gtk_widget_set_sensitive(m_wrbWrappedRight,FALSE);
	  gtk_widget_set_sensitive(m_wrbWrappedLeft,FALSE);
	  gtk_widget_set_sensitive(m_wrbWrappedBoth,FALSE);
	  gtk_widget_set_sensitive(m_wrbSquareWrap,FALSE);
	  gtk_widget_set_sensitive(m_wrbTightWrap,FALSE);
	}
	else if(isTightWrap())
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbTightWrap),TRUE);
	}
	else if(!isTightWrap())
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbSquareWrap),TRUE);
	}
}


void XAP_UnixDialog_Image::setPositionToGUI()
{
  if(!isInHdrFtr())
  {
	if(getPositionTo() == POSITION_TO_PARAGRAPH)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbPlaceParagraph),TRUE);
	}
	else if(getPositionTo() == POSITION_TO_COLUMN)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbPlaceColumn),TRUE);
	}
	else if(getPositionTo() == POSITION_TO_PAGE)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbPlacePage),TRUE);
	}
  }
  else
  {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbPlaceParagraph),FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbPlaceColumn),FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wrbPlacePage),FALSE);
    gtk_widget_set_sensitive(m_wPlaceTable,FALSE);
    gtk_widget_set_sensitive(m_wWrapTable,FALSE);
    gtk_widget_set_sensitive(m_wrbPlaceParagraph,FALSE);
    gtk_widget_set_sensitive(m_wrbPlaceColumn,FALSE);
    gtk_widget_set_sensitive(m_wrbPlacePage,FALSE);
  }
}

void XAP_UnixDialog_Image::runModal(XAP_Frame * pFrame)
{
	// build the dialog
	GtkWidget * cf = _constructWindow();
	UT_return_if_fail(cf);	
	
	setHeightEntry();
	setWidthEntry();
	double height = UT_convertToInches(getHeightString());
	double width = UT_convertToInches(getWidthString());
	
	if((height > 0.0001) && (width > 0.0001))
		m_dHeightWidth = height/width;
	else
	{
		m_dHeightWidth = 0.0;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wAspectCheck), FALSE);
	}	  
	
	setWrappingGUI();
	setPositionToGUI();
	wrappingChanged();

	switch ( abiRunModalDialog ( GTK_DIALOG(cf), pFrame, this, BUTTON_CANCEL, false ) )
    {
    case GTK_RESPONSE_OK:
		event_Ok (); break;
    default:
		event_Cancel (); break;
    }

	abiDestroyWidget (cf);
}

void XAP_UnixDialog_Image::_connectSignals (void)
{
  g_signal_connect(G_OBJECT(m_wHeightSpin),
				   "changed",
				   G_CALLBACK(s_HeightSpin_changed),
				   static_cast<gpointer>(this));
  
  m_iHeightID = g_signal_connect(G_OBJECT(m_wHeightEntry),
								 "activate",
								 G_CALLBACK(s_HeightEntry_changed),
								 static_cast<gpointer>(this));

  g_signal_connect_after(G_OBJECT(m_wHeightEntry),
		   "focus_out_event",
		   G_CALLBACK(s_HeightEntry_FocusOut),
		   static_cast<gpointer>(this));

  g_signal_connect_after(G_OBJECT(m_wWidthEntry),
		   "focus_out_event",
		   G_CALLBACK(s_WidthEntry_FocusOut),
		   static_cast<gpointer>(this));



  g_signal_connect(G_OBJECT(m_wWidthSpin),
				   "changed",
				   G_CALLBACK(s_WidthSpin_changed),
				   static_cast<gpointer>(this));

  g_signal_connect(G_OBJECT(m_wrbInLine),
				   "clicked",
				   G_CALLBACK(s_wrapping_changed),
				   static_cast<gpointer>(this));

  g_signal_connect(G_OBJECT(m_wrbNone),
				   "clicked",
				   G_CALLBACK(s_wrapping_changed),
				   static_cast<gpointer>(this));


  g_signal_connect(G_OBJECT(m_wrbWrappedRight),
				   "clicked",
				   G_CALLBACK(s_wrapping_changed),
				   static_cast<gpointer>(this));


  g_signal_connect(G_OBJECT(m_wrbWrappedLeft),
				   "clicked",
				   G_CALLBACK(s_wrapping_changed),
				   static_cast<gpointer>(this));


  g_signal_connect(G_OBJECT(m_wrbWrappedBoth),
				   "clicked",
				   G_CALLBACK(s_wrapping_changed),
				   static_cast<gpointer>(this));


  g_signal_connect(G_OBJECT(m_wrbSquareWrap),
				   "clicked",
				   G_CALLBACK(s_wrapping_changed),
				   static_cast<gpointer>(this));

  g_signal_connect(G_OBJECT(m_wrbTightWrap),
				   "clicked",
				   G_CALLBACK(s_wrapping_changed),
				   static_cast<gpointer>(this));
  
  m_iWidthID = g_signal_connect(G_OBJECT(m_wWidthEntry),
  								"activate",
  								G_CALLBACK(s_WidthEntry_changed),
  								static_cast<gpointer>(this));

  g_signal_connect(G_OBJECT(m_wAspectCheck),
				   "clicked",
				   G_CALLBACK(s_aspect_clicked),
				   static_cast<gpointer>(this));
}

GtkWidget * XAP_UnixDialog_Image::_constructWindow ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	const char * ui_file;

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
    ui_file = "xap_UnixHildonDlg_Image.ui";
#else
    ui_file = "xap_UnixDlg_Image.ui";
#endif

    GtkBuilder * builder = newDialogBuilder(ui_file);
	
	mMainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "xap_UnixDlg_Image"));
	m_wHeightSpin = GTK_WIDGET(gtk_builder_get_object(builder, "sbHeight"));
	m_wHeightEntry = GTK_WIDGET(gtk_builder_get_object(builder, "edHeight"));
	m_wWidthSpin = GTK_WIDGET(gtk_builder_get_object(builder, "sbWidth"));
	m_wWidthEntry = GTK_WIDGET(gtk_builder_get_object(builder, "edWidth"));
	m_wAspectCheck = GTK_WIDGET(gtk_builder_get_object(builder, "cbAspect"));

	m_wTitleEntry = GTK_WIDGET(gtk_builder_get_object(builder, "edTitle"));
	m_wDescriptionEntry = GTK_WIDGET(gtk_builder_get_object(builder, "edDescription"));

	m_bAspect = getPreserveAspect();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_wAspectCheck), m_bAspect);
	
	m_oHeightSpin_adj = (GtkAdjustment*)gtk_adjustment_new( 1,-2000, 2000, 1, 1, 10);
   	gtk_widget_set_size_request(m_wHeightSpin,14,-1);  
	gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON(m_wHeightSpin), GTK_ADJUSTMENT(m_oHeightSpin_adj));
	
	m_oWidthSpin_adj = (GtkAdjustment*)gtk_adjustment_new( 1,-2000, 2000, 1, 1, 10);
	gtk_widget_set_size_request(m_wWidthSpin,14,-1);  
	gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON(m_wWidthSpin), GTK_ADJUSTMENT(m_oWidthSpin_adj));
	
    std::string s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Image_Title,s);
	abiDialogSetTitle(mMainWindow, "%s", s.c_str());

    localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbDescTab")), pSS, XAP_STRING_ID_DLG_Image_DescTabLabel);
    localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbWrapTab")), pSS, XAP_STRING_ID_DLG_Image_WrapTabLabel);
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
    localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbPlacementTab")), pSS, XAP_STRING_ID_DLG_Image_PlacementTabLabel);
#endif

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbSize")), pSS, XAP_STRING_ID_DLG_Image_ImageSize);
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbImageDescription")), pSS, XAP_STRING_ID_DLG_Image_ImageDesc);
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbTextWrapping")), pSS, XAP_STRING_ID_DLG_Image_TextWrapping);
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbImagePlacement")), pSS, XAP_STRING_ID_DLG_Image_Placement);
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbWrapType")), pSS, XAP_STRING_ID_DLG_Image_WrapType);
	
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbHeight")), pSS, XAP_STRING_ID_DLG_Image_Height);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbWidth")), pSS, XAP_STRING_ID_DLG_Image_Width);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbTitle")), pSS, XAP_STRING_ID_DLG_Image_LblTitle);
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbDescription")), pSS, XAP_STRING_ID_DLG_Image_LblDescription);

	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbInLine")), pSS, XAP_STRING_ID_DLG_Image_InLine);
	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbNone")), pSS, XAP_STRING_ID_DLG_Image_WrappedNone);
	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbWrappedRight")), pSS, XAP_STRING_ID_DLG_Image_WrappedRight);
	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbWrappedLeft")), pSS, XAP_STRING_ID_DLG_Image_WrappedLeft);
	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbWrappedBoth")), pSS, XAP_STRING_ID_DLG_Image_WrappedBoth);

	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbPlaceParagraph")), pSS, XAP_STRING_ID_DLG_Image_PlaceParagraph);
	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbPlaceColumn")), pSS, XAP_STRING_ID_DLG_Image_PlaceColumn);
	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbPlacePage")), pSS, XAP_STRING_ID_DLG_Image_PlacePage);

	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbSquareWrap")), pSS, XAP_STRING_ID_DLG_Image_SquareWrap);
	localizeButton(GTK_WIDGET(gtk_builder_get_object(builder, "rbTightWrap")), pSS, XAP_STRING_ID_DLG_Image_TightWrap);

	m_wPlaceTable = GTK_WIDGET(gtk_builder_get_object(builder, "tbPlacement"));
	m_wrbInLine = GTK_WIDGET(gtk_builder_get_object(builder, "rbInLine"));
	m_wrbNone = GTK_WIDGET(gtk_builder_get_object(builder, "rbNone"));
	m_wrbWrappedRight = GTK_WIDGET(gtk_builder_get_object(builder, "rbWrappedRight"));
	m_wrbWrappedLeft = GTK_WIDGET(gtk_builder_get_object(builder, "rbWrappedLeft"));
	m_wrbWrappedBoth = GTK_WIDGET(gtk_builder_get_object(builder, "rbWrappedBoth"));

	m_wrbPlaceParagraph = GTK_WIDGET(gtk_builder_get_object(builder, "rbPlaceParagraph"));
	m_wrbPlaceColumn = GTK_WIDGET(gtk_builder_get_object(builder, "rbPlaceColumn"));
	m_wrbPlacePage = GTK_WIDGET(gtk_builder_get_object(builder, "rbPlacePage"));

	m_wWrapTable = GTK_WIDGET(gtk_builder_get_object(builder, "tbWrapTable"));
	m_wrbSquareWrap = GTK_WIDGET(gtk_builder_get_object(builder, "rbSquareWrap"));
	m_wrbTightWrap = GTK_WIDGET(gtk_builder_get_object(builder, "rbTightWrap"));

	pSS->getValueUTF8 (XAP_STRING_ID_DLG_Image_Aspect,s);
	gtk_button_set_label(GTK_BUTTON(m_wAspectCheck), s.c_str());

	m_iWidth = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wWidthSpin));
	m_iHeight = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wHeightSpin));

	gtk_entry_set_text (GTK_ENTRY(m_wTitleEntry), getTitle().utf8_str());
	gtk_entry_set_text (GTK_ENTRY(m_wDescriptionEntry), getDescription().utf8_str());

	_connectSignals ();
	
	g_object_unref(G_OBJECT(builder));

	return mMainWindow;
}
