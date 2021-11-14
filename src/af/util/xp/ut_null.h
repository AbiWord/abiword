/* AbiSource Program Utilities
 *
 * Copyright (C) 2021 Hubert Figuiere <hub@figuiere.net>
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

#pragma once

#include <type_traits>

/// Template to get a null value: 0 scalar or nullptr.
template <typename T, bool is_ptr = std::is_pointer<T>::value>
struct UT_null {
};

template <typename T>
struct UT_null<T, true> {
    constexpr static T value = nullptr;
};

template <typename T>
struct UT_null<T, false> {
    constexpr static T value = 0;
};
