/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
# 	include "tb_text_subscript.xpm"
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
#	include "tb_redo.xpm"
#	include "tb_save.xpm"
#	include "tb_save_as.xpm"
#	include "tb_undo.xpm"
#	include "tb_todo.xpm"

	// these are specific to one or more translations
#	include "tb_text_bold_G.xpm"
#	include "tb_text_strikeout_B.xpm"
#	include "tb_text_underline_S.xpm"
#	include "tb_text_italic_K.xpm"
	// Finnish translations
#	include "tb_text_bold_L.xpm"
#	include "tb_text_strikeout_Y.xpm"
#	include "tb_text_underline_A.xpm"
	// Danish translations
#   include "tb_text_bold_F.xpm"
#   include "tb_text_strikeout_G.xpm"
	// Hungarian translations
//#	include "tb_text_bold_F.xpm"
#	include "tb_text_italic_D.xpm"
//#	include "tb_text_underline_A.xpm"
#	include "tb_text_overline_F.xpm"
#	include "tb_text_strikeout_K.xpm"
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
	DefineToolbarIcon(tb_redo_xpm)
	DefineToolbarIcon(tb_save_xpm)
	DefineToolbarIcon(tb_save_as_xpm)
	DefineToolbarIcon(tb_undo_xpm)
	DefineToolbarIcon(tb_todo_xpm)

	// these are specific to one or more translations
	DefineToolbarIcon(tb_text_bold_G_xpm)
	DefineToolbarIcon(tb_text_strikeout_B_xpm)
	DefineToolbarIcon(tb_text_underline_S_xpm)
	DefineToolbarIcon(tb_text_italic_K_xpm)
	// Finnish translations
	DefineToolbarIcon(tb_text_bold_L_xpm)
	DefineToolbarIcon(tb_text_strikeout_Y_xpm)
	DefineToolbarIcon(tb_text_underline_A_xpm)
	// Danish translations
    DefineToolbarIcon(tb_text_bold_F_xpm)	
    DefineToolbarIcon(tb_text_strikeout_G_xpm)
	// Hungarian translations
		//DefineToolbarIcon(tb_text_bold_F_xpm)
	DefineToolbarIcon(tb_text_italic_D_xpm)
		//DefineToolbarIcon(tb_text_underline_A_xpm)
	DefineToolbarIcon(tb_text_strikeout_K_xpm)
	DefineToolbarIcon(tb_text_overline_F_xpm)

	// ... also add new icons here ...

#endif
