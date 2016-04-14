/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2004 Hubert Figuiere
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

#include "ut_types.h"
#include "ut_vector.h"

#include "ev_EditEventMapper.h"

#ifndef XAP_INPUTMODES_H
#define XAP_INPUTMODES_H

class ABI_EXPORT XAP_InputModes
{
public:
	XAP_InputModes(void);
	~XAP_InputModes(void);

	bool							createInputMode(const char * szName,
													EV_EditBindingMap * pBindingMap);
	bool							setCurrentMap(const char * szName);
	EV_EditEventMapper *			getCurrentMap(void) const;
	const char * 					getCurrentMapName(void) const;
	EV_EditEventMapper *			getMapByName(const char * szName) const;

protected:
	UT_GenericVector<EV_EditEventMapper *>	m_vecEventMaps; /* EV_EditEventMapper * */
	UT_GenericVector<char*>			m_vecNames;		/* const char * */

	UT_uint32						m_indexCurrentEventMap;
};

#endif // XAP_INPUTMODES_H

