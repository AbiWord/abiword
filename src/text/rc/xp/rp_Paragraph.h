/* AbiWord
 * Copyright (C) 2004 Marc Maurer
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

#ifndef RP_PARAGRAPH_H
#define RP_PARAGRAPH_H

#include "pt_Types.h"
#include "rp_Object.h"
#include "rp_ContainerObject.h"
#include "rl_Layout.h"
#include "ut_vector.h"

class ABI_EXPORT rp_Paragraph : public rp_ContainerObject
{
public:
	rp_Paragraph(rl_Layout* pLayout);
	virtual ~rp_Paragraph() {}
};

#endif /* RP_PARAGRAPH_H */
