/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2000 Frodo Looijaard <frodol@dds.nl>
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

/* This importer was written by Frodo Looijaard <frodol@dds.nl> */

//  Import Word or TextEd data from a Psion file. 
//  We use libpsiconv for the real work

// To do:
//	Fix characters with high ASCII codes

// To do once Abiword supports it:
//   Styles: enable
//   Page header and footer, page size, first page number
//   Paragraph borders and background color

// Note that the current bullets implementation is incompatible with
// styles: for a bullet to be displayed, we *have to set the paragraph
// style to `Bullet List'. The current bullet implementation is one big
// nasty hack :-(

#include <stdio.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_growbuf.h"
#include "ut_mbtowc.h"
#include "ut_units.h"
#include "ut_bytebuf.h"

#include "pd_Document.h"
#include "xap_EncodingManager.h"

#include "ie_imp_Psion.h"
#include <psiconv/parse.h>

// The style combo box does not support new styles yet... 
#undef ENABLE_STYLES


/*****************************************************************/
/*****************************************************************/

UT_Error IE_Imp_Psion::importFile(const char * szFilename)
{
	FILE *fp = fopen(szFilename, "r");
	psiconv_buffer buf;
	psiconv_file psionfile;
	int res;

	if (!fp)
	{
		UT_DEBUGMSG(("Could not open file %s\n",szFilename));
		return UT_IE_FILENOTFOUND;
	}

	if (!(buf = psiconv_buffer_new())) {
		fclose(fp);
		return(UT_IE_NOMEMORY);
	}

	if (psiconv_buffer_fread_all(buf,fp)) {
		psiconv_buffer_free(buf);
		fclose(fp);
		return UT_IE_NOMEMORY;
	}

	fclose(fp);

	res = psiconv_parse(buf,&psionfile);
	psiconv_buffer_free(buf);

	if (res) {
		if (res == PSICONV_E_NOMEM)
			return UT_IE_NOMEMORY;
		else
			return UT_IE_BOGUSDOCUMENT;
	}

	return parseFile(psionfile);
}

/*****************************************************************/
/*****************************************************************/

// Callback function to emit psiconv error and debug messages.
static void psion_error_handler (int kind, psiconv_u32 off, const char *message)
{
	// We need to output messages even if not in debug mode...
	_UT_OutputMessage("%s\n",message);
}

// Destructor. Note that we do not reset the psiconv error handler or the
// verbosity: they are global for the whole program. Fortunately, we do
// not want to change them.
IE_Imp_Psion::~IE_Imp_Psion()
{
}

// Constructor. We set the psiconv verbosity and error handlers here;
// strictly taken, this needs only to be done once in the whole program,
// but this is so much easier.
IE_Imp_Psion::IE_Imp_Psion(PD_Document * pDocument)
	: IE_Imp(pDocument)
{
	// Ouch. This is ugly. We simply never reset it.
	psiconv_error_handler = &psion_error_handler;
#ifdef UT_DEBUG
	psiconv_verbosity=PSICONV_VERB_DEBUG;
#else
	psiconv_verbosity=PSICONV_VERB_WARN;
#endif
	listid = NULL;
}

/*****************************************************************/
/*****************************************************************/

// Output all styles.
bool IE_Imp_Psion::applyStyles(psiconv_word_styles_section style_sec)
{
	// UT_Byte is `unsigned char', so we need some nasty casts :-(
	class UT_ByteBuf props(256);

	int i;
	const XML_Char *stylename;
	psiconv_word_style style;

	for (i = -1; i < (int) psiconv_list_length(style_sec->styles); i++) {
		if (i == -1)
			style = style_sec->normal;
		else if (!(style = (psiconv_word_style) 
		                    psiconv_list_get(style_sec->styles,i)))
			return false;
		// UT_DEBUGMSG(("Importing style %s\n",style->name));
		props.truncate(0);
		if (!getParagraphAttributes(style->paragraph,&props))
			return false;
		if (!getCharacterAttributes(style->character,&props))
			return false;
		// Not yet implemented: hotkey
		// Not yet implemented: built_in 
		// Not yet implemented: outline_level
		
		// Append the string termination character '\000'
		if (!(props.append((unsigned char *) "",1)))
			return false;

		if (i == -1)
			stylename = (const XML_Char *) "Normal";
		else
			stylename = (const XML_Char *) style->name;

		// UT_DEBUGMSG(("Style attributes: %s\n",props.getPointer(0)));

		const XML_Char* propsArray[7];
		propsArray[0] = (const XML_Char *) "props";
		propsArray[1] = (const XML_Char *) props.getPointer(0);
		propsArray[2] = (const XML_Char *) "name";
		propsArray[3] = stylename;
		// All Psion styles are based upon the Normal style
		propsArray[4] = (const XML_Char *) "basedon";
		propsArray[5] = (const XML_Char *) "Normal";
		propsArray[6] = (const XML_Char *) NULL;

		if (!( m_pDocument->appendStyle(propsArray))) {
			UT_DEBUGMSG(("AppendStyle failed...\n"));
			return false;
		}
	}
	return true;
}

// Set all page (section) attributes, and do an appendStrux(PTX_Section,...)
// These settings are global for the whole document: Psion documents
// contain only one single section.
bool IE_Imp_Psion::applyPageAttributes(psiconv_page_layout_section layout)
{
	// UT_Byte is `unsigned char', so we need some nasty casts :-(
	class UT_ByteBuf props(256);
	// This is only used for fixed string expansions, and should be big
	// enough in all circumstances.
	char buffer[64]; 

	// first page number: not yet implemented

	// left margin
	sprintf(buffer,"page-margin-left:%6.3fcm",layout->left_margin);
	if (!(props.append((unsigned char *) buffer,strlen(buffer))))
		return false;
	
	// right margin
	sprintf(buffer,"; page-margin-right:%6.3fcm",layout->right_margin);
	if (!(props.append((unsigned char *) buffer,strlen(buffer))))
		return false;

	// top margin
	sprintf(buffer,"; page-margin-top:%6.3fcm",layout->top_margin);
	if (!(props.append((unsigned char *) buffer,strlen(buffer))))
		return false;

	// bottom margin
	sprintf(buffer,"; page-margin-bottom:%6.3fcm",layout->bottom_margin);
	if (!(props.append((unsigned char *) buffer,strlen(buffer))))
		return false;

#if 0
	// page width: not yet implemented; what would its name be?!?
	sprintf(buffer,"; page-width:%6.3fcm",layout->page_width);
	props.append(buffer,strlen(buffer));
#endif
#if 0
	// page height: not yet implemented; what would its name be?!?
	sprintf(buffer,"; page-height:%6.3fcm",layout->page_height);
	props.append(buffer,strlen(buffer));
#endif
	// Header and footer: not yet implemented (complex!)

	// Append the string termination character '\000'
	if (!(props.append((unsigned char *) "",1)))
		return false;

	// UT_DEBUGMSG(("Page: %s\n",props.getPointer(0)));
	const XML_Char* propsArray[3];
	propsArray[0] = (const XML_Char *) "props";
	propsArray[1] = (const XML_Char *) props.getPointer(0);
	propsArray[2] = (const XML_Char *) NULL;

	return  m_pDocument->appendStrux(PTX_Section,propsArray);
}


// Get all paragraph-related attributes and append them to props.
// Note that you have to append a string termination character yourself!
// props is allocated on the heap if it is NULL.
// If props is not empty, we start with '; ', else we do not.
bool IE_Imp_Psion::getParagraphAttributes(psiconv_paragraph_layout layout,
                                             UT_ByteBuf *props)
{
	// This is only used for fixed string expansions, and should be big
	// enough in all circumstances.
	char buffer[64]; 

	int i;
	psiconv_tab tab;
	bool props_allocated = false;

	if (!props) {
		props = new UT_ByteBuf(256);
		props_allocated = true;
	}

	// If this is a bulleted paragraph with indent, we need to make sure
	// the indent_first is positive. Stupid Psion.
	if (layout->bullet && layout->bullet->on && layout->bullet->indent &&
	    (layout->indent_first > 0)) {
		layout->indent_left += layout->indent_first;
		layout->indent_first = -layout->indent_first;
	}

	if (props->getLength())
		if (!(props->append((unsigned char *) "; ",2)))
			goto ERROR;

	// Left indent
	sprintf(buffer,"margin-left:%6.3fcm",layout->indent_left);
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// Right indent
	sprintf(buffer,"; margin-right:%6.3fcm",layout->indent_right);
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// First line indent
	sprintf(buffer,"; text-indent:%6.3fcm",layout->indent_first);
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// Horizontal justify
	sprintf(buffer,"; text-align:%s",
	                   layout->justify_hor==psiconv_justify_left  ? "left" : 
	                   layout->justify_hor==psiconv_justify_right ? "right":
	                   layout->justify_hor==psiconv_justify_centre? "center":
	                                                                "justify");
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// Vertical justify: ignored (never used in Word documents)
	
#if 0
	// background color: not yet implemented; what would its name be?!?
	sprintf(buffer, "; backcolor: %02x%02x%02x",
	                                      layout->back_color->red,
	                                      layout->back_color->green,
	                                      layout->back_color->blue);
	props->append((unsigned char *) buffer,strlen(buffer));
#endif

	// Linespacing
	sprintf(buffer, "; line-height: %dpt",(int) layout->linespacing);
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;
	if (! layout->linespacing_exact)
		if (!(props->append((unsigned char *) "+",1)))
			goto ERROR;

	// Space above
	sprintf(buffer,"; margin-top:%dpt",(int) layout->space_above);
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// Space below
	sprintf(buffer,"; margin-bottom:%dpt",(int) layout->space_below);
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// Keep together
	sprintf(buffer,"; keep-together:%s",layout->keep_together?"yes":"no");
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// Keep with next
	sprintf(buffer,"; keep-with-next:%s",layout->keep_with_next?"yes":"no");
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// On next page
	// This is not yet implemented in AbiWord. We use a hack in 
	// applyParagraphAttributes; styles are out of luck in this.

	// Widow control
	// I'm not quite sure about the difference between setting widows and
	// orphans?!?
	
	sprintf(buffer,"; widows:%d; orphans:%d",
	        layout->no_widow_protection?0:2,
	        layout->no_widow_protection?0:2);
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// Default tab interval.
	sprintf(buffer,"; default-tab-interval:%6.3fcm",layout->tabs->normal);
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// Other tabs
	if (psiconv_list_length(layout->tabs->extras)) {
		strcat(buffer,"; tabstops:");
		if (!(props->append((unsigned char *) buffer,strlen(buffer))))
			goto ERROR;
		for (i = 0; i < (int) psiconv_list_length(layout->tabs->extras); i++) {
			if (!(tab = (psiconv_tab) psiconv_list_get(layout->tabs->extras,
			                                           i))) {
				UT_ASSERT(tab != NULL);
				return(false);
			}
			sprintf(buffer, "%s%6.3fcm/%c",	
					i==0?"":",",
					tab->location,
					tab->kind == psiconv_tab_centre?'C':
			        tab->kind == psiconv_tab_right? 'R':
			                                        'L');
			if (!(props->append((unsigned char *) buffer,strlen(buffer))))
				goto ERROR;
		}
	}

	// Bullets. I don't think there is a general way to do this yet.
	// For now, we will hardcode all bullets to style 'Bullet List',
	// because we might get into real trouble. Note that we hack
	// this together in applyParagraphAttributes. That means we can
	// not combine styles with bullets, and do other nifty things.
	
	// Not yet implemented: borders
	
	return true;

ERROR:
	if (props_allocated)
		delete props;
	return  false;
}

// Amazing. This actually works, even though it is stolen from the RTF importer
// and mutilated severely by me.
// It does an appendStrux setting the current paragraph attributes and opening
// a new paragraph.
bool IE_Imp_Psion::applyParagraphAttributes(psiconv_paragraph_layout layout,
                      const XML_Char *stylename)
{
	// UT_Byte is `unsigned char', so we need some nasty casts :-(
	class UT_ByteBuf props(256);
	const XML_Char* propsArray[11];

	// HACK: there is no real setting to do this.
	if (layout->on_next_page) {
		UT_UCSChar ucs = UCS_FF;
		if (!(m_pDocument->appendSpan(&ucs,1)))
			return false;
	}
	// Get all attributes into prop
	if (!(getParagraphAttributes(layout,&props)))
		return false;

	// HACK: Handle bullets
	// This is really, really ugly. One day, when fields have stabilized,
	// we will do it better.
	if (layout->bullet->on) {
		// Hardcode the stylename; it is the only way at this moment...
		// This means that we throw away the real style if styles are enabled...
		stylename = (const XML_Char *) "Bullet List";
		// We need to generate the list once, but only if we actually
		// have a bullet somewhere. Nasty. The attributes are mostly
		// black magickish...
		if (!listid) {
			listid = (const XML_Char *) "666";
			propsArray[0] = (const XML_Char *) "id";
			propsArray[1] = listid;
			propsArray[2] = (const XML_Char *) "parentid";
			propsArray[3] = (const XML_Char *) "0";
			propsArray[4] = (const XML_Char *) "type";
			propsArray[5] = (const XML_Char *) "5";
			propsArray[6] = (const XML_Char *) "start-value";
			propsArray[7] = (const XML_Char *) "0";
			propsArray[8] = (const XML_Char *) "list-delim";
			propsArray[9] = (const XML_Char *) "%L";
			propsArray[10] =(const XML_Char *)  NULL;
			m_pDocument->appendList(propsArray);
		}
	}

	// Append the string termination character '\000'
	props.append((unsigned char *) "",1);

	// UT_DEBUGMSG(("Paragraph: %s\n",props.getPointer(0)));
	propsArray[0] = (const XML_Char *) "props";
	propsArray[1] = (const XML_Char *) props.getPointer(0);
	propsArray[2] = (const XML_Char *) "style";
	propsArray[3] = stylename;
	propsArray[4] = (const XML_Char *) NULL;

	if (layout->bullet->on) {
		propsArray[4] = (const XML_Char *) "listid";
		propsArray[5] = listid;
		propsArray[6] = (const XML_Char *) NULL;
	}

	if (!(m_pDocument->appendStrux(PTX_Block,propsArray)))
		return false;
	
	// We need to append a field and some other stuff...
	if (layout->bullet->on) {
		propsArray[0] = (const XML_Char *) "type";
		propsArray[1] = (const XML_Char *) "list_label";
		propsArray[2] = (const XML_Char *) NULL;
		if (!(m_pDocument->appendObject(PTO_Field,propsArray)))
			return false;

		// If this is a bullet-with-indent, we need a tab to get the
		// text alligned to the selected left margin.
		if (layout->bullet->indent) {
			UT_UCSChar uc = (UT_UCSChar) UCS_TAB;
			if (!(m_pDocument->appendSpan(&uc,1)))
				return false;
		}
	}
	return true;
}

// Get all character-related attributes and append them to props.
// Note that you have to append a string termination character yourself!
// props is allocated on the heap if it is NULL.
// If props is not empty, we start with '; ', else we do not.
bool IE_Imp_Psion::getCharacterAttributes(psiconv_character_layout layout,
                                             UT_ByteBuf *props)
{
	// This is only used for fixed string expansions, and should be big
	// enough in all circumstances.
	char buffer[64]; 
	int fontsize;

	bool props_allocated = false;

	if (!props) {
		props = new UT_ByteBuf(256);
		props_allocated = true;
	}

	if (props->getLength())
		if (!(props->append((unsigned char *) "; ",2)))
			goto ERROR;

	// font family
	// BUG: No checking is done yet whether this family is known to AbiWord
	// and no sanitizing of the font name is done. Theoretically, this
	// could bomb Abiword if you hand-edited a Psion file and have the
	// font-family name contain really weird stuff.
	strcpy(buffer,"font-family:");
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;
	// We can't sprintf this to buffer, because it might be long.
	if (!(props->append((unsigned char *) layout->font->name,
	                    strlen(layout->font->name))))
		goto ERROR;

	// font size. 
	// This should be moved to some general-purpose function.
	// At the moment, only the following font-sizes seem to be supported
	// by the GUI: 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28,
	// 36, 48 and 72. Others give GTK errors. This should be changed I think.
	fontsize = (int) layout->font_size;
	if (fontsize < 8)
		fontsize = 8;
	if ((fontsize % 2) && (fontsize > 11))
		fontsize -=1;
	if (fontsize > 28) {
		if (fontsize < 32)
			fontsize = 28;
		else if (fontsize < 42)
			fontsize = 36;
		else if (fontsize < 60)
			fontsize = 48;
		else
			fontsize = 72;
	}
	sprintf(buffer,"; font-size:%dpt",fontsize);
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// bold
	sprintf(buffer, "; font-weight:%s", layout->bold ? "bold" : "normal");
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// italic
	sprintf(buffer, "; font-style:%s",layout->italic ? "italic" : "normal");
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// underline & overline & strike-out
	sprintf(buffer, "; text-decoration:%s",
	        layout->underline && layout->strikethrough?"underline line-through":
	        layout->underline && !layout->strikethrough?"underline":
	        !layout->underline && layout->strikethrough?"line-through":
		                                                "none");
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// superscript and subscript
	sprintf(buffer, "; text-position:%s",
	       layout->super_sub == psiconv_superscript?"superscript":
	       layout->super_sub == psiconv_subscript  ?"subscript":
		                                            "normal");
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// color
	sprintf(buffer, "; color:%02x%02x%02x", (layout->color->red),
	                                        (layout->color->green),
	                                        (layout->color->blue));
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	// background color
	sprintf(buffer, "; bgcolor:%02x%02x%02x", layout->back_color->red,
	                                          layout->back_color->green,
	                                          layout->back_color->blue);
	if (!(props->append((unsigned char *) buffer,strlen(buffer))))
		goto ERROR;

	return true;

ERROR:
	if (props_allocated)
		delete props;
	return  false;
}
	

// Amazing. This actually works, even though it is stolen from the RTF importer
// and mutilated severely by me.
// It does an appendFmt setting the current character attributes. The next
// appendSpan will use these settings.
bool IE_Imp_Psion::applyCharacterAttributes(psiconv_character_layout layout)
{
	// UT_Byte is `unsigned char', so we need some nasty casts :-(
	class UT_ByteBuf props(256);

	// Get all attributes into prop
	if (!(getCharacterAttributes(layout,&props)))
		return false;

	// Append the string termination character '\000'
	props.append((unsigned char *) "",1);

	// UT_DEBUGMSG(("Character: %s\n",props.getPointer(0)));

	const XML_Char* propsArray[3];
	propsArray[0] = (const XML_Char *) "props";
	propsArray[1] = (const XML_Char *) props.getPointer(0);
	propsArray[2] = NULL;

	return m_pDocument->appendFmt(propsArray);
}

// Read length character from input, translate them to the internal
// Abiword format, and append them to the gbBlock.
// You must insure the input has at least length characters!
bool IE_Imp_Psion::prepareCharacters(char *input, int length,
                                        UT_GrowBuf *gbBlock)
{
	class UT_Mbtowc mbtowc;
	UT_UCSChar uc;
	wchar_t wc;
	int i;

	mbtowc.setInCharset("CP1252");

	for (i = 0; i < length; i++) {
		// Note that we may actually encounter an '\000' here too. This
		// is a psiconv left-over: the line ending sign may have 
		// layout too, so the layout applies to typically one character
		// more than the paragraph length we see here.
		if (input[i] == '\006')      // New paragraph (should never happen)
			continue;
		else if (input[i] == '\007') // New line (is this right?)
			uc = UCS_LF; 
		else if (input[i] == '\010') // Hard page
			uc = UCS_FF; 
		else if (input[i] == '\011') // Tab
			uc = UCS_TAB; 
		else if (input[i] == '\012') // Unbreakable tab (not implemented?)
			uc = UCS_TAB; 
		else if (input[i] == '\013') // Unbreakable dash (is this right?)
			uc = UCS_EN_DASH; 
		else if (input[i] == '\014') // Potential hyphen (do we have it?)
			continue;
		else if (input[i] == '\015') // Unknown functionality
			continue;
		else if (input[i] == '\016') // Object placeholder 
		                             // Not yet implemented in psiconv 
			continue;
		else if (input[i] == '\017') // Visible space. Handle as normal space.
			uc = UCS_SPACE; 
		else if (input[i] == '\020') // Unbreakable space
			uc = UCS_NBSP; 
		else if (input[i] < 32) // Not implemented
			continue;
		else if (!mbtowc.mbtowc(wc,input[i]))
			continue;
		else
			 uc = (UT_UCSChar) wc;
		if (!(gbBlock->ins(gbBlock->getLength(),&uc,1)))
			return false;
	}
	return true;
}

UT_Error IE_Imp_Psion::readParagraphs(psiconv_text_and_layout psiontext,
                                      psiconv_word_styles_section style_sec)
{
	unsigned int i,inline_nr,loc;
	psiconv_paragraph paragraph;
	psiconv_in_line_layout in_line;
	UT_GrowBuf gbBlock;
#if ENABLE_STYLES
	psiconv_word_style style;
#endif
	const XML_Char *stylename;

	for (i=0; i < psiconv_list_length(psiontext); i++) {
		if (!(paragraph = (psiconv_paragraph) psiconv_list_get(psiontext,i))) {
			// Something is really wrong...
			UT_ASSERT(paragraph != NULL);
			return UT_ERROR;
		}

#if ENABLE_STYLES
		// Determine the style name
		if (!style_sec ||
		      !(style = psiconv_get_style(style_sec,paragraph->base_style)) ||
		  	  !(stylename = style->name))
#endif
			stylename = (const XML_Char *) "Normal";

		loc = 0;
		if (!(applyParagraphAttributes(paragraph->base_paragraph,stylename))) 
			return UT_IE_NOMEMORY;
		for(inline_nr=0; inline_nr < psiconv_list_length(paragraph->in_lines);
		    inline_nr++) {
			if (!(in_line = (psiconv_in_line_layout) psiconv_list_get(paragraph->in_lines,inline_nr))) {
				// Something is really wrong...
				UT_ASSERT(in_line != NULL);
				return UT_ERROR;
			}
			gbBlock.truncate(0);
			if (!(prepareCharacters(paragraph->text + loc,in_line->length,
			      &gbBlock))) 
				return UT_IE_NOMEMORY;
			// Yes, gbBlock may be empty!
			if (gbBlock.getLength()) {
				if (!( applyCharacterAttributes(in_line->layout))) 
					return UT_IE_NOMEMORY;
				if (!( m_pDocument->appendSpan(gbBlock.getPointer(0), 
		   		                  gbBlock.getLength())))
					return UT_IE_NOMEMORY;
			}
			loc += in_line->length;
		}
		if (loc < strlen(paragraph->text)) {
			gbBlock.truncate(0);
			if (!(prepareCharacters(paragraph->text+loc,
			                       strlen(paragraph->text - loc),&gbBlock))) 
				return UT_IE_NOMEMORY;
			// Yes, gbBlock may be empty!
			if (gbBlock.getLength()) {
				if (!(applyCharacterAttributes(paragraph->base_character))) 
					return UT_IE_NOMEMORY;
				if (!( m_pDocument->appendSpan(gbBlock.getPointer(0), 
		   		                  gbBlock.getLength())))
					return UT_IE_NOMEMORY;
			}
		}
	}
	return UT_OK;
}

void IE_Imp_Psion::pasteFromBuffer(PD_DocumentRange * pDocRange,
                                   unsigned char * pData, UT_uint32 lenData)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
}


/*****************************************************************/
/*****************************************************************/

IE_Imp_Psion_Word::~IE_Imp_Psion_Word()
{
}

IE_Imp_Psion_Word::IE_Imp_Psion_Word(PD_Document * pDocument)
	: IE_Imp_Psion(pDocument)
{
}

UT_Error IE_Imp_Psion_Word::parseFile(psiconv_file psionfile)
{
	if (psionfile->type != psiconv_word_file) 
		return UT_IE_BOGUSDOCUMENT;

#if ENABLE_STYLES
	if (!applyStyles(((psiconv_word_f) (psionfile->file))->styles_sec))
		return UT_IE_NOMEMORY;
#endif
	if (!applyPageAttributes(((psiconv_word_f) (psionfile->file))->page_sec))
		return UT_IE_NOMEMORY;
	return readParagraphs(((psiconv_word_f) (psionfile->file))->paragraphs,
#if ENABLE_STYLES
	                      ((psiconv_word_f) (psionfile->file))->styles_sec
#else
						  NULL 
#endif
);
}


bool IE_Imp_Psion_Word::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	
	UT_uint32 i;

	psiconv_buffer pl = psiconv_buffer_new();
	if (!pl) 
		return false;
	for (i=0; i < iNumbytes; i++)
		if ((psiconv_buffer_add(pl,szBuf[i]))) {
			psiconv_buffer_free(pl);
			return false;
		}
	psiconv_file_type_t filetype = psiconv_file_type(pl,NULL,NULL);
	psiconv_buffer_free(pl);
	if (filetype == psiconv_word_file)
		return true;
	else
		return false;
}

bool IE_Imp_Psion_Word::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".psiword") == 0);
}

UT_Error IE_Imp_Psion_Word::StaticConstructor(PD_Document * pDocument, IE_Imp ** ppie)
{
	IE_Imp_Psion_Word * p = new IE_Imp_Psion_Word(pDocument);
	*ppie = p;
	return UT_OK;
}

// We take the .psi suffix for now, but this will need to change to none at all
bool	IE_Imp_Psion_Word::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "Psion Word (.psiword)";
	*pszSuffixList = "*.psiword";
	*ft = IEFT_Psion_Word;
	return true;
}

bool IE_Imp_Psion_Word::SupportsFileType(IEFileType ft)
{
	return (IEFT_Psion_Word == ft);
}

/*****************************************************************/
/*****************************************************************/

IE_Imp_Psion_TextEd::~IE_Imp_Psion_TextEd()
{
}

IE_Imp_Psion_TextEd::IE_Imp_Psion_TextEd(PD_Document * pDocument)
	: IE_Imp_Psion(pDocument)
{
}

UT_Error IE_Imp_Psion_TextEd::parseFile(psiconv_file psionfile)
{
	if (psionfile->type != psiconv_texted_file) 
		return UT_IE_BOGUSDOCUMENT;

	if (!applyPageAttributes(((psiconv_texted_f) (psionfile->file))->page_sec))
		return UT_IE_NOMEMORY;
	return readParagraphs(((psiconv_texted_f) 
	                       (psionfile->file))->texted_sec->paragraphs,NULL);
}


bool IE_Imp_Psion_TextEd::RecognizeContents(const char * szBuf, UT_uint32 iNumbytes)
{
	
	UT_uint32 i;

	psiconv_buffer pl = psiconv_buffer_new();
	if (!pl) 
		return false;
	for (i=0; i < iNumbytes; i++)
		if ((psiconv_buffer_add(pl,szBuf[i]))) {
			psiconv_buffer_free(pl);
			return false;
		}
	psiconv_file_type_t filetype = psiconv_file_type(pl,NULL,NULL);
	psiconv_buffer_free(pl);
	if (filetype == psiconv_texted_file)
		return true;
	else
		return false;
}

bool IE_Imp_Psion_TextEd::RecognizeSuffix(const char * szSuffix)
{
	return (UT_stricmp(szSuffix,".psitext") == 0);
}

UT_Error IE_Imp_Psion_TextEd::StaticConstructor(PD_Document * pDocument, IE_Imp ** ppie)
{
	IE_Imp_Psion_TextEd * p = new IE_Imp_Psion_TextEd(pDocument);
	*ppie = p;
	return UT_OK;
}

// We take the .psi suffix for now, but this will need to change to none at all
bool	IE_Imp_Psion_TextEd::GetDlgLabels(const char ** pszDesc,
								  const char ** pszSuffixList,
								  IEFileType * ft)
{
	*pszDesc = "Psion TextEd (.psitext)";
	*pszSuffixList = "*.psitext";
	*ft = IEFT_Psion_TextEd;
	return true;
}

bool IE_Imp_Psion_TextEd::SupportsFileType(IEFileType ft)
{
	return (IEFT_Psion_TextEd == ft);
}

/*****************************************************************/
/*****************************************************************/


