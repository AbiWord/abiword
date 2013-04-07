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


#ifndef XAP_COCOAENCMGR_H
#define XAP_COCOAENCMGR_H

#include "xap_EncodingManager.h"

/* it's assumed that only one instance of this class will exist */
class XAP_CocoaEncodingManager: public XAP_EncodingManager
{
protected:
    XAP_CocoaEncodingManager();
    virtual ~XAP_CocoaEncodingManager();

    void 	initialize();

public:
    const char* getNativeEncodingName() 	const;
    const char* getNative8BitEncodingName()	const;
    const char* getNativeUnicodeEncodingName() 	const;
    const char* getLanguageISOName() 		const;
    const char* getLanguageISOTerritory() 	const;

	friend class XAP_EncodingManager;
};

#endif /* XAP_COCOAENCMGR_H */
