/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2002 AbiSource, Inc.
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

#ifndef XAP_DIALOG_HTMLOPTIONS_H
#define XAP_DIALOG_HTMLOPTIONS_H

struct XAP_Exp_HTMLOptions
{
	bool	bIs4;
	bool	bIsAbiWebDoc;
	bool	bDeclareXML;
	bool	bAllowAWML;
	bool	bEmbedCSS;
	bool	bEmbedImages;

	/* other options, not set/saved/restore by options dialog
	 */
	bool	bMultipart;
};

#endif /* XAP_DIALOG_HTMLOPTIONS_H */
