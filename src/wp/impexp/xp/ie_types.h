/* AbiWord
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


#ifndef IE_TYPES_H
#define IE_TYPES_H

typedef enum _IEStatus { IES_OK,
						 IES_Error,
						 IES_FileNotFound,
						 IES_NoMemory,
						 IES_UnknownType,
						 IES_BogusDocument,
						 IES_CouldNotOpenForWriting,
						 IES_CouldNotWriteToFile } IEStatus;

typedef enum _IEFileType {	IEFT_Unknown,
							IEFT_AbiWord_1 } IEFileType;


#endif /* IE_TYPES_H */
