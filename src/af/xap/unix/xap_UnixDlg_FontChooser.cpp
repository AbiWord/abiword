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

#include <gtk/gtk.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ap_UnixDialog_FontChooser.h"
#include "ap_UnixApp.h"
#include "ap_UnixFrame.h"

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

	// TODO do the thing....
	
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


#if 0
/*** delete everything below here once we have salvaged what we can ***/
/*****************************************************************/
/*****************************************************************/


#include "ap_UnixApp.h"
#include "ap_UnixFrame.h"
#include "ps_Graphics.h"

static void set_ok (GtkWidget * /*widget*/, UT_Bool *dialog_result)
{
	*dialog_result = TRUE;
	gtk_main_quit();
}

UT_Bool _chooseFont(AP_Frame * pFrame, FV_View * pView)
{
	// These define the color element offsets in a vector
	guint RED = 0;
	guint GREEN = 1;
	guint BLUE = 2;
	gdouble currentColor[3] = { 0, 0, 0 };

	GtkFontSelectionDialog * cf;
	UT_Bool accepted = FALSE;
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
					   GTK_SIGNAL_FUNC(set_ok),
					   &accepted);
	gtk_signal_connect(GTK_OBJECT(cf->cancel_button),
					   "clicked",
					   GTK_SIGNAL_FUNC(gtk_main_quit),
					   NULL);
	// TIP: bind something (null at least) to "destroy" or you'll
	// never get out of gtk_main();
	gtk_signal_connect(GTK_OBJECT(cf),
					   "destroy",
					   GTK_SIGNAL_FUNC(NULL),
					   NULL);

	// Do we really want to position a new window at the cursor?
	//gtk_window_position(GTK_WINDOW(cf), GTK_WIN_POS_MOUSE);

	// We're now ready to query the view for its properties, to set
	// the font dialog's pop-up appearance to match.
	const XML_Char ** props_in = NULL;
	const XML_Char * s;
	DG_Graphics * pG = pView->getLayout()->getGraphics();

	if (!pView->getCharFormat(&props_in))
		return UT_FALSE;

	// TODO set the proper length.
	// This length is completely arbitrary.  If you know the max X font
	// descriptor length, please set this array to that size and check
	// accordingly.
	gchar * fontString = new gchar[1024];
	fontString[0] = NULL;

	// we don't have a "foundry" attribute to match X's, fake it
	strcat(fontString, "-*");

	// family is *,[name]
	s = UT_getAttribute("font-family", props_in);
	if (s)
	{
		// If blank or standard Windows font
		if (!UT_stricmp(s, "Times New Roman") || !UT_stricmp(s, ""))
		{
			// this satisfies the condition the view doesn't have
			// a family set
			strcat(fontString, "-");
			strcat(fontString, "times");
		}
		else
		{
			strcat(fontString, "-");
			strcat(fontString, s);
		}
	}
	else
		// this is kind of redundant, fix it
		strcat(fontString, "-*");

	// weight is *,black,bold,demibold,medium,regular
	s = UT_getAttribute("font-weight", props_in);
	if (s)
	{
		if (!UT_stricmp(s, "bold"))
			strcat(fontString, "-bold");
		else
			strcat(fontString, "-medium");
	}
	else
		strcat(fontString, "-*");

	// slant is *,i,o,r
	s = UT_getAttribute("font-style", props_in);
	if (s)
	{
		if (!UT_stricmp(s, "italic"))
			strcat(fontString, "-i");
		else if (!UT_stricmp(s, "oblique"))
			strcat(fontString, "-o");
		else
			strcat(fontString, "-r");
	}
	else
		strcat(fontString, "-*");

	// sWidth
	strcat(fontString, "-*");
	// adStyle
	strcat(fontString, "-*");

	// pxlsz (we use points)
	strcat(fontString, "-*");

	// point size
	s = UT_getAttribute("font-size", props_in);
	if (s)
	{
		strcat(fontString, "-");
		char fontSize[5];
		snprintf(fontSize, 5, "%d", (pG->convertDimension(s)) * 10);
		strcat(fontString, fontSize);
	}
	else
		// is this the default size?
		strcat(fontString, "-14");

	// Fill in the last 6 attributes we don't touch
	strcat(fontString, "-*");
	strcat(fontString, "-*");
	strcat(fontString, "-*");
	strcat(fontString, "-*");
	strcat(fontString, "-*");
	strcat(fontString, "-*");

	UT_DEBUGMSG(("Priming dialog with XLFD: [%s]\n", fontString));

	// Underline or Strikeout aren't standard X font properties.
	s = UT_getAttribute("text-decoration", props_in);
	if (s)
	{
		XML_Char*	p = strdup(s);
		UT_ASSERT(p);
		XML_Char*	q = strtok(p, " ");

		while (q)
		{
			if (0 == UT_stricmp(q, "underline"))
			{
				// TODO: do something for the GTK font selector to
				// consider this attribute?
			}
			else if (0 == UT_stricmp(q, "line-through"))
			{
				// TODO: do something for the GTK font selector to
				// consider this attribute?
			}
			q = strtok(NULL, " ");
		}

		free(p);
	}

	// Set color in the color selector
	s = UT_getAttribute("color", props_in);
	if (s)
	{
		UT_RGBColor c;
		UT_parseColor(s, c);

		currentColor[RED] = ((gdouble) c.m_red / (gdouble) 255.0);
		currentColor[GREEN] = ((gdouble) c.m_grn / (gdouble) 255.0);
		currentColor[BLUE] = ((gdouble) c.m_blu / (gdouble) 255.0);

		gtk_color_selection_set_color(GTK_COLOR_SELECTION(colorSelector), currentColor);

	}

	if (!gtk_font_selection_dialog_set_font_name(cf, fontString))
	{
		// We couldn't set the name, which means we don't have it
		// TODO maybe do substitution, does it matter?
		UT_DEBUGMSG(("Couldn't hint to font selection dialog to use XLFD.  "
					 "This font was not found on this X server.\n"));
	}

	// Set up a nice sample string
	gchar * sampleString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijlkmnopqrstuvwxyz";
	gtk_font_selection_dialog_set_preview_text(cf, (const gchar *) sampleString);

	// Run the dialog
	gtk_widget_show(GTK_WIDGET(cf));
	gtk_grab_add(GTK_WIDGET(cf));
	gtk_main();

	if (accepted)
	{

		selectedFont = gtk_font_selection_dialog_get_font_name(cf);

		if (selectedFont)
		{
			int i = 0;

			UT_DEBUGMSG(("Font selection returned [%s].\n\n", selectedFont));
			
			// currently a maximum of six simultaneous properties
			const XML_Char * props_out[] = {
				NULL, NULL,
				NULL, NULL,
				NULL, NULL,	
				NULL, NULL,
				NULL, NULL,
				NULL, NULL,
				0 };

			// duplicate and tokenize the XLFD
			gchar * cloneString = strdup(selectedFont);
			UT_ASSERT(cloneString);

			// first call gets foundry, which we ignore
			gchar * token = strtok(cloneString, "-");

			// font family name
			if ((token = strtok(NULL, "-")))
			{
				props_out[i] = "font-family";
				props_out[i+1] = token;
				i+=2;
			}

			/* weight (X has lots of defined weights, we just
			   cast them to bold or normal for now)
			*/
			if ((token = strtok(NULL, "-")))
			{
				if (!UT_stricmp(token, "black"))
				{
					props_out[i] = "font-weight";
					props_out[i+1] = "bold";
				}
				else if (!UT_stricmp(token, "bold"))
				{
					props_out[i] = "font-weight";
					props_out[i+1] = "bold";
				}
				else if (!UT_stricmp(token, "demibold"))
				{
					props_out[i] = "font-weight";
					props_out[i+1] = "bold";
				}
				else if (!UT_stricmp(token, "medium"))
				{
					props_out[i] = "font-weight";
					props_out[i+1] = "normal";
				}
				else if (!UT_stricmp(token, "regular"))
				{
					props_out[i] = "font-weight";
					props_out[i+1] = "normal";
				}
				else
				{
					props_out[i] = "font-weight";
					props_out[i+1] = "normal";
				}
				i += 2;
			}

			// slant (X has i,o,r, which we cast to italic or normal)
			if ((token = strtok(NULL, "-")))
			{
				if (!UT_stricmp(token, "i"))
				{
					props_out[i] = "font-style";
					props_out[i+1] ="italic";
				}
				else if (!UT_stricmp(token, "o"))
				{
					props_out[i] = "font-style";
					props_out[i+1] ="oblique";
				}
				else // o and r
				{
					props_out[i] = "font-style";
					props_out[i+1] ="normal";
				}
				i += 2;
			}

			// GTK doesn't ever return an Underline or
			// Strike attribute, so we'll use whatever was
			// set when the user started the selection

			s = UT_getAttribute("text-decoration", props_in);
			if (s)
			{
				props_out[i] = "text-decoration";
				props_out[i+1] = strdup(s);
				i += 2;
			}
			else
			{
				props_out[i] = "text-decoration";
				props_out[i+1] = "none";
				i += 2;
			}
		   
			// the windows code looks like this, for reference
			/*
			if ((lf.lfUnderline == TRUE) &&
				(lf.lfStrikeOut == TRUE))
			{
				props_out[i] = "text-decoration";
				props_out[i+1] ="underline line-through";
				i += 2;
			}
			else if (lf.lfUnderline == TRUE)
			{
				props_out[i] = "text-decoration";
				props_out[i+1] ="underline";
				i += 2;
			}
			else if (lf.lfStrikeOut == TRUE)
			{
				props_out[i] = "text-decoration";
				props_out[i+1] ="line-through";
				i += 2;
			}
			else
			{
				props_out[i] = "text-decoration";
				props_out[i+1] ="none";
				i += 2;
			}
			*/
			
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
				sprintf(buf_size, "%dpt", (atoi(token)/10));
				props_out[i] = "font-size";
				props_out[i+1] = buf_size;
				i += 2;
			}

			// Color
			gtk_color_selection_get_color(GTK_COLOR_SELECTION(colorSelector), currentColor);
			
			char buf_color[6];
			sprintf(buf_color, "%02x%02x%02x",
					(unsigned int) (currentColor[RED] 	* (gdouble) 255.0),
					(unsigned int) (currentColor[GREEN]	* (gdouble) 255.0),
					(unsigned int) (currentColor[BLUE] 	* (gdouble) 255.0));

			props_out[i] = "color";
			props_out[i+1] = buf_color;
			i += 2;

			// apply changes
			pView->setCharFormat(props_out);

			if (cloneString)
				free(cloneString);
		}
	}

	gtk_widget_destroy (GTK_WIDGET(cf));

	delete fontString;
	return UT_TRUE;

}

#endif /* 0 */
