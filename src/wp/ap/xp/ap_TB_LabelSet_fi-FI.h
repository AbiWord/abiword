/* AbiWord
* Copyright (C) 1998-2001 AbiSource, Inc.
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

// Original Finnish translations provided by Jarmo Karvonen <Jarmo.Karvonen@lpg.fi>
// 2nd update by Ismo M‰kinen <ismtaol@luukku.com> - March 2002

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// Note: if the tooltip is blank, the status message will be used as the
// Note: tooltip.  therefore, we probably don't need most tooltip strings
// Note: here -- unless the status message is too long to look good in
// Note: a tooltip.

// Note: the icon field should not be localized unless absolutely necessary.
// Note: the icon name here is to a specific icon (pixmap or bitmap or whatever)
// Note: that will always be in the application.  if, for example, a big fat 'B'
// Note: for BOLD doesn't make sense in another language, change the entry in
// Note: the localization and add the icon to whatever table.

// Note: if a tool item does not use an icon (like a combo box), use the
// Note: constant "NoIcon" in that column.

// If the third argument is true, then this is the fall-back for
// this language (named in the first argument).

BeginSetEnc(fi,FI,true,"iso-8859-1")

ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__, NULL, NoIcon, NULL,NULL)

//          (id, szLabel, IconName, szToolTip, szStatusMsg)

ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW, "Uusi", tb_new_xpm, NULL, "Luo uusi asiakirja")     
ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN, "Avaa", tb_open_xpm, NULL, "Avaa asiakirja")
ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE, "Tallenna", tb_save_xpm, NULL, "Tallenna asiakirja")
ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS, "Tallenna nimell‰", tb_save_as_xpm, NULL, "Tallenna k‰sitelt‰v‰ asiakirja eri nimell‰")
ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT, "Tulosta", tb_print_xpm, NULL, "Tulosta asiakirja")
ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW, "Esikatselu", tb_print_preview_xpm, NULL, "Tulostuksen esikatselu")

ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO, "Peru", tb_undo_xpm, NULL, "Peru muutokset")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO, "Uudelleen", tb_redo_xpm, NULL, "Palauta peruttu muokkaus")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT, "Leikkaa", tb_cut_xpm, NULL, "Leikkaa")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY, "Kopioi", tb_copy_xpm, NULL, "Kopioi")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE, "Liit‰", tb_paste_xpm, NULL, "Liit‰")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_HEADER, "Yl‰tunniste", tb_edit_editheader_xpm, NULL, "Muokkaa yl‰tunnistetta")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_FOOTER, "Alatunniste", tb_edit_editfooter_xpm, NULL, "Muokkaa alatunnistetta")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEHEADER, "Poista yl‰tunniste", tb_edit_removeheader_xpm, NULL, "Poista yl‰tunniste")
ToolbarLabel(AP_TOOLBAR_ID_EDIT_REMOVEFOOTER, "Poista alatunniste", tb_edit_removefooter_xpm, NULL, "Poista alatunniste")
ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK, "Oikoluku", tb_spellcheck_xpm, NULL, "Asiakirjan oikeinkirjoituksen tarkistus")
ToolbarLabel(AP_TOOLBAR_ID_IMG, "Lis‰‰ kuva", tb_insert_graphic_xpm, NULL, "Lis‰‰ kuva asiakirjaan")

ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE, "Tyyli", NoIcon, NULL, "Tyyli")
ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT, "Kirjasin", NoIcon, NULL, "Kirjasin")
ToolbarLabel(AP_TOOLBAR_ID_FMT_HYPERLINK, "Hyperlinkki", tb_hyperlink, NULL, "Lis‰‰ hyperlinkki asiakirjaan")
ToolbarLabel(AP_TOOLBAR_ID_FMT_BOOKMARK, "Kirjanmerkki", tb_anchor, NULL, "Lis‰‰ kirjanmerkki asiakirjaan")

ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE, "Kirjasinkoko", NoIcon, NULL, "Kirjasinkoko")
ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD, "Lihavoitu", tb_text_bold_L_xpm, NULL, "Lihavoitu")
ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC, "Kursivoitu", tb_text_italic_K_xpm, NULL, "Kursivoitu")
ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE, "Alleviivattu",tb_text_underline_A_xpm, NULL, "Alleviivattu")
ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE, "Ylleviivattu",tb_text_overline_xpm, NULL, "Ylleviivattu")
ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE, "Yliviivattu", tb_text_strikeout_Y_xpm, NULL, "Yliviivattu")
ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE, "Yl‰viiva", tb_text_topline_xpm, NULL, "Valitun tekstirivin yl‰puolelle viiva")
ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE, "Alaviiva", tb_text_bottomline_xpm, NULL, "Valitun tekstirivin alapuolelle viiva")
ToolbarLabel(AP_TOOLBAR_ID_HELP, "Ohje", tb_help_xpm, NULL, "Ohje")

ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT, "Yl‰indeksi", tb_text_superscript_xpm, NULL, "Yl‰indeksi")
ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT, "Alaindeksi", tb_text_subscript_xpm, NULL, "Alaindeksi")
ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL, "Lis‰‰ merkki", tb_symbol_xpm, NULL, "Lis‰‰ (erikois)merkki")

ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT, "Vasen", tb_text_align_left_xpm, NULL, "Vasen tasaus")
ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER, "Keskitetty", tb_text_center_xpm, NULL, "Keskitetty tasaus")
ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT, "Oikea", tb_text_align_right_xpm, NULL, "Oikea tasaus")
ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY, "Tasattu", tb_text_justify_xpm, NULL, "Molemmat reunat tasattu")

ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE, "Ei kappalev‰li‰", tb_para_0before_xpm, NULL, "Riviv‰li ennen kappaletta: ei v‰li‰")
ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE, "12 pt ennen", tb_para_12before_xpm, NULL, "Riviv‰li ennen kappaletta: 12 pt")

ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE, "Ykkˆsriviv‰li", tb_line_single_space_xpm, NULL, "Riviv‰li 1")
ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE, " Riviv‰li 1,5", tb_line_middle_space_xpm, NULL, " Riviv‰li 1,5")
ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE, "Kakkosriviv‰li", tb_line_double_space_xpm, NULL, "Riviv‰li 2")

ToolbarLabel(AP_TOOLBAR_ID_1COLUMN, "1 palsta", tb_1column_xpm, NULL, "1 palsta")
ToolbarLabel(AP_TOOLBAR_ID_2COLUMN, "2 palstaa", tb_2column_xpm, NULL, "2 palstaa")
ToolbarLabel(AP_TOOLBAR_ID_3COLUMN, "3 palstaa", tb_3column_xpm, NULL, "3 palstaa")
ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA, "N‰yt‰ kaikki", tb_view_showpara_xpm, NULL, "N‰yt‰ myˆskin tulostumattomat merkit")

ToolbarLabel(AP_TOOLBAR_ID_ZOOM, "Zoomaus", NoIcon, NULL, "N‰ytˆn suurennussuhde")
ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS, "Luettelo", tb_lists_xpm, NULL, "Luettelomerkit ja numerointi")
ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS, "Numerointi", tb_lists_numbers_xpm, NULL, "Valittujen rivien numerointi")
ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE, "Tekstiv‰ri", tb_text_fgcolor_xpm, NULL, "Tekstin v‰ri")
ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK, "Korostus", tb_text_bgcolor_xpm, NULL, "Tekstin taustav‰ri")
ToolbarLabel(AP_TOOLBAR_ID_INDENT, "Suurenna sisennyst‰", tb_text_indent_xpm, NULL, "Suurenna sisennyst‰")
ToolbarLabel(AP_TOOLBAR_ID_UNINDENT, "Pienenn‰ sisennyst‰", tb_text_unindent_xpm, NULL, "Pienenn‰ sisennyst‰")
ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY, "Aja skripti", tb_script_play_xpm, NULL, "Suorita skripti")
 ToolbarLabel(AP_TOOLBAR_ID_FMTPAINTER, "Muotoilusivellin", tb_stock_paint_xpm, NULL, "K‰yt‰ aikaisemmin kopioitua kappalemuotoilua valitussa tekstiss‰")

#ifdef BIDI_ENABLED
ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR, "Vasen-oikea", tb_text_direction_ltr_xpm, NULL, "Tekstin suunta vasemmalta oikealle")
ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL, " Oikea-vasen", tb_text_direction_rtl_xpm, NULL, "Tekstin suunta oikealta vasemmalle")
ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION, "Kappalesuunta ", tb_text_dom_direction_rtl_xpm, NULL, "Muuta kappaleen p‰‰suuntaa")
#endif

// ... add others here ...

ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__, NULL, NoIcon, NULL,NULL)

EndSet()

