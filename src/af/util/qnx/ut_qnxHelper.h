/* AbiSource Program Utilities
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

#ifndef UT_QNXHELER_H
#define UT_QNXHELPER_H

#include <Pt.h>

#define ABI_DEFAULT_BUTTON_WIDTH 80
#define ABI_MODAL_WINDOW_RENDER_FLAGS (Ph_WM_RENDER_RESIZE | Ph_WM_RENDER_MIN | \
									   Ph_WM_RENDER_MAX | Ph_WM_RENDER_HELP | \
									   Ph_WM_RENDER_COLLAPSE | Ph_WM_RENDER_MENU | \
									   Ph_WM_RENDER_ASPALETTE )
#define ABI_MODAL_WINDOW_MANAGE_FLAGS (Ph_WM_RESIZE | Ph_WM_HIDE | Ph_WM_MAX )

/*
 This will center one window on another, if parent is NULL then
 the widget is centered on the screen.
 Return 0 on success, <0 on error
*/
int  UT_QNXCenterWindow(PtWidget_t *parent, PtWidget_t *widget);

/* 
 This will get the dimensions for the widget and stuff them into
 whatever variables you provide.  Used to help with subwindow
 positioning
*/
int UT_QNXGetWidgetArea(PtWidget_t *widget, short *x, short *y, unsigned short *w, unsigned short *h);

/*
 This will block/unblock the widget, effectively enabling
 and disabling the widget.  Mainly used to block input to
 a window.
*/
int  UT_QNXBlockWidget(PtWidget_t *widget, int block);

#endif /* UT_QNXHELPER_H */
