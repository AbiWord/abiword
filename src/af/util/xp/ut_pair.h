/* AbiSource Program Utilities
 *
 * Copyright (C) 2001 AbiSource, Inc.
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
#ifndef UT_PAIR_H
#define UT_PAIR_H

#include <stdlib.h>
#include "xap_AbiObject.h"

typedef void* pair_type;

class UT_Pair : public XAP_AbiObject
{
public:
    UT_Pair(const pair_type car, const pair_type cdr);
    ~UT_Pair();

	inline const pair_type& car() const { return car_; }
	inline const pair_type& cdr() const { return cdr_; }
	inline pair_type& car() { return car_; }
	inline pair_type& cdr() { return cdr_; }

private:
    pair_type car_, cdr_;
};

#endif
