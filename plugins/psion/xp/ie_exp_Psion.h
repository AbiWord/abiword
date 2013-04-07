/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2000,2001,2004 Frodo Looijaard <frodol@dds.nl>
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

/* This exporter is written by Frodo Looijaard <frodol@dds.nl> */

#ifndef IE_EXP_PSION_H
#define IE_EXP_PSION_H

#include "ie_exp.h"
#include "ie_impexp_Psion.h"
#include "pl_Listener.h"
#include "pd_Document.h"
#include "psiconv/data.h"

// The exporter/writer for Psion Files.


/*!
 * Sniffer class for Psion Word files
 *
 * This sniffer class is specific for Psion Word files. It does not extend
 * the basic IE_ExpSniffer class.
 */
class IE_Exp_Psion_Word_Sniffer : public IE_ExpSniffer
{
public:
	IE_Exp_Psion_Word_Sniffer (const char * _name): IE_ExpSniffer(_name) {}
	virtual ~IE_Exp_Psion_Word_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

/*!
 * Sniffer class for Psion TextEd files
 *
 * This sniffer class is specific for Psion TextEd files. It does not extend
 * the basic IE_ExpSniffer class.
 */
class IE_Exp_Psion_TextEd_Sniffer : public IE_ExpSniffer
{
public:
	IE_Exp_Psion_TextEd_Sniffer (const char * _name): IE_ExpSniffer(_name) {}
	virtual ~IE_Exp_Psion_TextEd_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};


/*!
 * Abstract listener class for Psion files.
 *
 * The actual Word and TextEd listeners are derived from this class.
 * Listener classes are used to traverse the AbiWord document, using the
 * defined callbacks whenever something interesting is found. This specific
 * class builds a psiconv representation of the AbiWord document internally.
 * The abstract createPsionFile method will need to be defined to return this
 * psiconv document, currently in Word or TextEd form.
 */
class PL_Psion_Listener : public PL_Listener
{
public:
	// Constructors and destructor
	PL_Psion_Listener(PD_Document * pDocument);
	virtual ~PL_Psion_Listener(void);

	// Overriding methods from the base class
	virtual bool populate(fl_ContainerLayout* sfh,
	                      const PX_ChangeRecord * pcr);
	virtual bool populateStrux(pf_Frag_Strux* sdh,
	                           const PX_ChangeRecord * pcr,
	                           fl_ContainerLayout* * psfh);

	// New public methods
	bool startDocument(void);
	bool finishDocument(void);
	virtual psiconv_file createPsionFile(void) = 0;

protected:
	// New data
	PD_Document *m_pDocument;
	psiconv_text_and_layout m_paragraphs;
	psiconv_word_styles_section m_styles;
	psiconv_page_header m_header,m_footer;

private:
	// New data
	bool m_inParagraph;
	enum sectionType { section_none,section_header,section_footer,
                       section_main };
	sectionType m_sectionType;

	psiconv_list m_currentParagraphText /* of psiconv_ucs2 */;
	psiconv_paragraph_layout m_currentParagraphPLayout;
	psiconv_character_layout m_currentParagraphCLayout;
	psiconv_in_line_layouts m_currentParagraphInLines;
	psiconv_s16 m_currentParagraphStyle;

	// New methods
	bool _writeText(const UT_UCSChar *p, UT_uint32 inlength,
	                UT_uint32 &outlength);
	bool _openParagraph(const PT_AttrPropIndex api);
	bool _closeParagraph(void);
	bool _addInLine(const PT_AttrPropIndex api,UT_uint32 textlen);
	bool _processStyles(void);
	bool _setStyleLayout(PD_Style *style,
					     psiconv_paragraph_layout para_layout,
                         psiconv_character_layout char_layout);
	bool _insertImage(const PT_AttrPropIndex api);

public:
	// The following three base class methods should never be called.
	virtual bool change(fl_ContainerLayout* /*sfh*/,
						const PX_ChangeRecord * /*pcr*/)
	                         { UT_ASSERT(UT_SHOULD_NOT_HAPPEN); return false; }
	virtual bool insertStrux(fl_ContainerLayout* /*sfh*/,
							 const PX_ChangeRecord * /*pcr*/,
							 pf_Frag_Strux* /*sdh*/,
							 PL_ListenerId /*lid*/,
							 void (* /*pfnBindHandles*/)
	                                            (pf_Frag_Strux* sdhNew,
	                                             PL_ListenerId lid,
	                                             fl_ContainerLayout* sfhNew))
		                     { UT_ASSERT(UT_SHOULD_NOT_HAPPEN); return false; }
	virtual bool signal(UT_uint32 /*iSignal*/)
	                         { UT_ASSERT(UT_SHOULD_NOT_HAPPEN); return false; }

};


/*!
 * Listener class for Psion Word files.
 *
 * The only important difference with its base class is that createPsionFile
 * returns an actual Psion Word document.
 */
class PL_Psion_Word_Listener: public PL_Psion_Listener
{
public:
	// Constructors and destructor
	PL_Psion_Word_Listener(PD_Document * pDocument): PL_Psion_Listener(pDocument) {}
	virtual ~PL_Psion_Word_Listener(void) { }

	// Public functions
	virtual psiconv_file createPsionFile(void);
};


/*!
 * Listener class for Psion TextEd files.
 *
 * The only important difference with its base class is that createPsionFile
 * returns an actual Psion TextEd document.
 */
class PL_Psion_TextEd_Listener: public PL_Psion_Listener
{
public:
	// Constructors and destructor
	PL_Psion_TextEd_Listener(PD_Document * pDocument): PL_Psion_Listener(pDocument) {}
	virtual ~PL_Psion_TextEd_Listener(void) { }

	// Public functions
	virtual psiconv_file createPsionFile(void);
};


/*!
 * Abstract base class for Psion exporters
 *
 * The actual Word and TextEd exporters are derived from this class.
 * Exporter classes are used to write an AbiWord file to disk. This
 * specific implementation is meant for Psion files. It extends its
 * base class with the _constructListener method, that should return
 * a corresponding listener (a Psion Word listener for Psion Word exporters,
 * and a Psion TextEd listener for Psion TextEd exporters). It also redefines
 * the _writeDocument method, which you do not need to override in the
 * derived classes.
 */
class IE_Exp_Psion: public IE_Exp
{
public:
	// Constructors and destructor
	IE_Exp_Psion(PD_Document * pDocument): IE_Exp(pDocument) { }
	virtual ~IE_Exp_Psion(void) { }

protected:
	// Overriding methods from the base class
	virtual UT_Error _writeDocument(void);

	// New methods.
	virtual PL_Psion_Listener *_constructListener(void) = 0;
};


/*!
 * Exporter Class for Psion TextEd files.
 *
 * The only method which this class overrides is the _constructListener
 * method, that is made to return a listener of type PL_Psion_TextEd_Listener.
 */
class IE_Exp_Psion_TextEd : public IE_Exp_Psion
{
public:
	// Constructors and destructor
	IE_Exp_Psion_TextEd(PD_Document * pDocument): IE_Exp_Psion(pDocument) {}
	virtual ~IE_Exp_Psion_TextEd(void) { }

protected:
	// Overriding methods from the base class
	virtual PL_Psion_Listener *_constructListener(void);
};


/*!
 * Exporter Class for Psion Word files.
 *
 * The only method which this class overrides is the _constructListener
 * method, that is made to return a listener of type PL_Psion_Word_Listener.
 */
class IE_Exp_Psion_Word : public IE_Exp_Psion
{
public:
	// Constructors and destructor
	IE_Exp_Psion_Word(PD_Document * pDocument): IE_Exp_Psion(pDocument) { }
	virtual ~IE_Exp_Psion_Word(void) { }

protected:
	// Overriding methods from the base class
	virtual PL_Psion_Listener *_constructListener(void);
};


#endif /* IE_EXP_PSION_H */
