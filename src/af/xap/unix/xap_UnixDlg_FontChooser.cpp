/* AbiSource Application Framework
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "xap_UnixDialog_FontChooser.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

/*****************************************************************/
AP_Dialog * AP_UnixDialog_FontChooser::static_constructor(AP_DialogFactory * pFactory,
														 AP_Dialog_Id id)
{
	AP_UnixDialog_FontChooser * p = new AP_UnixDialog_FontChooser(pFactory,id);
	return p;
}

AP_UnixDialog_FontChooser::AP_UnixDialog_FontChooser(AP_DialogFactory * pDlgFactory,
												   AP_Dialog_Id id)
	: AP_Dialog_FontChooser(pDlgFactory,id)
{
}

AP_UnixDialog_FontChooser::~AP_UnixDialog_FontChooser(void)
{
}

/*****************************************************************/

static void s_ok_clicked(GtkWidget * widget,
						 AP_Dialog_FontChooser::tAnswer * answer)
{
	*answer = AP_Dialog_FontChooser::a_OK;
	gtk_main_quit();
}

static void s_cancel_clicked(GtkWidget * widget,
							 AP_Dialog_FontChooser::tAnswer * answer)
{
	*answer = AP_Dialog_FontChooser::a_CANCEL;
	gtk_main_quit();
}

/*****************************************************************/

void AP_UnixDialog_FontChooser::buildXLFD(char * buf)
{
	// relevant X properties
	char * 	family;
	char * 	weight;
	char 	slant;
	guint 	size;

	// family
	if (m_pFontFamily)
	{
		if (!UT_stricmp(m_pFontFamily, "Times New Roman"))
			family = "times";
		else
			family = m_pFontFamily;
	}
	else
		family = "*";

	// weight
	if (m_pFontWeight)
		if (!UT_stricmp(m_pFontWeight, "normal"))
			weight = "medium";
		else
			weight = m_pFontWeight;
	else
		family = "*";

	// slant
	if (m_pFontStyle)
	{
		if (!UT_stricmp(m_pFontStyle, "italic"))
			slant = 'i';
		else if (!UT_stricmp(m_pFontStyle, "oblique"))
			slant = 'o';
	    else
			slant = 'r';
	}
	else
		slant = '*';

	// size
	if (m_pFontSize)
	{
		sscanf(m_pFontSize, "%d", &size);
		size *= 10;
	}
	else
		size = 140; // is this the default size?
	
	sprintf(buf, "-*-%s-%s-%c-normal-*-*-%d-75-75-*-*-*-*",
			family, weight, slant, size);
}

void AP_UnixDialog_FontChooser::parseXLFD(char * buf)
{
	// duplicate and tokenize the XLFD
	gchar * cloneString = strdup(buf);
	UT_ASSERT(cloneString);

	// first call gets foundry, which we ignore
	gchar * token = strtok(cloneString, "-");

	// font family name
	if ((token = strtok(NULL, "-")))
	{
		setFontFamily(token);
		m_bChangedFontFamily = UT_TRUE;
	}

	/* weight (X has lots of defined weights, we just
	   cast them to bold or normal for now)
	*/
	if ((token = strtok(NULL, "-")))
	{
		if (!UT_stricmp(token, "black"))
			setFontWeight("bold");
		else if (!UT_stricmp(token, "bold"))
			setFontWeight("bold");
		else if (!UT_stricmp(token, "demibold"))
			setFontWeight("bold");
		else if (!UT_stricmp(token, "medium"))
			setFontWeight("normal");
		else if (!UT_stricmp(token, "regular"))
			setFontWeight("normal");
		else
			setFontWeight("normal");
		m_bChangedFontWeight = UT_TRUE;
	}
	
	// slant (X has i,o,r, which we cast to italic or normal)
	if ((token = strtok(NULL, "-")))
	{
		if (!UT_stricmp(token, "i"))
			setFontStyle("italic");
		else if (!UT_stricmp(token, "o"))
			setFontStyle("oblique");
		else // o and r
			setFontStyle("normal");
		m_bChangedFontStyle = UT_TRUE;
	}

	// sWidth
	strtok(NULL, "-");
	// adStyle
	strtok(NULL, "-");

	// pxlStyle
	strtok(NULL, "-");

	// point size
	char buf_size[5];
	if ((token = strtok(NULL, "-")))
	{
		sprintf(buf_size, "%dpt", (atoi(token) / 10));
		setFontSize(buf_size);
		m_bChangedFontSize = UT_TRUE;
	}

	if (cloneString)
		free(cloneString);
}

	
void AP_UnixDialog_FontChooser::runModal(AP_Frame * pFrame)
{
	m_pUnixFrame = (AP_UnixFrame *)pFrame;
	UT_ASSERT(m_pUnixFrame);
	AP_UnixApp * pApp = (AP_UnixApp *)m_pUnixFrame->getApp();
	UT_ASSERT(pApp);

	UT_DEBUGMSG(("FontChooserStart: Family[%s] Size[%s] Weight[%s] Style[%s] Color[%s] Underline[%d] StrikeOut[%d]\n",
				 ((m_pFontFamily) ? m_pFontFamily : ""),
				 ((m_pFontSize) ? m_pFontSize : ""),
				 ((m_pFontWeight) ? m_pFontWeight : ""),
				 ((m_pFontStyle) ? m_pFontStyle : ""),
				 ((m_pColor) ? m_pColor : "" ),
				 (m_bUnderline),
				 (m_bStrikeOut)));

	// These define the color element offsets in a vector
	guint RED = 0;
	guint GREEN = 1;
	guint BLUE = 2;
	gdouble currentColor[3] = { 0, 0, 0 };

	GtkFontSelectionDialog * cf;
	gchar * selectedFont = NULL;

	// TODO move this title to resources?
	cf = (GtkFontSelectionDialog *) gtk_font_selection_dialog_new("Font Selection");

	// To match the Windows dialog, we add a "color" tab to the font dialog
	// This is built up top to satisfy the requirement that these widgets
	// exist to set their properties.  :)
	GtkWidget * colorSelector = gtk_color_selection_new();
	UT_ASSERT(colorSelector);
	gtk_widget_show(colorSelector);

	// Padded with spaces to fake min size without gtk_widget_set_usize()
	GtkWidget * tabLabel = gtk_label_new("        Color        ");
	UT_ASSERT(tabLabel);
	gtk_widget_show(tabLabel);
	
	GtkFontSelection * fontsel = GTK_FONT_SELECTION(cf->fontsel);
	UT_ASSERT(fontsel);

	gtk_notebook_insert_page(&fontsel->notebook,
							 colorSelector,
							 tabLabel,
							 3); // 0 based index

    // Connect the signals to the buttons
	gtk_signal_connect(GTK_OBJECT(cf->ok_button),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   &m_answer);
	gtk_signal_connect(GTK_OBJECT(cf->cancel_button),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   &m_answer);
	// TIP: bind something (null at least) to "destroy" or you'll
	// never get out of gtk_main();
	gtk_signal_connect(GTK_OBJECT(cf),
					   "destroy",
					   GTK_SIGNAL_FUNC(NULL),
					   NULL);

	// build an XLFD to try out
	gchar * buf = new char[1024];	// anyone know the max size for an XLFD?
	UT_ASSERT(buf);

	// suck member variables into a buffer in XLFD format
	buildXLFD(buf);

	UT_DEBUGMSG(("Priming dialog with XLFD: [%s]\n", buf));

	// Set color in the color selector, since this can't be done via XLFD
	{
		UT_RGBColor c;
		UT_parseColor(m_pColor, c);

		currentColor[RED] = ((gdouble) c.m_red / (gdouble) 255.0);
		currentColor[GREEN] = ((gdouble) c.m_grn / (gdouble) 255.0);
		currentColor[BLUE] = ((gdouble) c.m_blu / (gdouble) 255.0);

		gtk_color_selection_set_color(GTK_COLOR_SELECTION(colorSelector), currentColor);
	}

	if (!gtk_font_selection_dialog_set_font_name(cf, buf))
	{
		UT_DEBUGMSG(("Couldn't hint to font selection dialog to use XLFD.  "
					 "This font was not found on this X server.\n"));
	}

	// Set up a nice sample string
	gchar * sampleString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijlkmnopqrstuvwxyz";
	gtk_font_selection_dialog_set_preview_text(cf, (const gchar *) sampleString);

	// Run the dialog
	gtk_window_position(GTK_WINDOW(cf), GTK_WIN_POS_CENTER);
	gtk_widget_show(GTK_WIDGET(cf));
	gtk_grab_add(GTK_WIDGET(cf));

	// To make 8-bit visuals happy, we have to push a new visual on to the GTK
	// visual stack, and push the new dialog's colormap onto the GTK colormap
	// stack.
	// This doesn't seem to work with any target for gdk_window_get_colormap();
/*
	gtk_widget_push_visual(gdk_rgb_get_visual());
	gtk_widget_push_colormap(gdk_window_get_colormap(GTK_WIDGET(cf)->window));
*/
	gtk_main();

	if (m_answer == AP_Dialog_FontChooser::a_OK)
	{

		selectedFont = gtk_font_selection_dialog_get_font_name(cf);

		if (selectedFont)
		{
			UT_DEBUGMSG(("Font selection returned [%s].\n\n", selectedFont));

			// blow XLFD buffer back into member variables
			parseXLFD(selectedFont);

			// Strikeout or underline aren't done via XLFD
			// TODO: do them ?
			
			// Color isn't done via XLFD
			gtk_color_selection_get_color(GTK_COLOR_SELECTION(colorSelector), currentColor);
			
			char buf_color[6];
			sprintf(buf_color, "%02x%02x%02x",
					(unsigned int) (currentColor[RED] 	* (gdouble) 255.0),
					(unsigned int) (currentColor[GREEN]	* (gdouble) 255.0),
					(unsigned int) (currentColor[BLUE] 	* (gdouble) 255.0));

			setColor(buf_color);
			m_bChangedColor = UT_TRUE;
		}
	}

	gtk_widget_destroy (GTK_WIDGET(cf));
/*
	gtk_widget_pop_colormap();
	gtk_widget_pop_visual();
*/
	delete [] buf;
	
	UT_DEBUGMSG(("FontChooserEnd: Family[%s%s] Size[%s%s] Weight[%s%s] Style[%s%s] Color[%s%s] Underline[%d%s] StrikeOut[%d%s]\n",
				 ((m_pFontFamily) ? m_pFontFamily : ""),	((m_bChangedFontFamily) ? "(chg)" : ""),
				 ((m_pFontSize) ? m_pFontSize : ""),		((m_bChangedFontSize) ? "(chg)" : ""),
				 ((m_pFontWeight) ? m_pFontWeight : ""),	((m_bChangedFontWeight) ? "(chg)" : ""),
				 ((m_pFontStyle) ? m_pFontStyle : ""),		((m_bChangedFontStyle) ? "(chg)" : ""),
				 ((m_pColor) ? m_pColor : "" ),				((m_bChangedColor) ? "(chg)" : ""),
				 (m_bUnderline),							((m_bChangedUnderline) ? "(chg)" : ""),
				 (m_bStrikeOut),							((m_bChangedStrikeOut) ? "(chg)" : "")));

	// answer should be set by the appropriate callback
	// the caller can get the answer from getAnswer().

	m_pUnixFrame = NULL;
}
