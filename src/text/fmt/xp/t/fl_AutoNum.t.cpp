/* -*- mode: C++; tab-width: 2; c-basic-offset: 2; indent-tabs-mode: nil; -*- */
/*
 * © 2016 Hubert Figuière
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

#include "fl_AutoNum.h"
#include "pd_Document.h"

#define TFSUITE "core.text.fmt.autonum"

namespace {

static const char *DATA_FILE =
  "/test/wp/table.abw";

}

PD_Document *makeDocument()
{
  std::string data_file;
  TFPASS(TF_Test::ensure_test_data(DATA_FILE, data_file));

  PD_Document* doc = new PD_Document;

  UT_Error err = doc->readFromFile(data_file.c_str(), IEFT_Unknown, NULL);
  TFPASSEQ(err, UT_OK);
  return doc;
}

TFTEST_MAIN("fl_AutoNum")
{
  PD_Document* pDoc = makeDocument();
  FV_View *pView = nullptr;

  fl_AutoNumPtr autoNum = std::make_shared<fl_AutoNum>(2, 0, NUMBERED_LIST, 1, "*", ".",
                                                       pDoc, pView);

  TFPASS(pDoc->getListByID(2) == autoNum);

  autoNum->fixHierarchy();
  TFPASS(!autoNum->isDirty());

  TFPASS(autoNum->getType() == NUMBERED_LIST);
  TFPASS(strcmp(autoNum->getDelim(), "*") == 0);
  autoNum->setDelim("&");
  TFPASS(strcmp(autoNum->getDelim(), "&") == 0);
  TFPASS(autoNum->isDirty());

  TFPASS(strcmp(autoNum->getDecimal(), ".") == 0);
  autoNum->setDecimal(",");
  TFPASS(strcmp(autoNum->getDecimal(), ",") == 0);
  TFPASS(autoNum->isDirty());

  pDoc->addList(autoNum);
  autoNum->fixHierarchy();
//  autoNum->update(0);
//  TFPASS(!autoNum->isDirty());


  std::vector<std::string> attr;
  autoNum->getAttributes(attr, false);
  TFPASS(!attr.empty());

  pDoc->unref();
}
