/* AbiSource Program Utilities
 *
 * Copyright (C) 2009 Hubert Figuiere
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


#ifndef __UT_STD_ALGO_H__
#define __UT_STD_ALGO_H__

template <typename _Container>
void UT_std_delete_all(const _Container & cont)
{
    for(typename _Container::const_iterator iter = cont.begin();
        iter != cont.end(); ++iter) {
        delete *iter;
    }
}

#endif
