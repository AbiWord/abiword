/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2000 Hubert Figuiere <hfiguiere@teaser.fr>
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
 
 
#ifndef __AP_MAC_RESID_H__
#define __AP_MAC_RESID_H__

/* 'ALRT' resource IDs */
#define RES_ALRT_OK				128
#define RES_ALRT_OKCANCEL		129
#define RES_ALRT_YESNO			130
#define RES_ALRT_YESNOCANCEL	131

#define RES_ALRT_ASSERT			1000

/* 'DITL' resource IDs */
#define RES_DITL_ALRT_OK 		RES_ALRT_OK
#define RES_DITL_ALRT_OKCANCEL	RES_ALRT_OKCANCEL
#define RES_DITL_ALRT_YESNO		RES_ALRT_YESNO
#define RES_DITL_ALRT_YESNOCANCEL	RES_ALRT_YESNOCANCEL

#define RES_DITL_ALRT_ASSERT	RES_ALRT_ASSERT

/* 'STR ' resource IDs */
#define HFDR_STR_ID				128

#ifndef __INCLUDING_REZ__
/* section for C code */

#endif

#endif
