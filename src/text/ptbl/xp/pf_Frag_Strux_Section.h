/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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


#ifndef PF_FRAG_STRUX_SECTION_H
#define PF_FRAG_STRUX_SECTION_H

#include "ut_types.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"

#define pf_FRAG_STRUX_SECTION_LENGTH 1

class pt_PieceTable;

/*!
 pf_Frag_Strux_Section represents structure information for
 a section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_Section : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_Section(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_Section();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionHdrFtr represents structure information for
 a header/footer section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionHdrFtr : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionHdrFtr(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionHdrFtr();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};

/*!
 pf_Frag_Strux_SectionEndnote represents structure information for
 a Endnote section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionEndnote : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionEndnote(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionEndnote();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionFrames represents structure information for
 a Frames section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionFrame : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionFrame(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionFrame();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionEndFrame represents structure information for
 a EndFrame section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionEndFrame : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionEndFrame(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionEndFrame();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionTable represents structure information for
 a table section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionTable : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionTable(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionTable();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionCell represents structure information for
 a cell section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionCell : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionCell(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionCell();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionFootnote represents structure information for
 a footnote section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionFootnote : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionFootnote(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionFootnote();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionAnnotation represents structure information for
 a Annotation section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionAnnotation : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionAnnotation(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionAnnotation();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};



/*!
 pf_Frag_Strux_SectionMarginnote represents structure information for
 a header/footer section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionMarginnote : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionMarginnote(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionMarginnote();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionEndTable represents structure information for
 the end of a Table section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionEndTable : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionEndTable(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionEndTable();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionEndCell represents structure information for
 the end of a Cell section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionEndCell : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionEndCell(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionEndCell();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionEndFootnote represents structure information for
 the end of a Footnote section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionEndFootnote : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionEndFootnote(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionEndFootnote();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionEndAnnotation represents structure information for
 the end of a Annotation section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionEndAnnotation : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionEndAnnotation(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionEndAnnotation();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};




/*!
 pf_Frag_Strux_SectionEndMarginnote represents structure information for
 the end of a Table section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionEndMarginnote : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionEndMarginnote(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionEndMarginnote();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};

/*!
 pf_Frag_Strux_SectionEndEndnote represents structure information for
 the end of a Endnote section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionEndEndnote : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionEndEndnote(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionEndEndnote();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionTOC represents structure information for
 a Table of Conents section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionTOC : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionTOC(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionTOC();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};


/*!
 pf_Frag_Strux_SectionEndTOC represents structure information for
 the end of a Table of Contents section in the document.
*/

class ABI_EXPORT pf_Frag_Strux_SectionEndTOC : public pf_Frag_Strux
{
public:
	pf_Frag_Strux_SectionEndTOC(pt_PieceTable * pPT,
						  PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Strux_SectionEndTOC();

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif
};

#endif /* PF_FRAG_STRUX_SECTION_H */
