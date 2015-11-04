/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2011 Hub Figuiere
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

#include <stdio.h>

#include "tf_test.h"
#include "pf_Fragments.h"
#include "pf_Frag.h"

#define TFSUITE "core.text.ptbl.fragments"

TFTEST_MAIN("pf_Fragments")
{
#if 0
  pf_Fragments fragments;

  pf_Frag *frag1, *frag2, *frag3;

  frag1 = new pf_Frag(NULL, pf_Frag::PFT_Text, 0);
  frag2 = new pf_Frag(NULL, pf_Frag::PFT_Text, 0);
  frag3 = new pf_Frag(NULL, pf_Frag::PFT_Text, 0);

  printf("frag1 = %p frag2 = %p frag3 = %p\n", frag1, frag2, frag3);

  fragments.appendFrag(frag1);
  TFPASS(fragments.getFirst() == frag1);
  TFPASS(fragments.getLast() == frag1);

  printf("first = %p, last = %p\n", fragments.getFirst(), fragments.getLast());
  
  fragments.insertFrag(frag1, frag2);
  TFPASS(fragments.getFirst() == frag1);
  TFPASS(fragments.getLast() == frag2);

  printf("first = %p, last = %p\n", fragments.getFirst(), fragments.getLast());

  fragments.insertFragBefore(frag1, frag3);
  TFPASS(fragments.getFirst() == frag3);
  TFPASS(fragments.getLast() == frag2);

  printf("first = %p, last = %p\n", fragments.getFirst(), fragments.getLast());
#endif
}
