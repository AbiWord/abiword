/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifndef AP_DIALOG_FIELD_H
#define AP_DIALOG_FIELD_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

#define CURRENT_FIELD_SIZE 256

struct _fieldText
{
	char *m_Name;
	char *m_Tag;
};

static const _fieldText FieldFmts[] = {
	{ "Current time",		"time" },
	{ "Page number",		"page_number" },
	{ "Number of pages",	"page_count" },
	{ NULL,					NULL } };

class XAP_Frame;

class AP_Dialog_Field : public XAP_Dialog_NonPersistent
{
public:
	typedef enum { a_OK, a_CANCEL } tAnswer;

	AP_Dialog_Field(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	AP_Dialog_Field::tAnswer	getAnswer(void) const;
	const char *GetFieldFormat(void) const;
protected:
	AP_Dialog_Field::tAnswer	m_answer;
	int m_iFormatIndex;
};

#endif /* AP_DIALOG_FIELD_H */
