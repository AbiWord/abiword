/* AbiWord
 * Copyright (C) 2007 One Laptop Per Child
 * Author: Marc Maurer <uwog@uwog.net>
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

#ifndef AP_FRAMELISTENER_H
#define AP_FRAMELISTENER_H

typedef enum _AP_FrameSignal
{
	APF_ReplaceDocument = 0,
	APF_ReplaceView
} AP_FrameSignal;

class ABI_EXPORT AP_FrameListener
{
public:
	virtual ~AP_FrameListener() {}
	virtual void signalFrame(AP_FrameSignal) = 0;
};

#endif // AP_FRAMELISTENER_H
