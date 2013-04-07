/* AbiSource Application Framework
 * Copyright (C) 2001 Hubert Figuiere
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


/*
	Implements Cocoa timers using NSTimer
 */

#ifndef XAP_COCOATIMER
#define XAP_COCOATIMER

#include "ut_types.h"

void _checkLock(void);
UT_uint32 XAP_newCocoaTimer (UT_uint32 time, int (*proc)(void *), void *p);
void XAP_stopCocoaTimer (UT_uint32 timer);


#endif
