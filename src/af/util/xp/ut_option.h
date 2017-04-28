/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:nil; -*- */
/* AbiWord
 * Copyright (C) 2017 Hubert Figuière
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


// an option<> template class inspired by Rust

#pragma once

#include <stdexcept>

#include "ut_types.h"

template<class T>
class ABI_EXPORT UT_Option
{
public:
  typedef T data_type;

  UT_Option()
    : m_none(true)
  {
  }
  UT_Option(T&& data)
    : m_none(false)
    , m_data(data)
  {
  }
  explicit UT_Option(const T& data)
    : m_none(false)
    , m_data(data)
  {
  }
  template<class... Args>
  UT_Option(Args&&... args)
    : m_none(false)
    , m_data(args...)
  {
  }

  T&& unwrap()
  {
    if (m_none) {
      throw std::runtime_error("none option value");
    }
    m_none = true;
    return std::move(m_data);
  }
  T unwrap_or(const T& value)
  {
    if (m_none) {
      return value;
    }
    return unwrap();
  }
  bool empty() const
  { return m_none; }
  explicit operator bool() const
  { return !m_none; }
private:
  bool m_none;
  T m_data;
};
