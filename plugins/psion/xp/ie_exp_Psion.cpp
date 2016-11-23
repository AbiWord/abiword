/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2000, 2001, 2004 Frodo Looijaard <frodol@dds.nl>
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

//  Export Word or TextEd data from a Psion file. 
//  We use libpsiconv for the real work


// To do:
//		Add support for bullets
//		Better determination of font type (serif, sansserif or nonproportional)
//		Add support for other page-level layout
//		Add support for objects, fields, format marks

// Search for TODO for more things to do.


#include "ie_exp_Psion.h"

#include "ut_string.h"
#include "ut_stringbuf.h"
#include "ut_bytebuf.h"
#include "ut_units.h"
#include "ut_debugmsg.h"
#include "ut_wctomb.h"
#include "ut_assert.h"

#include "pt_Types.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "pd_Style.h"
#include "fd_Field.h"

#include "png.h"
#include <psiconv/generate.h>


/*********************
 * Local definitions *
 *********************/

/*! 
 * A ByteBuf with current location structure
 *
 * This is a very ugly and hackish implementation, copied from ut_png.cpp.
 * The idea is we need a ByteBuf which can remember where we have last read
 * it. This is used for the libpng stuff, which is itself quite ugly.
 */
struct _bb
{
    UT_ConstByteBufPtr pBB;
    UT_uint32 iCurPos;
};

/***********************
 * Auxiliary functions *
 ***********************/

/*!
 * Convert a AbiWord UTF8 string to a Psiconv UCS2 string.
 *
 * Returns NULL on error.
 */
static psiconv_ucs2 *utf8_to_ucs2(const gchar *input)
{
                                                                                
    UT_uint32 read=0,written=0;
	UT_uint32 i;
    char *intermediate;
    psiconv_ucs2 *result;

    if (!input)
        return NULL;
    read=written=0;
    intermediate = UT_convert((char *)input,strlen(input) * sizeof(*input),
                      "UTF-8","UCS-2",&read,&written);
    if (!(result = (psiconv_ucs2 *) malloc(sizeof(*result) * 
					                       (written / 2 + 1)))) {
		free(intermediate);
		return NULL;
	}
	for (i = 0; i < written/2; i++) 
		result[i] = intermediate[i*2] + (intermediate[i*2+1] << 8);
	result[i] = 0;
	free(intermediate);
	return result;
}


/*!  
 * Translate a hex digit character token to its decimal value
 *
 */
static int hexDigitToDec(char in)
{
	switch(in) {
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': return 10;
		case 'b': return 11;
		case 'c': return 12;
		case 'd': return 13;
		case 'e': return 14;
		case 'f': return 15;
		default: return 0;
	}
}

/*!
 * Parse a color value
 *
 * The color is in the input parameter, in AbiWord format.
 * Example formats: 000000  ffffff  ab6e10
 */
static void parseColor(const char *input,psiconv_color color)
{
	color->red = hexDigitToDec(input[0])*16+hexDigitToDec(input[1]);
	color->green = hexDigitToDec(input[2])*16+hexDigitToDec(input[3]);
	color->blue = hexDigitToDec(input[4])*16+hexDigitToDec(input[5]);
}

/*! 
 * Parse a single tab (clobbering the input!)
 *
 * The tab is in the input parameter, in AbiWord format 
 * Example formats: 7.315cm/C  6.4322in/R  
 * Note that the input string is changed for efficiency reasons. As this
 * function is only meant to be called from parseTabs, that is no problem.
 */
static void parseTab(char *input,psiconv_tab tab)
{
	char *slash;
	slash = strchr(input,'/');
	tab->kind = psiconv_tab_left;
	if (slash) {
		if (*(slash+1) == 'R')
			tab->kind = psiconv_tab_right;
		else if (*(slash+1) == 'C')
			tab->kind = psiconv_tab_centre;
		*slash = '\000';
	}
	tab->location = (float)UT_convertToDimension(input,DIM_CM);
}


/*!
 * Parse all tabs
 *
 * The tabs are in the input parameter, in AbiWord format.
 * Example format: 7.315cm/C,14.641cm/R
 */
static bool parseTabs(const char *input,psiconv_tab_list tabs)
{
	const char *currentPos;
	const char *nextPos;
	char *copy;
	struct psiconv_tab_s tab;

	currentPos = input;
	while (*currentPos != '\000') {
		// Determine where this tab ends in the input
		nextPos = strchr(currentPos,',');
		if (!nextPos)
			nextPos = strchr(currentPos,'\000');
		// Copy the current tab value to a newly allocated variable
		if (!(copy = (char *) malloc(nextPos - currentPos + 1)))
			return false;
		memcpy(copy,currentPos,nextPos-currentPos);
		copy[nextPos - currentPos] = '\000';
		// Parse the tab
		parseTab(copy,&tab);
		// Free the allocated variable
		free(copy);
		// Add the psiconv tab to the list
		if (psiconv_list_add(tabs,&tab))
			return false;
		// Skip tab seperators
		currentPos = nextPos;
		while ((*currentPos == ',' || *currentPos == ' '))
			currentPos++;
	}
	return true;
}

/*!
 * Parse a font
 */
static bool parseFont(const char *fontname,psiconv_font font)
{
	psiconv_ucs2 *tempstr;
	const psiconv_ucs2 text_Courier[] = {'C','o','u','r','i','e','r',0 };
	const psiconv_ucs2 text_Mono[] = {'M','o','n','o',0 };
	const psiconv_ucs2 text_Arial[] = {'A','r','i','a','l',0 };
	const psiconv_ucs2 text_Goth[] = {'G','o','t','h',0 };
	const psiconv_ucs2 text_Helvetic[] = 
	                              {'H','e','l','v','e','t','i','c',0 };
	const psiconv_ucs2 text_Univers[] = {'U','n','i','v','e','r','s',0 };
	const psiconv_ucs2 text_Sans[] = {'S','a','n','s',0 };

	
	// If utf8_to_ucs2 fails, we still need to have the default fontname around
	// to reassign it. And if it succeeds, we need to free it. Hence the
	// juggling around with tempstr.
	tempstr = font->name;
	if (!(font->name = utf8_to_ucs2(fontname))) {
		font->name = tempstr;
		return false;
	}
	free(tempstr);
	
	// There may be a good general way to do this, but I do
	// not know how. We need to determine whether we have a proportional
	// serifed font, a proportional non-serifed font, or a non-proportional
	// font.
	if (psiconv_unicode_strstr(font->name,text_Courier) ||
	    psiconv_unicode_strstr(font->name,text_Mono))
		font->screenfont = psiconv_font_nonprop;
	else if (psiconv_unicode_strstr(font->name,text_Arial) ||
	         psiconv_unicode_strstr(font->name,text_Goth) ||
	         psiconv_unicode_strstr(font->name,text_Helvetic) ||
	         psiconv_unicode_strstr(font->name,text_Univers) ||
	         psiconv_unicode_strstr(font->name,text_Sans))
		font->screenfont = psiconv_font_sansserif;
	else
		font->screenfont = psiconv_font_serif;
	
	return true;
}


/*!
 * Update all character layout with the properties pointed to.
 */
static bool updateCharacterLayout(const PP_AttrProp *pAP,
                                  psiconv_character_layout layout)
{
	const gchar* szValue;

	// Font name
	if (pAP->getProperty("font-family",szValue)) 
		if (!parseFont(szValue,layout->font))
			return false;
		
	// Font size
	if (pAP->getProperty("font-size",szValue))
		layout->font_size = 
					(float)UT_convertToDimension((const char *) szValue,DIM_PT);
	
	// Bold
	if (pAP->getProperty("font-weight",szValue)) {
		if (!strcmp((const char *) szValue,"bold"))
			layout->bold = psiconv_bool_true;
		else
			layout->bold = psiconv_bool_false;
	}
	
	// Italic
	if (pAP->getProperty("font-style",szValue)) {
		if (!strcmp((const char *) szValue,"italic"))
			layout->italic = psiconv_bool_true;
		else
			layout->italic = psiconv_bool_false;
	}
	
	// Underline and line-through are collected under text-decoration
	// (AbiWord really looked too much to CSS). For the Psion, they are
	// two independent settings.
	if (pAP->getProperty("text-decoration",szValue)) {
		if (strstr((const char *) szValue,"underline")) 
			layout->underline = psiconv_bool_true;
		else
			layout->underline = psiconv_bool_false;
		if (strstr((const char *) szValue,"line-through")) 
			layout->strikethrough = psiconv_bool_true;
		else
			layout->strikethrough = psiconv_bool_false;
	}
	
	// Superscript and subscript
	if (pAP->getProperty("text-position",szValue)) {
		if (!strcmp((const char *) szValue,"superscript"))
			layout->super_sub = psiconv_superscript;
		else if (!strcmp((const char *) szValue,"subscript"))
			layout->super_sub = psiconv_subscript;
		else
			layout->super_sub = psiconv_normalscript;
	}
	
	// Text color
	if (pAP->getProperty("color",szValue)) 
		parseColor((char *) szValue,layout->color);
	
	// Background color
	if (pAP->getProperty("bgcolor",szValue)) 
		parseColor((char *) szValue,layout->back_color);
	
	return true;
}


/*!
 * Update all paragraph layout with the properties pointed to.
 */
static bool updateParagraphLayout(const PP_AttrProp *pAP,
                                  psiconv_paragraph_layout layout)
{
	const gchar* szValue;
	bool widowsorphans;

	// Indentation left, right and first line.
	if (pAP->getProperty("margin-left",szValue))
		layout->indent_left = 
		        (psiconv_length_t) UT_convertToDimension((const char *) szValue,
	                                                     DIM_CM);
	if (pAP->getProperty("margin-right",szValue)) 
		layout->indent_right = 
				(psiconv_length_t) UT_convertToDimension((const char *) szValue,
	                                                     DIM_CM);
	if (pAP->getProperty("text-indent",szValue)) 
		layout->indent_first = 
			    (psiconv_length_t) UT_convertToDimension((const char *) szValue,
	                                                     DIM_CM);


	// Text justification
	if (pAP->getProperty("text-align",szValue)) {
		if (!strcmp((const char *) szValue,"center"))
			layout->justify_hor = psiconv_justify_centre;
		else if (!strcmp((const char *) szValue,"right"))
			layout->justify_hor = psiconv_justify_right;
		else if (!strcmp((const char *) szValue,"justify"))
			layout->justify_hor = psiconv_justify_full;
		else
			layout->justify_hor = psiconv_justify_left;
	}
	
#if 0
	char *tempstr;
	// Muchos trouble. Forget about it for now.
	if (pAP->getProperty("line-height",szValue)) {
		layout->linespacing_exact =
			   szValue[strlen((char *)szValue)-1]=='+'?psiconv_bool_false:
													   psiconv_bool_true;
		tempstr = g_strdup((char *)szValue);
		if (!layout->linespacing_exact)
			tempstr[strlen((char *)szValue)-1] = '\000';
		layout->linespacing = 
					  (psiconv_size_t)UT_convertToDimension(tempstr,DIM_PT);
		free(tempstr);
	}
#endif
	
	// Space above and below paragraphs
	if (pAP->getProperty("margin-top",szValue)) 
		layout->space_above = 
				   (psiconv_size_t)UT_convertToDimension((const char *) szValue,
	                                                     DIM_PT);
	if (pAP->getProperty("margin-bottom",szValue)) 
		layout->space_below = 
				   (psiconv_size_t)UT_convertToDimension((const char *) szValue,
	                                                     DIM_PT);

	// Text flow on page breaks: keep paragraph on one page, keep paragraph
	// with next paragraph on one page, protect agains widows and orphans.
	if (pAP->getProperty("keep-together",szValue)) { 
		if (!strcmp((const char *) szValue,"yes"))
			layout->keep_together = psiconv_bool_true;
		else
			layout->keep_together = psiconv_bool_false;
	}
	if (pAP->getProperty("keep-with-next",szValue)) {
		if (!strcmp((const char *) szValue,"yes"))
			layout->keep_with_next = psiconv_bool_true;
		else
			layout->keep_with_next = psiconv_bool_false;
	}
	// Set widowsorphans if either widows or orphans is set and 
	// unequal to "0".
	widowsorphans = false;
	if (pAP->getProperty("widows",szValue)) 
		widowsorphans |= (strcmp((const char *) szValue,"0") != 0);
	if (pAP->getProperty("orphans",szValue)) 
		widowsorphans |= (strcmp((const char *) szValue,"0") != 0);
	layout->no_widow_protection = 
					   widowsorphans?psiconv_bool_false:psiconv_bool_true;
	
	// Tabs. We have both a default tab interval, and specific tab stops here.
	if (pAP->getProperty("default-tab-interval",szValue))  
		layout->tabs->normal = 
			     (psiconv_length_t)UT_convertToDimension((const char *) szValue,
	                                                     DIM_CM);
	if (pAP->getProperty("tabstops",szValue))  
		if (!parseTabs((char *)szValue,layout->tabs->extras)) 
			return false;
	
	// TODO: Bullets
	return true;
}

/*!
 * Auxiliary function for use with libpng: read data from a ByteBuf.
 *
 * This function reads data for use with libpng. The data was put into a
 * ByteBuf. Libpng does not tell us what data we have already read, so we
 * must remember that ourselves. That is why we use the ultra-ugly _bb.
 */
static void read_png_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    struct _bb* p = (struct _bb*) png_get_io_ptr(png_ptr);
    UT_DEBUGMSG(("PSION: read_png_data: %d bytes at %d\n",length,p->iCurPos));
    const UT_Byte* pBytes = p->pBB->getPointer(0);
                                              		
	memcpy(data, pBytes + p->iCurPos, length);
	p->iCurPos += length;
}

/***********************************
 * class IE_Exp_Psion_Word_Sniffer *
 ***********************************/

/*!
 * Look at the extension to guess whether this is a Psion Word file.
 *
 * Actually, the Psion itself does not use extensions (much), so I just
 * made up my own convention (.psiword) here. It's better than nothing.
 */
bool IE_Exp_Psion_Word_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!g_ascii_strcasecmp(szSuffix,".psiword"));
}

/*! 
 * Create an IE_Exp_Psion_Word object
 */
UT_Error IE_Exp_Psion_Word_Sniffer::constructExporter(PD_Document * pDocument,
											          IE_Exp ** ppie)
{
	IE_Exp_Psion_Word *p = new IE_Exp_Psion_Word(pDocument);
	*ppie = p;
	return UT_OK;
}

/*!
 * Some import filter settings. We use the .psiword extension
 */
bool IE_Exp_Psion_Word_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "Psion Word (.psiword)";
	*pszSuffixList = "*.psiword";
	*ft = getFileType();
	return true;
}


/*************************************
 * class IE_Exp_Psion_TextEd_Sniffer *
 *************************************/

/*!
 * Look at the extension to guess whether this is a Psion TextEd file.
 *
 * Actually, the Psion itself does not use extensions (much), so I just
 * made up my own convention (.psitext) here. It's better than nothing.
 */
bool IE_Exp_Psion_TextEd_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!g_ascii_strcasecmp(szSuffix,".psitext"));
}

/*! 
 * Create an IE_Exp_Psion_TextEd object
 */
UT_Error IE_Exp_Psion_TextEd_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_Psion_TextEd * p = new IE_Exp_Psion_TextEd(pDocument);
	*ppie = p;
	return UT_OK;
}

/*!
 * Some import filter settings. We use the .psitext extension
 */
bool IE_Exp_Psion_TextEd_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "Psion TextEd (.psitext)";
	*pszSuffixList = "*.psitext";
	*ft = getFileType();
	return true;
}


/**********************
 * class IE_Exp_Psion *
 **********************/

/*!
 * Write a Psion document to file.
 *
 * This function writes a document to file. To do this, we must first
 * construct a psiconv_file (a sort of abstract syntax tree of the document)
 * then translate it to a psiconv_buffer (the file in Psion format), and
 * finally write the contents of this buffer to a file.
 * As most of the work is the same for any type of Psion file, we define
 * this method here in the base exporter class. The only thing that is
 * really different between TextEd and Word files is the psiconv_file;
 * so we construct it through the virtual method _createPsionFile.
 */
UT_Error IE_Exp_Psion::_writeDocument(void)
{
	const int MAXBUFLEN = 512;
	char buf[MAXBUFLEN];
	char *pByte;
	unsigned int i;
	int iRes;
	PL_Psion_Listener *listener;
	psiconv_file psionfile;
	psiconv_buffer psiondump;
	psiconv_config config;
//	psiconv_text_and_layout paragraphs;
//	psiconv_word_style_list styles;

	
	// A whole cascade of commands, with only one purpose:
    // create a proper listener, signal it to traverse the document and
	// let it create a complete Psion file.	
	if (!(listener = _constructListener()) ||
		!listener->startDocument() ||
		!getDoc()->tellListener(listener) ||
		!listener->finishDocument() ||
	    !(psionfile = listener->createPsionFile())) {
		delete listener;
		return UT_IE_COULDNOTWRITE;
	}
	delete listener;
	
	// Initialize a Psiconv config structure
	config = psiconv_config_default();
	if (!config)
		return UT_IE_NOMEMORY;
	config->error_handler = &psion_error_handler;
	psiconv_config_read(NULL,&config);
	
	// Write the file to a buffer (psiondump), and free all allocated data
	iRes = psiconv_write(config,&psiondump,psionfile);
	psiconv_free_file(psionfile);
	psiconv_config_free(config);
	if (iRes)
		return UT_IE_COULDNOTWRITE;

	// Write the content of psiondump to file. We do this in 
	// MAXBUFFERLEN chunks, because it would be horribly slow if we did it
	// byte by byte. Note that we can't just access the psiconv_buffer,
	// we have to read it byte by byte.
	for (i = 0; i < psiconv_buffer_length(psiondump); i++) {
		if (!(pByte = (char *) psiconv_buffer_get(psiondump,i))) {
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			psiconv_buffer_free(psiondump);
			return UT_IE_COULDNOTWRITE;
		}
		buf[i % MAXBUFLEN] = *pByte;
		if ((i % MAXBUFLEN == MAXBUFLEN-1) || 
		    (i == psiconv_buffer_length(psiondump) - 1)) 
		write(buf,(i % MAXBUFLEN)+1);
		if (m_error) {
			psiconv_buffer_free(psiondump);
			return UT_IE_COULDNOTWRITE;
		}
	}

	// We are finished. Free the buffer and return happily.
	psiconv_buffer_free(psiondump);
	return UT_OK;
}



/**************************
 * class PL_Psion_Listener *
 **************************/

/*! Constructor
 *
 * The listener uses several psiconv_file fragments while building
 * paragraphs (and perhaps other things). We can't allocate them
 * here, because we can't fail in a constructor.
 * At all places, we use NULL to indicate an unallocated structure.
 */
PL_Psion_Listener::PL_Psion_Listener(PD_Document * pDocument):
  m_pDocument(pDocument), 
  m_paragraphs(NULL),m_styles(NULL),
  m_header(NULL),m_footer(NULL),
  m_inParagraph(false),
  m_sectionType(section_none),
  m_currentParagraphText(NULL),
  m_currentParagraphPLayout(NULL),
  m_currentParagraphCLayout(NULL), 
  m_currentParagraphInLines(NULL),
  m_currentParagraphStyle(0)
{
}

/*! 
 * Destructor
 *
 * Deallocate those psiconv_file fragments that are currently allocated.
 * NULL is used to indicate an unallocated structure.
 */
PL_Psion_Listener::~PL_Psion_Listener(void)
{
	if (m_currentParagraphPLayout)
		psiconv_free_paragraph_layout(m_currentParagraphPLayout);
	if (m_currentParagraphCLayout)
		psiconv_free_character_layout(m_currentParagraphCLayout);
	if (m_currentParagraphInLines)
		psiconv_list_free(m_currentParagraphInLines);
	if (m_currentParagraphText)
		psiconv_list_free(m_currentParagraphText);
	if (m_paragraphs)
		psiconv_list_free(m_paragraphs);
	if (m_header)
		psiconv_free_page_header(m_header);
	if (m_footer)
		psiconv_free_page_header(m_footer);
}

/*!
 * Initialize a new Psion Listener.
 *
 * Call this right after the constructor.
 * We can't call it from the constructor, because our constructor may not fail.
 * We initialize all kinds of private variables. Some juggling is done with
 * NULL values to make sure everything can be safely deallocated at any time
 * by our destructor.
 * We also parse all styles here. This is needed further on, and listeners
 * do not traverse styles.
 */
bool PL_Psion_Listener::startDocument(void)
{
	// Allocate list of characters. This list will contain Psiconv ucs2
	// characters. It will only be freed in the destructor; use
	// psiconv_list_empty if you want to clear it.
	if (!(m_currentParagraphText = psiconv_list_new(sizeof(psiconv_ucs2))))
		return false;
	
	// Allocate the main text, which is a list of Psiconv paragraphs.
	if (!(m_paragraphs = psiconv_list_new(sizeof(struct psiconv_paragraph_s))))
		return false;
	
	// Allocate default header with no text
	if (!(m_header =  (psiconv_page_header) malloc(sizeof(*m_header))))
		return false;
	m_header->on_first_page = psiconv_bool_true;
	m_header->base_paragraph_layout = NULL;
	m_header->base_character_layout = NULL;
	m_header->text = NULL;
	if (!(m_header->base_paragraph_layout = psiconv_basic_paragraph_layout()))
		return false;
	if (!(m_header->base_character_layout = psiconv_basic_character_layout()))
		return false;
	if (!(m_header->text = (psiconv_texted_section) malloc(sizeof(*m_header->text))))
		return false;
	m_header->text->paragraphs = NULL;
	if (!(m_header->text->paragraphs = psiconv_list_new(sizeof(struct psiconv_paragraph_s))))
		return false;
	
	// Allocate default footer with no text
	if (!(m_footer = (psiconv_page_header) malloc(sizeof(*m_footer))))
		return false;
	m_footer->on_first_page = psiconv_bool_true;
	m_footer->base_paragraph_layout = NULL;
	m_footer->base_character_layout = NULL;
	m_footer->text = NULL;
	if (!(m_footer->base_paragraph_layout = psiconv_basic_paragraph_layout()))
		return false;
	if (!(m_footer->base_character_layout = psiconv_basic_character_layout()))
		return false;
	if (!(m_footer->text = (psiconv_texted_section) malloc(sizeof(*m_footer->text))))
		return false;
	m_footer->text->paragraphs = NULL;
	if (!(m_footer->text->paragraphs = psiconv_list_new(sizeof(struct psiconv_paragraph_s))))
		return false;

	// Parse all styles
	if (!(_processStyles()))
		return false;
	return true;
}

/*!
 * Finish the traversing of the document by the Listener.
 *
 * Call this after calling tellListener to traverse the document.
 * Closes remaining paragraphs and sections. 
 */
bool PL_Psion_Listener::finishDocument(void)
{
	return _closeParagraph();
}

/*! Standard listener callback for in-block data
 *
 * This method is called when a span, object or format mark is found in the
 * document while traversing it.
 */
bool PL_Psion_Listener::populate(fl_ContainerLayout*,
                                const PX_ChangeRecord * pcr)
{
	PT_BufIndex bi;
	const PX_ChangeRecord_Span * pcrs = NULL;
	const PX_ChangeRecord_Object * pcro = NULL;
	const fd_Field *field = NULL;
	UT_uint32 textlen;
	
	const PT_AttrPropIndex api = pcr->getIndexAP();

	switch(pcr->getType()) {
		case PX_ChangeRecord::PXT_InsertSpan:
			// A text with its layout. We first append the text to the
			// current paragraph, and afterwards its layout. Note that we
			// can safely do the static_cast.
			pcrs = static_cast<const PX_ChangeRecord_Span *> (pcr);
			bi = pcrs->getBufIndex();
			if (! _writeText(m_pDocument->getPointer(bi),pcrs->getLength(),
			                 textlen))
				return false;
			return _addInLine(api,textlen);
			break;
		case PX_ChangeRecord::PXT_InsertObject:
			pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			switch (pcro->getObjectType()) {
				case PTO_Image:
					return _insertImage(api);
				case PTO_Field:
					field = pcro->getField();
					if (field->getFieldType() == fd_Field::FD_ListLabel) {
						// Ugly as hell. We have found a list label, so we 
						// need to make a bulleted paragraph here.
						if (m_inParagraph) 
							m_currentParagraphPLayout->bullet->on = psiconv_bool_true;
						else {
							UT_DEBUGMSG(("PSION: List label found outside paragraph (ignored)\n"));
						}
					} else {
						UT_DEBUGMSG(("PSION: Field found (ignored)\n"));
					}
					return true;
				case PTO_Hyperlink:
				case PTO_Bookmark:
				default:
					UT_DEBUGMSG(("PSION: Unknown or unsupported type of object found (ignored)\n"));
					return true;
			}
		case PX_ChangeRecord::PXT_InsertFmtMark:
			// Format marks are not supported at the moment: ignore
			UT_DEBUGMSG(("PSION: Insert Format Mark (ignored)\n"));
			return true;
		default:
			// We should have covered all possibilities.
			UT_ASSERT(0);
			return false;
	}

}


/*! Standard listener callback for block and section data
 *
 * This method is called whenever a block (paragraph) or section is found in
 * the document while traversing it.
 */
bool PL_Psion_Listener::populateStrux(pf_Frag_Strux* /*sdh*/,
                                      const PX_ChangeRecord * pcr,
                                      fl_ContainerLayout* * /*psfh*/)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = 
	                        static_cast<const PX_ChangeRecord_Strux *> (pcr);
	PT_AttrPropIndex api;
	const PP_AttrProp * pAP = NULL;
	const gchar *sectiontype;

	switch (pcrx->getStruxType()) {
		case PTX_Block:
			// Start of a new paragraph. We start a new paragraph, setting
		    // all paragraph-level layouts.
			if (!_openParagraph(pcr->getIndexAP()))
				return false;
			return true;
			break;
		case PTX_Section:
			// A new (main) section. If there is more than one, they are
			// just appended. We first close the current paragraph, because
			// belongs to the current section. No harm if no paragraph is open.
			if (!_closeParagraph())
				return false;
			m_sectionType = section_main;
			UT_DEBUGMSG(("PSION: New main section\n"));
			return true;
			break;
		case PTX_SectionHdrFtr:
			// A new header or footer section. If there is more than one,
			// they are just appended (not good, but it will have to do for 
			// now). We first close the current paragraph, because
			// belongs to the current section. No harm if no paragraph is open.
			// We have to do some work to determine whether this is a header
			// or a footer.
			// Note that if the Psion file format had some more possibilities,
			// we good associate sections with their headers/footers like
			// AbiWord does. Regrettably, a Psion file can only have one
			// header and one footer, and one main section.
			if (!_closeParagraph())
				return false;
			if ((api = pcr->getIndexAP()) && 
				m_pDocument->getAttrProp(api,&pAP) && pAP &&
			    (pAP->getAttribute("type",sectiontype) && sectiontype)) {
				if (!strcmp(sectiontype,"header")) {
					UT_DEBUGMSG(("PSION: New header section\n"));
					m_sectionType = section_header;
				} else if (!strcmp(sectiontype,"footer")) {
					UT_DEBUGMSG(("PSION: New footer section\n"));
					m_sectionType = section_footer;
				} else {
					UT_DEBUGMSG(("PSION: Unknown section type (%s), ignored \n",
					             sectiontype));
					m_sectionType = section_none;
				}
			} else
				return false;	
			return true;
			break;
			
		case PTX_SectionTable:
		case PTX_EndTable:
		case PTX_SectionCell:
		case PTX_EndCell:
			// We do not support them, but there is no harm in them.
			return true;

		case PTX_EndFrame:
		case PTX_EndMarginnote:
		case PTX_EndFootnote:
		case PTX_SectionFrame:
		case PTX_SectionMarginnote:
		case PTX_SectionFootnote:
		case PTX_EndEndnote:
		default:
			// Things that should never happen.
			UT_ASSERT_NOT_REACHED();
			return false;
	}
}


/*! 
 * Add some AbiWord document text to the current paragraph
 *
 * Add some text to the current paragraph. Returns the length of the 
 * written text in outLength; this may be more or less than inLength,
 * because some input characters may need to be represented by more than one
 * output character, and because we may ignore some input characters.
 */
bool PL_Psion_Listener::_writeText(const UT_UCSChar *p, UT_uint32 inLength,
                                  UT_uint32 &outLength)
{
	UT_uint32 i;
	psiconv_ucs2 character;

#ifdef DEBUG
	UT_UTF8Stringbuf buffer;
	buffer.appendUCS4(p,inLength);
	UT_DEBUGMSG(("PSION: text `%s'\n",buffer.data()));
#endif
	// Append all characters one by one to the current paragraph
	outLength = 0;
	for (i = 0; i < inLength; i++) {
		if (p[i] == UCS_ABI_OBJECT)
			continue;
		else if (p[i] == UCS_TAB)
			character = 0x09;
		else if ((p[i] == UCS_LF) || (p[i] == UCS_CR))
			character = 0x07;
		else if (p[i] == UCS_FF)
			character = 0x08;
		else if ((p[i] == UCS_EN_SPACE) || (p[i] == UCS_EM_SPACE))
			character = 0x0f;   // I am unsure whether this is right!
		else if ((p[i] == UCS_EN_DASH) || (p[i] == UCS_EM_DASH))
			character = 0x0b;
		else if (p[i] < 0x20)
			continue;
		else if (p[i] > 0xffff)
			continue;
		else
			character = p[i];
		if (psiconv_list_add(m_currentParagraphText,&character))
			return false;
		outLength ++;
	}

	return true;
}

/*! 
 * Open a new paragraph, and set paragraph-level layout.
 *
 */
bool PL_Psion_Listener::_openParagraph(const PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	const gchar *stylename;
	psiconv_string_t stylename_ucs2 = NULL;
	psiconv_word_style style;
	int stylenr;

	// Close the last paragraph, in case it is still open.
	_closeParagraph();
	UT_DEBUGMSG(("PSION: Open paragraph\n"));
	
	// Get all attributes.
	if (!m_pDocument->getAttrProp(api,&pAP) || !pAP)
		goto ERROR1;

	// Set the base layout of this paragraph. If we can find its style,
	// we use it. If not, we use a Psion default. Note that a style can
	// contain both paragraph-level and character-level settings, even though
	// it is a paragraph style.
	if (pAP->getAttribute("style",stylename) && stylename &&
		(stylename_ucs2 = utf8_to_ucs2(stylename)) &&
		!psiconv_find_style(m_styles,stylename_ucs2,&stylenr) &&
		(style = psiconv_get_style(m_styles,stylenr))) {
		if (!(m_currentParagraphPLayout = 
			                psiconv_clone_paragraph_layout(style->paragraph)))
			goto ERROR1;
		if (!(m_currentParagraphCLayout = 
			                psiconv_clone_character_layout(style->character)))
			goto ERROR2;
		m_currentParagraphStyle = stylenr;
	} else {
		if (!(m_currentParagraphPLayout = psiconv_basic_paragraph_layout()))
			goto ERROR1;
		if (!(m_currentParagraphCLayout = psiconv_basic_character_layout())) 
			goto ERROR2;
		m_currentParagraphStyle = 0;
	}
	if (stylename_ucs2) {
		free(stylename_ucs2);
		stylename_ucs2 = NULL;
	}
		
	// New paragraph, new text.
	psiconv_list_empty(m_currentParagraphText);

	// New list of in-line layout
	if (!(m_currentParagraphInLines = 
	              psiconv_list_new(sizeof(struct psiconv_in_line_layout_s)))) 
		goto ERROR3;
	m_inParagraph = true;
	
	// Apply all layout for this paragraph (besides the style) which applies to
	// the whole paragraph.
	if(!updateParagraphLayout(pAP,m_currentParagraphPLayout))
		goto ERROR4;
	if(!updateCharacterLayout(pAP,m_currentParagraphCLayout))
		goto ERROR4;
	
	return true;

ERROR4:
	psiconv_list_free(m_currentParagraphInLines);
	m_currentParagraphInLines = NULL;
ERROR3:
	psiconv_free_character_layout(m_currentParagraphCLayout);
	m_currentParagraphCLayout = NULL;
ERROR2:
	psiconv_free_paragraph_layout(m_currentParagraphPLayout);
	m_currentParagraphPLayout = NULL;
ERROR1:
	if (stylename_ucs2) 
		free(stylename_ucs2);
	m_inParagraph = false;
	UT_DEBUGMSG(("PSION: open paragraph FAILED\n"));
	return false;
}

/*! Close the paragraph
 *
 * The current paragraph stuff is added to the paragraph list.
 * It is safe to call this function if no paragraph is opened.
 */
bool PL_Psion_Listener::_closeParagraph(void)
{
	struct psiconv_paragraph_s para;
	
	// It is safe to call us when we are not actually in a paragraph;
	// we will just nop.
	if (m_inParagraph) {
		UT_DEBUGMSG(("PSION: Close paragraph\n"));

		// If we are not in a valid section, we will throw away the current
		// paragraph, with no real harm done. We need to deallocate a bunch
		// of things, though.
		if (m_sectionType == section_none) {
			UT_DEBUGMSG(("PSION: Closing paragraph while not in a section\n"));
			psiconv_list_empty(m_currentParagraphText);
			psiconv_free_character_layout(m_currentParagraphCLayout);
			m_currentParagraphCLayout = NULL;
			psiconv_free_paragraph_layout(m_currentParagraphPLayout);
			m_currentParagraphPLayout = NULL;
			psiconv_list_free(m_currentParagraphInLines);
			m_currentParagraphInLines = NULL;
			m_inParagraph = false;
			return true;
		}
		
		// Modify the list of Psiconv UCS2 characters to a Psiconv string
		// and put it in the paragraph.
		if (!(para.text = psiconv_unicode_from_list(m_currentParagraphText)))
			goto ERROR1;
		psiconv_list_empty(m_currentParagraphText);
		
		// Get the base paragraph and character layout, its style and inlines.
		para.base_character = m_currentParagraphCLayout;
		m_currentParagraphCLayout = NULL;
		para.base_paragraph = m_currentParagraphPLayout;
		m_currentParagraphPLayout = NULL;
		para.base_style = m_currentParagraphStyle;
		para.in_lines = m_currentParagraphInLines;
		m_currentParagraphInLines = NULL;
		
		// We don't do replacements, because I do not really understand
		// the Psion-side of it yet.
		if (!(para.replacements = 
		            psiconv_list_new(sizeof(struct psiconv_replacement_s))))
			goto ERROR2;
				
		// Add the paragraph to the proper list.
		if (m_sectionType == section_main) {
			UT_DEBUGMSG(("PSION: Adding paragraph to the main section\n"));
			if (psiconv_list_add(m_paragraphs,&para))
				goto ERROR3;
		} else if (m_sectionType == section_header) {
			UT_DEBUGMSG(("PSION: Adding paragraph to the header section\n"));
			if (psiconv_list_add(m_header->text->paragraphs,&para)) 
				goto ERROR3;
		} else if (m_sectionType == section_footer) {
			UT_DEBUGMSG(("PSION: Adding paragraph to the footer section\n"));
			if (psiconv_list_add(m_footer->text->paragraphs,&para))
				goto ERROR3;
		}
		
		m_inParagraph = false;
	}
	return true;

// Cleanup if something goes wrong halfway-through.
ERROR3:
	psiconv_list_free(para.replacements);
ERROR2:
	psiconv_list_free(para.in_lines);
	psiconv_free_paragraph_layout(para.base_paragraph);
	psiconv_free_character_layout(para.base_character);
	free(para.text);
ERROR1:
	m_inParagraph = false;
	UT_DEBUGMSG(("PSION: close paragraph FAILED\n"));

	return false;
}

/*!
 * Parse the current in-line formatting
 */
bool PL_Psion_Listener::_addInLine(const PT_AttrPropIndex api,UT_uint32 textlen)
{
	const PP_AttrProp * pAP = NULL;
	psiconv_in_line_layout curInLine;

	UT_DEBUGMSG(("PSION: set inline formatting (%d)\n",textlen));
	// We really should be in a paragraph right now.
	if (!m_inParagraph) {
		UT_ASSERT(m_inParagraph);
		goto ERROR1;
	}
	
	// Allocate a new psiconv_in_line_layout
	if (!(curInLine = (psiconv_in_line_layout) malloc(sizeof(*curInLine))))
		goto ERROR1;

	// Fill CurInLine with sane defaults
	curInLine->object = NULL;
	curInLine->length = textlen;
	if (!(curInLine->layout =
	                psiconv_clone_character_layout(m_currentParagraphCLayout)))
		goto ERROR2;

	// Set all character-based attributes
	if (m_pDocument->getAttrProp(api,&pAP) && pAP) {
		if (!updateCharacterLayout(pAP,curInLine->layout))
			goto ERROR3;		
	}
	
	// Add to the list of inlines
	if (psiconv_list_add(m_currentParagraphInLines,curInLine))
		goto ERROR3;
	
	// Free the inline and return succes.
	free(curInLine);
	return true;
	
ERROR3:
	psiconv_free_character_layout(curInLine->layout);
ERROR2:
	free(curInLine);
ERROR1:
	UT_DEBUGMSG(("PSION: set inline formatting FAILED\n"));
	return false;
}


/*! 
 * Determine the layout of this style, considering the styles it is based on.
 *
 * A paragraph style is associated with a certain layout. That layout is
 * partly defined in the style itself, and partly because it inherits layout
 * from its parent style. This method gathers all that information,
 * translates it to Psiconv format and puts it in para_layout and
 * char_layout.
 * Note that we call ourselves recursively; if we ever encounter a
 * situation in which is style is based on itself, or is in some other circular
 * relationship, we will crash and burn. So don't do that :-)
 */
bool PL_Psion_Listener::_setStyleLayout(PD_Style *style,
                                        psiconv_paragraph_layout para_layout,
                                        psiconv_character_layout char_layout)
{
	PT_AttrPropIndex api;
	const PP_AttrProp * pAP = NULL;
	PD_Style *parent_style = NULL;

	// If we are based on another style, we need to get its layout first,
	// so that we can overrule it.
	parent_style = style->getBasedOn();
	if (parent_style)
		_setStyleLayout(parent_style,para_layout,char_layout);
	// Set the paragraph-level and character-level layouts.
	if ((api = style->getIndexAP()) &&
		 m_pDocument->getAttrProp(api,&pAP) && pAP) {
		if(!updateParagraphLayout(pAP,para_layout))
			return false;
		if(!updateCharacterLayout(pAP,char_layout))
			return false;
	}
	return true;
}


/*!
 * Translate all styles.
 *
 */
bool PL_Psion_Listener::_processStyles(void)
{
	UT_GenericVector<PD_Style *> vecStyles;
	UT_sint32 i = 0;
	PD_Style * pStyle=NULL;
	psiconv_word_style style;

	// Allocate a temporary psiconv_word_style structure.	
	if (!(style = (psiconv_word_style) malloc(sizeof(*style))))
		goto ERROR1;
	
	// Initialize the styles section private variable.
	if (!(m_styles = (psiconv_word_styles_section) malloc(sizeof(*m_styles))))
		goto ERROR1;
	if (!(m_styles->styles = psiconv_list_new(sizeof(*style))))
		goto ERROR1;
	
	m_styles->normal = NULL;
	
	// Examine all used styles one by one (so we do not export unused styles.
	// You could discuss whether this is a good idea. I think so. I do not
	// want tens of unused styles in my Psion documents. Do you?
	m_pDocument->getAllUsedStyles(&vecStyles);
	for (i=0; i < vecStyles.getItemCount(); i++) {
		pStyle = vecStyles.getNthItem(i);
		
		// Psion does not understand character styles, so we ignore them.
		if (pStyle->isCharStyle())
			continue;
		
		UT_DEBUGMSG(("PSION: Processing style %s\n",pStyle->getName()));

		// Initialize the style with sound defaults.
		if (!(style->character = psiconv_basic_character_layout()))
			goto ERROR2;
		if (!(style->paragraph = psiconv_basic_paragraph_layout()))
			goto ERROR3;
		style->hotkey = 0;
		style->built_in = psiconv_bool_false;
		style->outline_level = 0;
		
		// Set the style name
		if (!(style->name = utf8_to_ucs2(pStyle->getName())))
			goto ERROR4;
		
		// Set the style layout
		_setStyleLayout(pStyle,style->paragraph,style->character);		

		// Add the style to the styles section.
		// We handle the Normal style in a special way
		if (!strcmp(pStyle->getName(),"Normal")) {
			m_styles->normal = style;
			if (!(style = (psiconv_word_style) malloc(sizeof(*style))))
				goto ERROR1;
		} else {
			if (psiconv_list_add(m_styles->styles,style))
				goto ERROR5;
		}
	}
	
	// Just in case we somehow lost the Normal style (unlikely, but well...)
	if (!m_styles->normal) {
		// TODO: Perhaps we need better defaults here?
		UT_DEBUGMSG(("PSION: Creating our own Normal style %s\n",pStyle->getName()));
		if (!(style->character = psiconv_basic_character_layout()))
			goto ERROR2;
		if (!(style->paragraph = psiconv_basic_paragraph_layout()))
			goto ERROR3;
		style->hotkey = 0;
		style->built_in = psiconv_bool_false;
		style->outline_level = 0;
		if (!(style->name = utf8_to_ucs2("Normal")))
			goto ERROR4;
		m_styles->normal = style;
	} else		
		free(style);
	
	return true;

ERROR5:
	free(style->name);
ERROR4:
	psiconv_free_paragraph_layout(style->paragraph);
ERROR3:
	psiconv_free_character_layout(style->character);
ERROR2:
	free(style);
ERROR1:
	return false;
}

/*!
 * Handle images
 *
 * Images can be in several flavours in the AbiWord source. Here, we just
 * handle PNG ones.
 */
bool PL_Psion_Listener::_insertImage(const PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	const gchar *szValue;
	_bb image_data;
    std::string mimeType;
	const psiconv_ucs2 object_marker = 0x0e;
	png_structp png_ptr;
	png_infop info_ptr;
	int width,height,row,column,resx,resy;
	psiconv_paint_data_section paint_data;
	psiconv_sketch_section sketch_sec;
	psiconv_embedded_object_section object;
	psiconv_sketch_f sketch_file;
	struct psiconv_in_line_layout_s in_line;
	png_bytep *png_data_rows;

	// Get the attribute array
	if (!api || !m_pDocument->getAttrProp(api,&pAP) || !pAP)
		goto ERROR2;
	
	// Find the id of the image
	if (!pAP->getAttribute("dataid", szValue))
		goto ERROR2;
	
	// Retrieve the image
	if (!m_pDocument->getDataItemDataByName(szValue, image_data.pBB,
                                                &mimeType, NULL))
		goto ERROR2;
	image_data.iCurPos = 0;
	
	// At this moment, we only handle PNG images. Too bad.
	if (mimeType == "image/png") {
		UT_DEBUGMSG(("PSION: Unknown image MIME type (%s)\n", mimeType.c_str()));
		goto ERROR2;
	}
		
	// Prepare the PNG structure for reading
	png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, 
	                                               NULL, NULL, NULL);
    if (!png_ptr)
       goto ERROR2;
	
	// Prepare the PNG structure for info
	info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
	    png_destroy_read_struct(&png_ptr,NULL,NULL);
		goto ERROR2;
	}
	
	// Prepare PNG error handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		UT_DEBUGMSG(("PSION: PNG Error handler\n"));
		goto ERROR3;
    }
	
	// Use our own functions for reading the PNG data stream
	png_set_read_fn(png_ptr,(void *) &image_data,read_png_data);

	// Read the image into memory into 8-bit RGB
	png_read_png(png_ptr,info_ptr,PNG_TRANSFORM_STRIP_16 | 
	                              PNG_TRANSFORM_STRIP_ALPHA |
	                              PNG_TRANSFORM_PACKING |
	                              PNG_TRANSFORM_EXPAND,NULL);
	png_data_rows = png_get_rows(png_ptr,info_ptr);
	width = png_get_image_width(png_ptr,info_ptr);
	height = png_get_image_height(png_ptr,info_ptr);
	// If pixel ration unknown, use 72 DPI
	resx = png_get_x_pixels_per_meter(png_ptr,info_ptr);
	if (resx <= 0)
		resx = 72*40;
	resy = png_get_y_pixels_per_meter(png_ptr,info_ptr);
	if (resy <= 0)
		resy = 72*40;
	UT_DEBUGMSG(("PSION: Width %d, height %d, bitdepth %d, colortype %d, rowbytes: %lu\n", 
	             width,height,png_get_bit_depth(png_ptr,info_ptr),
	             png_get_color_type(png_ptr,info_ptr),
	             png_get_rowbytes(png_ptr,info_ptr)));
	
	// Allocate and fill all Psiconv structures. Quite a lot of work.
	if (!(paint_data = (psiconv_paint_data_section) malloc(sizeof(*paint_data))))
        goto ERROR3;
	paint_data->xsize = width;
	paint_data->ysize = height;
	paint_data->pic_xsize = paint_data->pic_ysize = 0;
	if (!(paint_data->red = (float *) malloc(sizeof(float) * width * height)))
		goto ERROR4;
	if (!(paint_data->green = (float *) malloc(sizeof(float) * width * height)))
		goto ERROR5;
	if (!((paint_data->blue = (float *) malloc(sizeof(float) * width * height))))
		goto ERROR6;
	for (row = 0; row < height; row++) {
		for (column = 0; column < width; column++) {
			paint_data->red[row*width+column]  = png_data_rows[row][column*3] / 255.0;
			paint_data->green[row*width+column] = png_data_rows[row][column*3+1] / 255.0;
			paint_data->blue[row*width+column] = png_data_rows[row][column*3+2] / 255.0;
			UT_DEBUGMSG(("PSION: Pixeldata in: %02x %02x %02x, Pixels: %f %f %f\n",
			            png_data_rows[row][column*3],
			            png_data_rows[row][column*3+1],
			            png_data_rows[row][column*3+2],
						paint_data->red[row*width+column],
						paint_data->green[row*width+column],
						paint_data->blue[row*width+column]));

		}
	}
	if (!(sketch_sec = (psiconv_sketch_section) malloc(sizeof(*sketch_sec))))
		goto ERROR7;
	sketch_sec->displayed_xsize = sketch_sec->form_xsize = width;
	sketch_sec->displayed_ysize = sketch_sec->form_ysize = height;
	sketch_sec->picture_data_x_offset = sketch_sec->picture_data_y_offset = 0;
	sketch_sec->displayed_size_x_offset = sketch_sec->displayed_size_y_offset = 0;
	sketch_sec->magnification_x = sketch_sec->magnification_y = 1.0;
	sketch_sec->cut_left = sketch_sec->cut_right = sketch_sec->cut_top = sketch_sec->cut_bottom = 0.0;
	sketch_sec->picture = paint_data;
	if (!(sketch_file = (psiconv_sketch_f) malloc(sizeof(*sketch_file))))
		goto ERROR8;
	sketch_file->sketch_sec = sketch_sec;
	if (!(object = (psiconv_embedded_object_section) malloc(sizeof(*object))))
		goto ERROR9;
	if (!(object->icon = (psiconv_object_icon_section) malloc(sizeof(*object->icon))))
		goto ERROR10;
	// Icon values are more or less random
	object->icon->icon_width = object->icon->icon_height = 0.5;
	if (!(object->icon->icon_name = utf8_to_ucs2("AbiWord Image")))
		goto ERROR11;
	if (!(object->display = (psiconv_object_display_section) malloc(sizeof(*object->display))))
		goto ERROR12;
	object->display->show_icon = psiconv_bool_false;
	object->display->width = width * 100 / resx;
	object->display->height = height * 100 / resy;
	if (!(object->object = (psiconv_file) malloc(sizeof(*object->object))))
		goto ERROR13;
	object->object->type = psiconv_sketch_file;
	object->object->file = sketch_file;
	if (!(in_line.layout = psiconv_clone_character_layout(m_currentParagraphCLayout)))
		goto ERROR14;
	in_line.length = 1;
	in_line.object = object;
	in_line.object_width = width * 100 / resx;
	in_line.object_height = height * 100 / resy;
	if (psiconv_list_add(m_currentParagraphInLines,&in_line))
		goto ERROR15;
	// Objects are represented by an object marker in the text.
	if (psiconv_list_add(m_currentParagraphText,&object_marker))
		goto ERROR3;
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	return true;
	
ERROR15:
	psiconv_free_character_layout(in_line.layout);
ERROR14:
	free(object->object);
ERROR13:
	free(object->display);
ERROR12:
	free(object->icon->icon_name);
ERROR11:
	free(object->icon);
ERROR10:
	free(object);
ERROR9:
	free(sketch_file);
ERROR8:
	free(sketch_sec);
ERROR7:
	free(paint_data->blue);
ERROR6:
	free(paint_data->green);
ERROR5:
	free(paint_data->red);
ERROR4:
	free(paint_data);
ERROR3:
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
ERROR2:
	UT_DEBUGMSG(("PSION: Parsing of image object failed\n"));
	return false;
}


/*******************************
 * class PL_Psion_Word_Listener *
 *******************************/

/*!
 * Create a Psion Word file
 */
psiconv_file PL_Psion_Word_Listener::createPsionFile(void)
{
	psiconv_file psionfile;
	psiconv_word_f wordfile;

	// We base our file on an empty file.
	if (!(psionfile = psiconv_empty_file(psiconv_word_file))) 
		return NULL;
	wordfile = (psiconv_word_f)(psionfile->file);
	
	// Free the old paragraphs structure and put ours into it. 
	psiconv_free_text_and_layout(wordfile->paragraphs);
	wordfile->paragraphs = m_paragraphs;
	m_paragraphs = NULL;
	
	// Free the old styles list and put ours into it
	psiconv_free_word_styles_section(wordfile->styles_sec);
	wordfile->styles_sec = m_styles;
	m_styles = NULL;
	
	// Free the old header and footer and put ours into it
	psiconv_free_page_header(wordfile->page_sec->header);
	wordfile->page_sec->header = m_header;
	m_header = NULL;
	psiconv_free_page_header(wordfile->page_sec->footer);
	wordfile->page_sec->footer = m_footer;
	m_footer = NULL;
	
	// That's it!
	return psionfile;
}



/*********************************
 * class PL_Psion_TextEd_Listener *
 *********************************/

/*!
 * Create a Psion TextEd file
 */
psiconv_file PL_Psion_TextEd_Listener::createPsionFile(void)
{
	psiconv_file psionfile;
	psiconv_texted_f textedfile;

	// We base our file on an empty file.
	if (!(psionfile = psiconv_empty_file(psiconv_texted_file)))  
		return NULL;
	textedfile = (psiconv_texted_f)(psionfile->file);
	
	// Free the old paragraphs structure and put ours into it.
	psiconv_free_text_and_layout(textedfile->texted_sec->paragraphs);
	textedfile->texted_sec->paragraphs = m_paragraphs;
	m_paragraphs = NULL;

	// Free the old header and footer and put ours into it
	psiconv_free_page_header(textedfile->page_sec->header);
	textedfile->page_sec->header = m_header;
	psiconv_free_page_header(textedfile->page_sec->footer);
	textedfile->page_sec->footer = m_footer;
	
	// That's it!
	return psionfile;
}

/***************************
 * class IE_Exp_Psion_Word *
 ***************************/

/*!
 * Create a Psion Word listener.
 */
PL_Psion_Listener *IE_Exp_Psion_Word::_constructListener(void)
{
	return new PL_Psion_Word_Listener(getDoc());
}



/*****************************
 * class IE_Exp_Psion_TextEd *
 *****************************/

/*!
 * Create a Psion TextEd listener.
 */
PL_Psion_Listener *IE_Exp_Psion_TextEd::_constructListener(void)
{
	return new PL_Psion_TextEd_Listener(getDoc());
}
