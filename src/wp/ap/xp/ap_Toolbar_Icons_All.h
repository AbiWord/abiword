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

/*****************************************************************
******************************************************************
** FOR EACH ICON YOU ADD, ADD IT TO BOTH SECTIONS OF THIS FILE. **
******************************************************************
*****************************************************************/

#ifndef AP_TOOLBAR_ICONS_ALL_H

#	define AP_TOOLBAR_ICONS_ALL_H

	// Include each toolbar icon that we want to build.

#	include "tb_text_center.xpm"
#	include "tb_text_justify.xpm"
#	include "tb_text_align_left.xpm"
#	include "tb_text_align_right.xpm"
#	include "tb_text_bold.xpm"
#	include "tb_text_font.xpm"
#	include "tb_text_italic.xpm"
#	include "tb_text_strikeout.xpm"
#	include "tb_text_underline.xpm"
#	include "tb_text_overline.xpm"
#	include "tb_text_superscript.xpm"
#	include "tb_text_subscript.xpm"
#	include "tb_help.xpm"
#ifdef BIDI_ENABLED
#	include "tb_text_direction_rtl.xpm" 
#	include "tb_text_direction_ltr.xpm" 
#	include "tb_text_dom_direction_rtl.xpm" 
#endif
#	include "tb_symbol.xpm"
#	include "tb_para_0before.xpm"
#	include "tb_para_12before.xpm"
#	include "tb_1column.xpm"
#	include "tb_2column.xpm"
#	include "tb_3column.xpm"
#	include "tb_line_single_space.xpm"
#	include "tb_line_middle_space.xpm"
#	include "tb_line_double_space.xpm"
#	include "tb_copy.xpm"
#	include "tb_cut.xpm"
#	include "tb_edit.xpm"
#	include "tb_new.xpm"
#	include "tb_open.xpm"
#	include "tb_paste.xpm"
#	include "tb_print.xpm"
#	include "tb_print_preview.xpm"
#	include "tb_redo.xpm"
#	include "tb_save.xpm"
#	include "tb_save_as.xpm"
#	include "tb_undo.xpm"
#	include "tb_todo.xpm"
#	include "tb_lists_bullets.xpm"
#	include "tb_lists_numbers.xpm"
#	include "tb_text_indent.xpm"
#	include "tb_text_unindent.xpm"
#	include "tb_spellcheck.xpm"
#	include "tb_insert_graphic.xpm"
#	include "tb_view_showpara.xpm"
#	include "tb_text_fgcolor.xpm"
#	include "tb_text_bgcolor.xpm"
	// these are specific to one or more translations
#	include "tb_text_bold_G.xpm"
#	include "tb_text_strikeout_B.xpm"
#	include "tb_text_underline_S.xpm"
#	include "tb_text_italic_K.xpm"
	// Finnish translations
#	include "tb_text_bold_L.xpm"
#	include "tb_text_strikeout_Y.xpm"
#	include "tb_text_underline_A.xpm"
	// Polish translations
#	include "tb_text_bold_P.xpm"
#	include "tb_text_strikeout_P.xpm"
#	include "tb_text_underline_P.xpm"
#	include "tb_text_overline_P.xpm"
	// Danish translations
#	include "tb_text_bold_F.xpm"
#	include "tb_text_strikeout_G.xpm"
	// Hungarian translations
//#	include "tb_text_bold_F.xpm"
#	include "tb_text_italic_D.xpm"
//#	include "tb_text_underline_A.xpm"
#	include "tb_text_overline_F.xpm"
#	include "tb_text_strikeout_K.xpm"
	// Spanish translations
#	include "tb_text_bold_N.xpm"
	// Russian translations
#	include "tb_text_bold_rus.xpm"
#	include "tb_text_italic_rus.xpm"
#	include "tb_text_underline_rus.xpm"
#	include "tb_text_overline_rus.xpm"
#	include "tb_text_strikeout_rus.xpm"
	// Italian translations
#	include "tb_text_overline_S.xpm"
	// ... add new icons here (don't forget to add below the #else) ...

#else

	// Declare each toolbar icon that we are building.
	
	DefineToolbarIcon(tb_text_center_xpm)
	DefineToolbarIcon(tb_text_justify_xpm)
	DefineToolbarIcon(tb_text_align_left_xpm)
	DefineToolbarIcon(tb_text_align_right_xpm)
	DefineToolbarIcon(tb_text_bold_xpm)
	DefineToolbarIcon(tb_text_font_xpm)
	DefineToolbarIcon(tb_text_italic_xpm)
	DefineToolbarIcon(tb_text_strikeout_xpm)
	DefineToolbarIcon(tb_text_underline_xpm)
	DefineToolbarIcon(tb_text_overline_xpm)
	DefineToolbarIcon(tb_text_superscript_xpm)	
	DefineToolbarIcon(tb_text_subscript_xpm)
	DefineToolbarIcon(tb_help_xpm)
#ifdef BIDI_ENABLED
	//DefineToolbarIcon(tb_text_direction_rtl_xpm)
	DefineToolbarIcon(tb_text_direction_ltr_xpm)
	DefineToolbarIcon(tb_text_direction_rtl_xpm)
	DefineToolbarIcon(tb_text_dom_direction_rtl_xpm)
#endif
	DefineToolbarIcon(tb_symbol_xpm)
	DefineToolbarIcon(tb_para_0before_xpm)
	DefineToolbarIcon(tb_para_12before_xpm)
	DefineToolbarIcon(tb_1column_xpm)
	DefineToolbarIcon(tb_2column_xpm)
	DefineToolbarIcon(tb_3column_xpm)
	DefineToolbarIcon(tb_line_single_space_xpm)
	DefineToolbarIcon(tb_line_middle_space_xpm)
	DefineToolbarIcon(tb_line_double_space_xpm)
	DefineToolbarIcon(tb_copy_xpm)
	DefineToolbarIcon(tb_cut_xpm)
	DefineToolbarIcon(tb_edit_xpm)
	DefineToolbarIcon(tb_new_xpm)
	DefineToolbarIcon(tb_open_xpm)
	DefineToolbarIcon(tb_paste_xpm)
	DefineToolbarIcon(tb_print_xpm)
	DefineToolbarIcon(tb_print_preview_xpm)
	DefineToolbarIcon(tb_redo_xpm)
	DefineToolbarIcon(tb_save_xpm)
	DefineToolbarIcon(tb_save_as_xpm)
	DefineToolbarIcon(tb_undo_xpm)
	DefineToolbarIcon(tb_todo_xpm)
	DefineToolbarIcon(tb_lists_numbers_xpm)
	DefineToolbarIcon(tb_lists_bullets_xpm)
	DefineToolbarIcon(tb_text_indent_xpm)
	DefineToolbarIcon(tb_text_unindent_xpm)
	DefineToolbarIcon(tb_spellcheck_xpm)
	DefineToolbarIcon(tb_insert_graphic_xpm)
	DefineToolbarIcon(tb_view_showpara_xpm)
	DefineToolbarIcon(tb_text_fgcolor_xpm)
	DefineToolbarIcon(tb_text_bgcolor_xpm)

	// these are specific to one or more translations
	DefineToolbarIcon(tb_text_bold_G_xpm)
	DefineToolbarIcon(tb_text_strikeout_B_xpm)
	DefineToolbarIcon(tb_text_underline_S_xpm)
	DefineToolbarIcon(tb_text_italic_K_xpm)
	// Finnish translations
	DefineToolbarIcon(tb_text_bold_L_xpm)
	DefineToolbarIcon(tb_text_strikeout_Y_xpm)
	DefineToolbarIcon(tb_text_underline_A_xpm)
	// Polish translations
	DefineToolbarIcon(tb_text_bold_P_xpm)
	DefineToolbarIcon(tb_text_strikeout_P_xpm)
	DefineToolbarIcon(tb_text_underline_P_xpm)
	DefineToolbarIcon(tb_text_overline_P_xpm)
	// Danish translations
	DefineToolbarIcon(tb_text_bold_F_xpm)	
	DefineToolbarIcon(tb_text_strikeout_G_xpm)
	// Hungarian translations
		//DefineToolbarIcon(tb_text_bold_F_xpm)
	DefineToolbarIcon(tb_text_italic_D_xpm)
		//DefineToolbarIcon(tb_text_underline_A_xpm)
	DefineToolbarIcon(tb_text_strikeout_K_xpm)
	DefineToolbarIcon(tb_text_overline_F_xpm)
	// Spanish translations
	DefineToolbarIcon(tb_text_bold_N_xpm)
	// Russian translations
	DefineToolbarIcon(tb_text_bold_rus_xpm)
	DefineToolbarIcon(tb_text_italic_rus_xpm)
	DefineToolbarIcon(tb_text_underline_rus_xpm)
	DefineToolbarIcon(tb_text_overline_rus_xpm)
	DefineToolbarIcon(tb_text_strikeout_rus_xpm)
	// Italian translations
	DefineToolbarIcon(tb_text_overline_S_xpm)


	// ... also add new icons here ...

#endif
