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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */



#ifndef XAP_LOADBINDINGS_H
#define XAP_LOADBINDINGS_H
class EV_EditMethodContainer;
class EV_EditBindingMap;

/*****************************************************************/

class ABI_EXPORT XAP_BindingSet
{
public:
	XAP_BindingSet(EV_EditMethodContainer * pemc);
	virtual ~XAP_BindingSet(void);

	virtual EV_EditBindingMap *	getMap(const char * szName) = 0;

protected:
	EV_EditMethodContainer	  * m_pemc;
};

#endif /* XAP_LOADBINDINGS_H */
