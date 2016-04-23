/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2016 Hubert Figuiere
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

#include "tf_test.h"
#include "ut_string.h"

#define TFSUITE "core.af.util.string"

TFTEST_MAIN("UT_ensureValidXML")
{
    std::string str;
    char *pstr;
    bool result = UT_ensureValidXML(str);

    TFPASS(result);

    str = "foo\nbar\tbaz\rfizz buzz";
    TFPASS(UT_isValidXML(str.c_str()));
    pstr = strdup(str.c_str());
    TFPASS(!UT_validXML(pstr));
    result = UT_ensureValidXML(str);
    TFPASS(result);
    TFPASS(str == "foo\nbar\tbaz\rfizz buzz");
    TFPASS(str == pstr);
    free(pstr);

    str = "f\004oo\nbar\tbaz\rfizz\226 buzz";
    TFPASS(!UT_isValidXML(str.c_str()));
    pstr = strdup(str.c_str());
    TFPASS(UT_validXML(pstr));
    result = UT_ensureValidXML(str);
    TFPASS(!result);
    TFPASS(str == "foo\nbar\tbaz\rfizz buzz");
    TFPASS(str == pstr);
    free(pstr);

    str = "poo\nbar\tbaz\rbizz\226 fuzz";
    TFPASS(!UT_isValidXML(str.c_str()));
    pstr = strdup(str.c_str());
    TFPASS(UT_validXML(pstr));
    result = UT_ensureValidXML(str);
    TFPASS(!result);
    TFPASS(str == "poo\nbar\tbaz\rbizz fuzz");
    TFPASS(str == pstr);
    free(pstr);
}


/*UT_XML_cloneNoAmpersands*/

/*UT_XML_transNoAmpersands*/

/*UT_decodeUTF8string*/
