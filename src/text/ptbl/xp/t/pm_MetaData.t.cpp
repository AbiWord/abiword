/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "ut_bytebuf.h"
#include "tf_test.h"

#include "pm_MetaData.h"


TFTEST_MAIN("pm_MetaData")
{
    pm_MetaData meta1;
    UT_ByteBuf buf;

    buf.append((const UT_Byte*)"foobar", 6);
    
    TFPASS(meta1.empty());
    meta1.setRawData(buf);
    TFPASS(!meta1.empty());
    TFPASS(meta1.isRaw());
    TFPASS(meta1.rawData());
    

    pm_MetaData meta2;
    TFPASS(meta2.empty());

    meta2.setSubject("foobar");
    TFPASS(!meta2.empty());

    TFPASS(meta2.getData().empty());
    meta2.insertData("baz", "42");
    TFPASS(!meta2.getData().empty());
    TFPASS(meta2.getData().size() == 1);
    meta2.insertData("Cylon", "Number 6");
    TFPASS(meta2.getData().size() == 2);    
    meta2.insertData("baz", "69");
    TFPASS(meta2.getData().size() == 2);
}

