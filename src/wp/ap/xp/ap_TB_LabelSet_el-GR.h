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

BeginSetEnc(el,GR,false,"iso-8859-7")

	ToolbarLabel(AP_TOOLBAR_ID__BOGUS1__,		NULL,			NoIcon,					NULL,NULL)

	//          (id, 		                    szLabel,		IconName,     			szToolTip,      szStatusMsg)

	ToolbarLabel(AP_TOOLBAR_ID_FILE_NEW,		"Νέο", 			tb_new_xpm,				NULL, "Δημιουργία νέου εγγράφου")	
	ToolbarLabel(AP_TOOLBAR_ID_FILE_OPEN,		"Άνοιγμα",			tb_open_xpm,			NULL, "Άνοιγμα υπάρχοντος εγγράφου")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVE,		"Αποθήκευση", 		tb_save_xpm,			NULL, "Αποθήκευση του εγγράφου")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_SAVEAS,		"Αποθήκευση ως", 		tb_save_as_xpm,			NULL, "Αποθήκευση του εγγράφου με διαφορετικό όνομα")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT,		"Εκτύπωση",		tb_print_xpm,			NULL, "Εκτύπωση του εγγράφου")
	ToolbarLabel(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW,	"Προεπισκόπηση Εκτύπωσης",	tb_print_preview_xpm, NULL, "Προεπισκόπιση του εγγράφου πριν από την εκτύπωση")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_UNDO,		"Αναίρεση",			tb_undo_xpm,			NULL, "Αναίρεση επεξεργασίας")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_REDO,		"Ακύρωση Αναίρεσης",			tb_redo_xpm,			NULL, "Ακύρωση αναίρεσης")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_CUT,		"Αποκοπή",			tb_cut_xpm,				NULL, "Αποκοπή")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_COPY,		"Αντιγραφή",			tb_copy_xpm,			NULL, "Αντιγραφή")
	ToolbarLabel(AP_TOOLBAR_ID_EDIT_PASTE,		"Επικόλληση",		tb_paste_xpm,			NULL, "Επικόλληση")
	ToolbarLabel(AP_TOOLBAR_ID_SPELLCHECK,		"Ορθογραφία",	tb_spellcheck_xpm,		NULL, "Ορθογραφικός Έλεγχος του εγγράφου")
	ToolbarLabel(AP_TOOLBAR_ID_IMG,				"Εισαγωγή Εικόνας",	tb_insert_graphic_xpm,	NULL, "Εισαγωγή εικόνας στο έγγραφο")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STYLE,		"Στυλ",		NoIcon,					NULL, "Στυλ")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_FONT,		"Γραμματοσειρά",			NoIcon,					NULL, "Γραμματοσειρά")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SIZE,		"Μέγεθος γραμματοσειράς",	NoIcon,					NULL, "Μέγεθος γραμματοσειράς")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOLD,		"Έντονη Γραφή",			tb_text_bold_xpm,		NULL, "Έντονη Γραφή")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_ITALIC,		"Πλάγια Γραφή",		tb_text_italic_xpm,		NULL, "Πλάγια Γραφή")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_UNDERLINE,	"Υπογράμμιση",	tb_text_underline_xpm,	NULL, "Υπογράμμιση")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_OVERLINE,	"Επιγράμμιση",		tb_text_overline_xpm,	NULL, "Επιγράμμιση")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_STRIKE,		"Μεσογράμμιση",		tb_text_strikeout_xpm,	NULL, "Μεσογράμμιση")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_TOPLINE,		"Επάνω γραμμή",		tb_text_topline_xpm,	NULL, "Επάνω γραμμή")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_BOTTOMLINE,		"Υπογράμμιση",		tb_text_bottomline_xpm,	NULL, "Υπογράμμιση")
	ToolbarLabel(AP_TOOLBAR_ID_HELP,			"Βοήθεια",			tb_help_xpm,			NULL, "Βοήθεια")

	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	"Superscript",	tb_text_superscript_xpm,	NULL, "Superscript")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_SUBSCRIPT,	"Subscript",	tb_text_subscript_xpm,		NULL, "Subscript")
	ToolbarLabel(AP_TOOLBAR_ID_INSERT_SYMBOL,	"Σύμβολο",		tb_symbol_xpm,				NULL, "Εισαγωγή Συμβόλου")

	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_LEFT,		"Αριστερά",			tb_text_align_left_xpm,		NULL, "Στοίχιση Αριστερά")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_CENTER,	"Κέντρο",		tb_text_center_xpm,			NULL, "Στοίχιση στο Κέντρο")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_RIGHT,		"Δεξιά",		tb_text_align_right_xpm,	NULL, "Στοίχιση Δεξιά")
	ToolbarLabel(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	"Στοίχιση",		tb_text_justify_xpm,		NULL, "Πλήρες Στοίχιση")

	ToolbarLabel(AP_TOOLBAR_ID_PARA_0BEFORE,	"Κανένα πριν",		tb_para_0before_xpm,	NULL, "Κενό πριν: Κανένα")
	ToolbarLabel(AP_TOOLBAR_ID_PARA_12BEFORE,	"12 pt πριν",		tb_para_12before_xpm,	NULL, "Κενό πριν: 12 pt")

	ToolbarLabel(AP_TOOLBAR_ID_SINGLE_SPACE,	"Μονή γραμμή",	tb_line_single_space_xpm,	NULL, "Μονή γραμμή")
	ToolbarLabel(AP_TOOLBAR_ID_MIDDLE_SPACE,	"1.5 γραμμή",		tb_line_middle_space_xpm,	NULL, "1.5 γραμμή")
	ToolbarLabel(AP_TOOLBAR_ID_DOUBLE_SPACE,	"Διπλή γραμμή",	tb_line_double_space_xpm,	NULL, "Διπλή γραμμή")

	ToolbarLabel(AP_TOOLBAR_ID_1COLUMN,			"1 Στήλη",			tb_1column_xpm,			NULL, "1 Στήλη")
	ToolbarLabel(AP_TOOLBAR_ID_2COLUMN,			"2 Στήλες",		tb_2column_xpm,			NULL, "2 Στήλες")
	ToolbarLabel(AP_TOOLBAR_ID_3COLUMN,			"3 Στήλες",		tb_3column_xpm,			NULL, "3 Στήλες")

	ToolbarLabel(AP_TOOLBAR_ID_VIEW_SHOWPARA,	"Εμφάνιση Όλων",			tb_view_showpara_xpm,		NULL, "Εμφάνιση/Απόκρυψη Ά")
	ToolbarLabel(AP_TOOLBAR_ID_ZOOM,			"Ζουμ",				NoIcon,					NULL, "Ζουμ")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_BULLETS,	"Κουκίδες",			tb_lists_bullets_xpm,	NULL, "Κουκίδες")
	ToolbarLabel(AP_TOOLBAR_ID_LISTS_NUMBERS,	"Αρίθμηση",		tb_lists_numbers_xpm,	NULL, "Αρίθμηση")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_FORE,		"Χρώμα Γραμματοσειράς",		tb_text_fgcolor_xpm,	NULL, "Χρώμα Γραμματοσειράς")
	ToolbarLabel(AP_TOOLBAR_ID_COLOR_BACK,		"Επισήμανση",		tb_text_bgcolor_xpm,	NULL, "Επισήμανση")
	ToolbarLabel(AP_TOOLBAR_ID_INDENT,			"Αύξηση εσοχής",	tb_text_indent_xpm, 	NULL, "Αύξηση εσοχής")
	ToolbarLabel(AP_TOOLBAR_ID_UNINDENT,		"Μείωση εσοχής",	tb_text_unindent_xpm,	NULL, "Μείωση εσοχής")

	ToolbarLabel(AP_TOOLBAR_ID_SCRIPT_PLAY,		"Ex. script",	tb_script_play_xpm,		NULL, "Execute script")

     // ... add others here ...
#ifdef BIDI_ENABLED
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	"Κείμενο Υποχρεωτικά ΔΠΑ",		tb_text_direction_ltr_xpm,	NULL, "Υποχρεωτική κατεύθυνση κειμένου ΔΠΑ")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,	"Κείμενο Υποχρεωτικά ΑΠΔ",		tb_text_direction_rtl_xpm,	NULL, "Υποχρεωτική κατεύθυνση κειμένου ΑΠΔ")
	ToolbarLabel(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,		"Κατεύθυνση Παραγράφου",	tb_text_dom_direction_rtl_xpm,	NULL, "Κυρίαρχη κατεύθυνση αλλαγής της παραγράφου")
#endif
	ToolbarLabel(AP_TOOLBAR_ID__BOGUS2__,		NULL,			NoIcon,			NULL,NULL)

EndSet()
