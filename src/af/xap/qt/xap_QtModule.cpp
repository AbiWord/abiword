/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Application Framework
 * Copyright (C) 2012 Hubert Figuiere
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

#include "xap_QtModule.h"

XAP_QtModule::XAP_QtModule()
	: m_bLoaded(false)
	, m_szname(NULL)
{
}

XAP_QtModule::~XAP_QtModule(void)
{
}

bool
XAP_QtModule::load (const char * name)
{
	return false;
}

bool
XAP_QtModule::unload (void)
{
	return false;
}

bool
XAP_QtModule::resolveSymbol (const char * symbol_name, void ** symbol)
{
	return false;
}

bool
XAP_QtModule::getModuleName (char ** dest) const
{
	return false;
}

bool
XAP_QtModule::getErrorMsg (char ** dest) const
{
	return false;
}
