/* AbiWord
 * Copyright (C) 2013-2016 Hubert Figuiere
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


#include "xap_UnixDlg_ColorChooser.h"


std::unique_ptr<UT_RGBColor> XAP_UnixDlg_RunColorChooser(GtkWindow* parent,
                                                         GtkColorButton* colorbtn)
{
	GtkWidget *colordlg = gtk_color_chooser_dialog_new("", parent);
	GtkColorChooser *colorsel = GTK_COLOR_CHOOSER(colordlg);

	GdkRGBA initialColor;
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colorbtn), &initialColor);
	gtk_color_chooser_set_rgba(colorsel, &initialColor);

	UT_RGBColor* rgb = nullptr;

	gint result = gtk_dialog_run (GTK_DIALOG (colordlg));
	if (result == GTK_RESPONSE_OK) {
		// update button
		GdkRGBA color;
		gtk_color_chooser_get_rgba (colorsel, &color);
		gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER(colorbtn), &color);

		// update dialog
		rgb = UT_UnixGdkColorToRGBColor (color);
	}

	// do not propagate further
	gtk_widget_destroy (colordlg);
	return std::unique_ptr<UT_RGBColor>(rgb);
}
