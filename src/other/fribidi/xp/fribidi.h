/* FriBidi - Library of BiDi algorithm
 * Copyright (C) 1999,2000 Dov Grobgeld, and
 * Copyright (C) 2001 Behdad Esfahbod. 
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 * 
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this library, in a file named COPYING.LIB; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA  
 * 
 * For licensing issues, contact <dov@imagic.weizmann.ac.il> and 
 * <fwpg@sharif.edu>. 
 */

#ifndef FRIBIDI_H
#define FRIBIDI_H

#define USE_SIMPLE_MALLOC

#ifdef __cplusplus
extern "C" {
#endif

#include "fribidi_types.h"
/*#include "fribidi_char_sets.h"*/

FriBidiCharType fribidi_get_type (FriBidiChar uch);

gboolean fribidi_get_mirror_char (	/* Input */
				   FriBidiChar ch,
				   /* Output */
				   FriBidiChar * mirrored_ch);

void fribidi_log2vis (		/* input */
		       FriBidiChar * str,
		       gint len, FriBidiCharType * pbase_dir,
		       /* output */
		       FriBidiChar * visual_str,
		       FriBidiStrIndex * position_L_to_V_list,
		       FriBidiStrIndex * position_V_to_L_list,
		       guint8 * embedding_level_list);

void fribidi_log2vis_get_embedding_levels (	/* input */
					    FriBidiChar * str,
					    int len,
					    FriBidiCharType * pbase_dir,
					    /* output */
					    guint8 * embedding_level_list);

/*======================================================================
//  fribidi_remove_explicits() removes explicit marks, and returns the
//  new length.
//----------------------------------------------------------------------*/
gint fribidi_remove_explicits (FriBidiChar * str, gint length);

/*======================================================================
//  fribidi_mirroring_status() returns whether mirroring is on or off,
//  default is on.
//----------------------------------------------------------------------*/
gboolean fribidi_mirroring_status (void);

void fribidi_set_mirroring (gboolean mirror);

/*======================================================================
//  fribidi_set_denug() turn on or off debugging, default is off.
//----------------------------------------------------------------------*/
gboolean fribidi_set_debug (gboolean debug);

/* fribidi_utils.c */

/*======================================================================
//  fribidi_find_string_changes() finds the bounding box of the section
//  of characters that need redrawing. It returns the start and the
//  length of the section in the new string that needs redrawing.
//----------------------------------------------------------------------*/
void fribidi_find_string_changes (	/* input */
				   FriBidiChar * old_str,
				   int old_len,
				   FriBidiChar * new_str, int new_len,
				   /* output */
				   int *change_start, int *change_len);


/*======================================================================
//  The find_visual_ranges() function is used to convert between a
//  continous span in either logical or visual space to a one, two or
//  three discontinous spans in the other space. The function outputs
//  the number of ranges needed to display the mapped range as
//  well as the resolved ranges.
//
//  The variable is_v2l_map indicates whether the position map is
//  is in the direction of visual-to-logical. This information is
//  needed in order to look up the correct character from the
//  embedding_level_list which is assumed to be in logical order.
//
//  This function is typically used to resolve a logical range to visual
//  ranges e.g. to display the selection.
//
//  Example:
//     The selection is between logical characters 10 to 45. Calculate
//     the corresponding visual selection(s):
//
//     gint sel_span[2] = {10,45};
//
//     fribidi_map_range(sel_span,
//                       TRUE,
//                       length,
//                       vis2log_map,
//                       embedding_levels,
//                       // output 
//                       &num_vis_ranges,
//                       vis_ranges);
//
//----------------------------------------------------------------------*/
void
fribidi_map_range (gint span[2],
		   int len,
		   gboolean is_v2l_map,
		   FriBidiStrIndex * position_map,
		   guint8 * embedding_level_list,
		   /* output */
		   int *num_mapped_spans, int spans[3][2]);

/*======================================================================
//  fribidi_is_char_rtl() answers the question whether a character
//  was resolved in the rtl direction. This simply involves asking
//  if the embedding level for the character is odd.
//----------------------------------------------------------------------*/
gboolean
fribidi_is_char_rtl (guint8 * embedding_level_list,
		     FriBidiCharType base_dir, int idx);

/*======================================================================
//  fribidi_xpos_resolve() does the complicated translation of
//  an x-coordinate, e.g. as received through a mouse press event,
//  to the logical and the visual position the xcoordinate is closest
//  to. It will also resolve the direction of the cursor according
//  to the embedding level of the closest character.
//
//  It does this through the following logics:
//  Here are the different possibilities:
//
//        Pointer              =>          Log Pos         Vis pos
//  
//     Before first vis char             log_pos(vis=0)L       0
//     After last vis char               log_pos(vis=n-1)R     n
//     Within 1/2 width of vis char i    log_pos(vis=i)L       i
//     Within last 1/2 width of vchar i  log_pos(vis=i)R       i+1
//     Border between vis chars i,i+1       resolve!           i+1
//
//  Input:
//     x_pos        The pixel position to be resolved measured in pixels.
//     x_offset     The x_offset is the pixel position of the left side
//                  of the leftmost visual character. 
//     len          The length of the embedding level, the vis2log and
//                  the char width arrays.
//     base_dir     The resolved base direction of the line.
//     vis2log      The vis2log mapping.
//                  x_position and the character widths. The position
//                  (x_pos-x_offset) is number of pixels from the left
//                  of logical character 0.
//     char_widths  Width in pixels of each character. Note that the
//                  widths should be provided in logical order.
//
//  Output:
//     res_log_pos  Resolved logical position.
//     res_vis_pos  Resolved visual position
//     res_cursor_x_pos   The resolved pixel position to the left or
//                  the right of the character position x_pos.
//     res_cursor_dir_is_rtl   Whether the resolved dir of the character
//                  at position x_pos is rtl.
//     res_attach_before  Whether the x_pos is cutting the bounding
//                  box in such a way that the visual cursor should be
//                  be positioned before the following logical character.
//                  Note that in the bidi context, the positions "after
//                  a logical character" and "before the following logical
//                  character" is not necessarily the same. If x_pos is
//                  beyond the end of the line, res_attach_before is true.
//
//----------------------------------------------------------------------*/

void fribidi_xpos_resolve (gint x_pos,
			   gint x_offset,
			   gint len,
			   guint8 * embedding_level_list,
			   FriBidiCharType base_dir,
			   FriBidiStrIndex * vis2log, gint16 * char_widths,
			   /* output */
			   gint * res_log_pos,
			   gint * res_vis_pos,
			   gint * res_cursor_x_pos,
			   gboolean * res_cursor_dir_is_rtl,
			   gboolean * res_attach_before);

#ifdef __cplusplus
}
#endif

#endif /* FRIBIDI_H */
