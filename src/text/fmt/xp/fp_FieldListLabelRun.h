/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#ifndef FP_FIELD_LIST_LABEL_RUN_H
#define FP_FIELD_LIST_LABEL_RUN_H

#include "ut_types.h"
#include "fp_Run.h"

class ABI_EXPORT fp_FieldListLabelRun : public fp_FieldRun
{
public:
	fp_FieldListLabelRun(fl_BlockLayout* pBL, UT_uint32 iOffsetFirst, UT_uint32 iLen);


	virtual bool			calculateValue(void);
	virtual bool            isListLabelField(void) { return true;}

private:
	virtual void			_lookupProperties(const PP_AttrProp * pSpanAP,
											  const PP_AttrProp * pBlockAP,
											  const PP_AttrProp * pSectionAP,
											  GR_Graphics * pG);

	virtual void			_draw(dg_DrawArgs*);

};


#endif	//FP_FIELD_LIST_LABEL_RUN_H
