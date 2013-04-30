/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2013 Hubert Figuiere
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

#ifndef XAP_QTDIALOGFACTORY_H
#define XAP_QTDIALOGFACTORY_H

#include "xap_DialogFactory.h"

class XAP_QtDialogFactory : public XAP_DialogFactory
{
public:
	XAP_QtDialogFactory(XAP_App * pApp, XAP_Frame * pFrame = NULL);
	virtual ~XAP_QtDialogFactory(void);

protected:
};

#endif /* XAP_QTDIALOGFACTORY_H */
