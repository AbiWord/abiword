/* 
 * Copyright (C) 2008 Robert Staudinger <robert.staudinger@gmail.com>
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

#include "abi-builtin-plugins.h"
#include "xap_Module.h"

#include "abi-builtin-plugins-decls.inc"

void abi_register_builtin_plugins()
{
	// TODO at the moment the statically linked plugins don't show up anywhere
	// they just work.
	XAP_ModuleInfo mi;
	XAP_ModuleInfo *pmi;

	pmi = &mi;

#include "abi-builtin-plugins-calls.inc"
}

