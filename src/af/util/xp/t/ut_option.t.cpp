/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:nil; -*- */
/* AbiWord
 * Copyright (C) 2017 Hubert Figuiere
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


#include <string>
#include "tf_test.h"
#include "ut_option.h"

#define TFSUITE "core.af.util.option"


TFTEST_MAIN("UT_option<string>")
{
  UT_Option<std::string> result;

  // Default, option is empty
  TFPASS(result.empty());
  bool unwrapped = false;
  try {
    result.unwrap();
    unwrapped = true;
  } catch(std::runtime_error&) {
    TFPASS(true);
  } catch(...) {
    TFPASS(false);
  }
  TFPASS(!unwrapped);
  TFPASS(result.empty());
  TFPASS(!result);

  // Option with value
  result = UT_Option<std::string>("hello world");
  TFPASS(!result.empty());
  TFPASS(!!result);
  TFPASS(result.unwrap() == "hello world");
  TFPASS(result.empty());
  TFPASS(!result);
  // try unwrapping again
  unwrapped = false;
  try {
    result.unwrap();
    unwrapped = true;
  } catch(std::runtime_error&) {
    TFPASS(true);
  } catch(...) {
    TFPASS(false);
  }
  TFPASS(!unwrapped);
  TFPASS(result.empty());
  TFPASS(!result);
}
