/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2000,2004 Frodo Looijaard <frodol@dds.nl>
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

/* This importer is written by Frodo Looijaard <frodol@dds.nl> */

//  Import Word or TextEd data from a Psion file.
//  We use libpsiconv for the real work

#include "ie_imp_Psion.h"

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_std_string.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_string_class.h"

#include "pd_Document.h"
#include "xap_EncodingManager.h"
#include "png.h"

#include <psiconv/parse.h>


/***********************
 * Auxiliary functions *
 ***********************/

static const gchar *global_listid = "1000";

/*!
 * Translate the stylename in UCS2 to a sanitized UTF8 string.
 * 
 * The input is a psiconv UCS2 string; the output an Abiword UTF8 string.
 * Special characters are filtered away.
 * \return NULL if input is NULL or something went horribly wrong, the
 * UTF8 string otherwise.
 */
static gchar *prepare_style_name(const psiconv_string_t input)
{
	psiconv_string_t input_copy;
	gchar *result;
	int i;
	UT_uint32 read,written;
	
	if (!(input_copy = psiconv_unicode_strdup(input)))
		return NULL;
	for (i = 0; i < psiconv_unicode_strlen(input_copy);i++) 
		if ((input[i] < 0x20) || (input[i] == ';') || (input[i] == ':')) 
			input[i] = '?';
	read=written=0;
	result = UT_convert((char *)input,
					    psiconv_unicode_strlen(input) * sizeof(*input),
					    "UCS-2","UTF-8",&read,&written);
	free(input_copy);
	return result;
}

/*! Write data to the PNG stream
 *
 * This is a callback function for the PNG library. It is called when 
 * some data needs to writting to the PNG file. We have implemented it as
 * writing to a ByteBuf.
 */
static void write_png_data(png_structp png_ptr, png_bytep data, 
                           png_size_t length) 
{
	UT_ByteBuf* bb = (UT_ByteBuf*) (png_get_io_ptr(png_ptr));
    UT_DEBUGMSG(("PSION: write_png_data: %d bytes\n",length));
	bb->append(data,length);
}

/*! Flush the PNG stream 
 *
 * This is a callback function for the PNG library. It is called when
 * writing of the PNG file is finished.
 * Nothing needs to be done.
 */
static void write_png_flush(png_structp /*png_ptr*/)
{
}


/******************************
 * class IE_Imp_Psion_Sniffer *
 ******************************/

/*!
 * Check whether this is a Psion file of the given type
 *
 * This is used by the recognizeContents methods of the Word and TextEd
 * sniffers.
 * \return Psion files have strong magic, so we either return 
 * UT_CONFIDENCE_PERFECT or UT_CONFIDENCE_ZILCH.
 */
UT_Confidence_t IE_Imp_Psion_Sniffer::checkContents(const char * szBuf,
												UT_uint32 iNumbytes,
												psiconv_file_type_t filetype)
{
	UT_uint32 i;
	psiconv_config config;
	psiconv_buffer pl;
	psiconv_file_type_t filetype_detected;

	// Prepare a new psiconv_config object
	config = psiconv_config_default();
	if (!config)
		goto ERROR1;
	config->error_handler = &psion_error_handler;
	psiconv_config_read(NULL,&config);
	// It is likely detection will fail, so keep it silent.
	config->verbosity = PSICONV_VERB_FATAL;

	// Copy the file data into a psiconv_buffer.
	pl = psiconv_buffer_new();
	if (!pl)
		goto ERROR2;
	for (i=0; i < iNumbytes; i++)
		if ((psiconv_buffer_add(pl,szBuf[i]))) {
				goto ERROR3;
		}

	// Check whether this is a Psion file.
	filetype_detected = psiconv_file_type(config,pl,NULL,NULL);
	psiconv_buffer_free(pl);
	psiconv_config_free(config);
	if (filetype == filetype_detected)
		return UT_CONFIDENCE_PERFECT;
	else
		return UT_CONFIDENCE_ZILCH;

ERROR3:
	psiconv_buffer_free(pl);
ERROR2:
	psiconv_config_free(config);
ERROR1:
	return UT_CONFIDENCE_ZILCH;
}


/***********************************
 * class IE_Imp_Psion_Word_Sniffer *
 ***********************************/

// supported suffixes
/*!
 * Look at the extension to guess whether this is a Psion Word file.
 *
 * Actually, the Psion itself does not use extensions (much), so I just
 * made up my own convention (.psiword) here. It's better than nothing.
 */
static IE_SuffixConfidence IE_Imp_Psion_Word_Sniffer__SuffixConfidence[] = {
	{ "psiword", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 		UT_CONFIDENCE_ZILCH 		}
};

const IE_SuffixConfidence * IE_Imp_Psion_Word_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_Psion_Word_Sniffer__SuffixConfidence;
}

/*!
 * Check whether this is a Psion Word file
 */
UT_Confidence_t IE_Imp_Psion_Word_Sniffer::recognizeContents(const char * szBuf,
												  UT_uint32 iNumbytes)
{
	return checkContents(szBuf,iNumbytes,psiconv_word_file);
}

/*!
 * Construct a new IE_Imp_Psion_Word object
 */
UT_Error IE_Imp_Psion_Word_Sniffer::constructImporter(PD_Document * pDocument,
													  IE_Imp ** ppie)
{
	IE_Imp_Psion_Word * p = new IE_Imp_Psion_Word(pDocument);
	*ppie = p;
	return UT_OK;
}

/*!
 * Some import filter settings. We use the .psiword extension
 */
bool	IE_Imp_Psion_Word_Sniffer::getDlgLabels(const char ** pszDesc,
												const char ** pszSuffixList,
												IEFileType * ft)
{
	*pszDesc = "Psion Word (.psiword)";
	*pszSuffixList = "*.psiword";
	*ft = getFileType();
	return true;
}


/*************************************
 * class IE_Imp_Psion_TextEd_Sniffer *
 *************************************/


// supported suffixes
/*!
 * Look at the extension to guess whether this is a Psion TextEd file.
 * Actually, the Psion itself does not use extensions (much), so I just
 * made up my own convention (.psitext) here. It's better than nothing.
 */
static IE_SuffixConfidence IE_Imp_Psion_TextEd_Sniffer__SuffixConfidence[] = {
	{ "psitext", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 		}
};

const IE_SuffixConfidence * IE_Imp_Psion_TextEd_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_Psion_TextEd_Sniffer__SuffixConfidence;
}

/*!
 * Check whether this is a Psion TextEd file
 */
UT_Confidence_t IE_Imp_Psion_TextEd_Sniffer::recognizeContents(const char * szBuf,
													UT_uint32 iNumbytes)
{
	return checkContents(szBuf,iNumbytes,psiconv_texted_file);
}

/*!
 * Construct a new IE_Imp_Psion_TextEd object
 */
UT_Error IE_Imp_Psion_TextEd_Sniffer::constructImporter(PD_Document * pDocument,
                                                        IE_Imp ** ppie)
{
	IE_Imp_Psion_TextEd * p = new IE_Imp_Psion_TextEd(pDocument);
	*ppie = p;
	return UT_OK;
}

/*!
 * Some import filter settings. We use the .psitext extension
 */
bool IE_Imp_Psion_TextEd_Sniffer::getDlgLabels(const char ** pszDesc,
											   const char ** pszSuffixList,
											   IEFileType * ft)
{
	*pszDesc = "Psion TextEd (.psitext)";
	*pszSuffixList = "*.psitext";
	*ft = getFileType();
	return true;
}


/**********************
 * class IE_Imp_Psion *
 **********************/

/*!
 * Import a file with the given filename from disk.
 */
UT_Error IE_Imp_Psion::_loadFile(GsfInput * fp)
{
	int res;
	psiconv_file psionfile;
	UT_Error err = UT_IE_NOMEMORY;
	psiconv_buffer buf;
	psiconv_config config;

	// Read the file contents into a new psiconv_buffer
	if (!(buf = psiconv_buffer_new())) 
		goto ERROR1;

	psiconv_u8 ch;
	while(gsf_input_read(fp, 1, (guint8*)&ch)) {
		if(psiconv_buffer_add(buf, ch))
			goto ERROR3;
	}

	// Prepare a new psiconv_config object
	config = psiconv_config_default();
	if (!config)
		goto ERROR3;
	config->error_handler = psion_error_handler;
	psiconv_config_read(NULL,&config);

	// Try to parse the file contents into psiconv internal data structures
	res = psiconv_parse(config,buf,&psionfile);

	// Tidy up
	g_object_unref(G_OBJECT(fp));
	psiconv_config_free(config);
	psiconv_buffer_free(buf);

	// Check whether this was a parsable Psion document
	if (res) {
		if (res == PSICONV_E_NOMEM)
			return UT_IE_NOMEMORY;
		else
			return UT_IE_BOGUSDOCUMENT;
	}

	// Translate the file into an AbiWord document
	return parseFile(psionfile);

ERROR3:
	psiconv_buffer_free(buf);
ERROR1:
	return err;
}


/*!  
 * Append all styles from the Psion Word Styles Section.
 */
UT_Error IE_Imp_Psion::applyStyles(const psiconv_word_styles_section style_sec)
{
	UT_UTF8String props;

	int i;
	gchar *stylename;
	psiconv_word_style style;
	UT_Error res;

	// Iterate through all defined styles.
	// Index -1 is misused to represent the default "Normal" style.
	for (i = -1; i < (int) psiconv_list_length(style_sec->styles); i++) {

		if (i == -1)
			style = style_sec->normal;
		else if (!(style = (psiconv_word_style)
		                    psiconv_list_get(style_sec->styles,i)))
			return UT_IE_IMPORTERROR;

		// Get the style paragraph and character attributes.
		props.clear();
		if ((res = getParagraphAttributes(style->paragraph,props)))
			return res;

		if ((res = getCharacterAttributes(style->character,props)))
			return res;

		// Not yet implemented: hotkey
		// Not yet implemented: built_in
		// Not yet implemented: outline_level
		// The three unimplemented features above are not yet available
		// within AbiWord.

		// Get the style name.
		if (i == -1)
			stylename = (gchar *) strdup("Normal");
		else
			stylename = prepare_style_name(style->name);
		if (!stylename)
			return UT_IE_NOMEMORY;

		UT_DEBUGMSG(("PSION: Importing style %s\n",stylename));
		UT_DEBUGMSG(("PSION: Style attributes: %s\n",props.utf8_str()));

		const PP_PropertyVector propsArray = {
			"props", props.utf8_str(),
			"name", stylename,
			// All Psion styles are based upon the Normal style
			"basedon", "Normal"
		};

		if (!(getDoc()->appendStyle(propsArray))) {
			UT_DEBUGMSG(("PSION: AppendStyle failed...\n"));
			free(stylename);
			return UT_IE_IMPORTERROR;
		}
		free(stylename);
	}
	return UT_OK;
}

/*!
 * Add the page attributes to the documents
 *
 * Set all page (section) attributes, and do an appendStrux(PTX_Section,...)
 * These settings are global for the whole document: Psion documents
 * contain only one single section.
 */
UT_Error IE_Imp_Psion::applyPageAttributes(const psiconv_page_layout_section layout,
                                           bool &with_header, bool &with_footer)
{
	UT_return_val_if_fail(layout != NULL, true /* perhaps should be false, but we want loading to proceed */);

	UT_UTF8String props,buffer;

	// Determine whether we have a header and a footer. We can't append them
	// here, because they have to come after the main section (or AbiWord will
	// become very confused).
	with_header = layout->header && layout->header->text && 
		          layout->header->text->paragraphs &&
		          psiconv_list_length(layout->header->text->paragraphs);
	with_footer = layout->footer && layout->footer->text && 
		          layout->footer->text->paragraphs &&
		          psiconv_list_length(layout->footer->text->paragraphs);

	const PP_PropertyVector propsArray = {
		// Page width
		"width", UT_std_string_sprintf("%6.3f", layout->page_width),
		// Page height
		"height", UT_std_string_sprintf("%6.3f", layout->page_width),
		// Units of width/height
		"units", "cm",
		// Orientation
		"orientation", layout->landscape ? "landscape" : "portrait",
		// Page type (we should check for common ones here!)
		"pagetype", "Custom"
	};

	if (!(getDoc()->setPageSizeFromFile(propsArray)))
		return UT_IE_IMPORTERROR;

	// First page number not yet implemented
	// On first page not yet implemented
	
	// left margin
	UT_UTF8String_sprintf(buffer,"page-margin-left:%6.3fcm",layout->left_margin);
	props += buffer;

	// right margin
	UT_UTF8String_sprintf(buffer,"; page-margin-right:%6.3fcm",layout->right_margin);
	props += buffer;

	// top margin
	UT_UTF8String_sprintf(buffer,"; page-margin-top:%6.3fcm",layout->top_margin);
	props += buffer;

	// bottom margin
	UT_UTF8String_sprintf(buffer,"; page-margin-bottom:%6.3fcm",layout->bottom_margin);
	props += buffer;
	
	// header distance
	UT_UTF8String_sprintf(buffer,"; page-margin-header:%6.3fcm",layout->header_dist);
	props += buffer;
	
	// footer distance
	UT_UTF8String_sprintf(buffer,"; page-margin-footer:%6.3fcm",layout->footer_dist);
	props += buffer;
	
	// Now actually append the properties in a PTX_Section strux to the document
	UT_DEBUGMSG(("PSION: Page: %s\n",props.utf8_str()));
	PP_PropertyVector propsArray2 = {
		"props", props.utf8_str()
	};
	if (with_header) {
		propsArray2.push_back("header");
		propsArray2.push_back("1");
	}
	if (with_footer) {
		propsArray2.push_back("footer");
		propsArray2.push_back("2");
	}
	if (!(appendStrux(PTX_Section,propsArray2))) {
		return UT_IE_IMPORTERROR;
	}
	return UT_OK;
}

UT_Error IE_Imp_Psion::processHeaderFooter(const psiconv_page_layout_section layout,
                                           bool with_header, bool with_footer)
{
	UT_Error res;

	// Header
	if (with_header) {
		const PP_PropertyVector propsArray = {
			"id", "1",
			"type", "header"
		};
		if (!appendStrux(PTX_SectionHdrFtr,propsArray)) {
			return UT_IE_IMPORTERROR;
		}
		if ((res = readParagraphs(layout->header->text->paragraphs,NULL))) {
			return res;
		}
	}

	// Footer
	if (with_footer) {
		const PP_PropertyVector propsArray = {
			"id", "2",
			"type", "footer"
		};
		if (!appendStrux(PTX_SectionHdrFtr,propsArray)) {
			return UT_IE_IMPORTERROR;
		}
		if ((res = readParagraphs(layout->footer->text->paragraphs,NULL))) {
			return res;
		}
	}
	return res;
}

/*!  
 * Get all paragraph-related attributes and append them to props.
 *
 * If props is not empty, we start with '; ', else we do not.
 */
UT_Error IE_Imp_Psion::getParagraphAttributes(const psiconv_paragraph_layout layout,
                                          UT_UTF8String &props)
{
	UT_return_val_if_fail(layout != NULL, true /* perhaps should be false, but we want loading to proceed */);

	UT_UTF8String buffer;
	psiconv_length_t indent_left,indent_first;

	int i;
	psiconv_tab tab;
	
	// Compute the indent_left and indent_first settings. Note that 
	// indent_first is always relative to indent_left. There are a few
	// special cases related to bullets; see the psiconv docs for details.
	if (layout->bullet && layout->bullet->on && layout->bullet->indent &&
		(layout->indent_first > 0)) 
		indent_left = layout->indent_left + layout->indent_first;
	else
		indent_left = layout->indent_left;
	if (layout->bullet && layout->bullet->on && (layout->indent_first > 0))
		if (layout->bullet->indent)
			indent_first = -layout->indent_first;
		else
			indent_first = 0;
	else
		indent_first = layout->indent_first;
	
	// Append a semicolon if there is already text in the props
	if (props.length())
		props += ";";

	// Left indent
	UT_UTF8String_sprintf(buffer,"margin-left:%6.3fcm",indent_left);
	props += buffer;

	// Right indent
	UT_UTF8String_sprintf(buffer,"; margin-right:%6.3fcm",layout->indent_right);
	props += buffer;

	// First line indent
	UT_UTF8String_sprintf(buffer,"; text-indent:%6.3fcm",indent_first);
	props += buffer;
	
	// Horizontal justify
	UT_UTF8String_sprintf(buffer,"; text-align:%s",
	                   layout->justify_hor==psiconv_justify_left  ? "left" :
	                   layout->justify_hor==psiconv_justify_right ? "right":
	                   layout->justify_hor==psiconv_justify_centre? "center":
	                                                                "justify");
	props += buffer;
	
	// Vertical justify: ignored (never used in Word documents)

	// Background color
	UT_UTF8String_sprintf(buffer, "; bgcolor: %02x%02x%02x",
			  layout->back_color->red,
			  layout->back_color->green,
			  layout->back_color->blue);
	props += buffer;

#if 0
	// Linespacing (gives trouble at the moment, so we disable it)
	UT_UTF8String_sprintf(buffer, "; line-height: %dpt",(int) layout->linespacing);
	props += buffer;	
	if (! layout->linespacing_exact)
		props += "+";
#endif
		
	// Space above
	UT_UTF8String_sprintf(buffer,"; margin-top:%dpt",(int) layout->space_above);
	props += buffer;
	
	// Space below
	UT_UTF8String_sprintf(buffer,"; margin-bottom:%dpt",(int) layout->space_below);
	props += buffer;
	
	// Keep together
	UT_UTF8String_sprintf(buffer,"; keep-together:%s",layout->keep_together?"yes":"no");
	props += buffer;
	
	// Keep with next
	UT_UTF8String_sprintf(buffer,"; keep-with-next:%s",layout->keep_with_next?"yes":"no");
	props += buffer;
	
	// On next page
	// This is not yet implemented in AbiWord. We use a hack in
	// applyParagraphAttributes; styles are out of luck in this.
	// Last checked 20040229: Dialog is available, but no property yet.
	
	// Widow control
	// I'm not quite sure about the difference between setting widows and
	// orphans?!?
	UT_UTF8String_sprintf(buffer,"; widows:%d; orphans:%d",
	        layout->no_widow_protection?0:2,
	        layout->no_widow_protection?0:2);
	props += buffer;
	
	// Default tab interval.
	UT_UTF8String_sprintf(buffer,"; default-tab-interval:%6.3fcm",layout->tabs->normal);
	props += buffer;
	
	// Other tabs
	if (psiconv_list_length(layout->tabs->extras)) {
		props += "; tabstops:";
		for (i = 0; i < (int) psiconv_list_length(layout->tabs->extras); i++) {
			if (!(tab = (psiconv_tab) psiconv_list_get(layout->tabs->extras,
			                                           i))) {
				UT_ASSERT(tab != NULL);
				return(UT_IE_IMPORTERROR);
			}
			UT_UTF8String_sprintf(buffer, "%s%6.3fcm/%c",
					i==0?"":",",
					tab->location,
					tab->kind == psiconv_tab_centre?'C':
			        tab->kind == psiconv_tab_right? 'R':
			                                        'L');
			props += buffer;
		}
	}

	// Bullets. I don't think there is a general way to do this yet.
	// For now, we will hardcode all bullets to type 'Bullet List',
	// because we might get into real trouble. Note that we hack
	// this together in applyParagraphAttributes.
	
	// Not yet implemented: borders
	// These are not yet available in AbiWord.

	return UT_OK;
}

/*!  
 * Get all paragraph-related attributes and add them to the document.
 *
 * It does an appendStrux setting the current paragraph attributes and opening
 * a new paragraph. Several special cases are handled here too, mostly for
 * bullets.
 */
UT_Error IE_Imp_Psion::applyParagraphAttributes(const psiconv_paragraph_layout layout,
                      const gchar *stylename)
{
	UT_return_val_if_fail(layout != NULL, true /* perhaps should be false, but we want loading to proceed */);

	UT_UTF8String props;
	UT_Error res;

	// Get all attributes into prop
	if ((res = getParagraphAttributes(layout,props)))
		return UT_IE_IMPORTERROR;

	// HACK: Handle bullets
	// This is really, really ugly.
	// We can not really select the bullet symbol to use, so we do not even
	// try and just use always the plain round bullet.
	// Indent magic is done in getParagraphAttributes.
	if (layout->bullet->on) {
		props += ";list-style:Bullet List;field-font:Symbol";
		// We need to generate the list once, but only if we actually
		// have a bullet somewhere. Nasty. The attributes are mostly
		// black magickish...
		if (!list) {
			list = true;
			const gchar* propsArray[13];
			propsArray[0] = (const gchar *) "id";
			propsArray[1] = global_listid;
			propsArray[2] = (const gchar *) "parentid";
			propsArray[3] = (const gchar *) "0";
			propsArray[4] = (const gchar *) "type";
			propsArray[5] = (const gchar *) "5";
			propsArray[6] = (const gchar *) "start-value";
			propsArray[7] = (const gchar *) "0";
			propsArray[8] = (const gchar *) "list-delim";
			propsArray[9] = (const gchar *) "%L";
			propsArray[10] = (const gchar *) "list-decimal";
			propsArray[11] = (const gchar *) "NULL";
			propsArray[12] =(const gchar *)  NULL;
			getDoc()->appendList(propsArray);
		}
	}

	// Prepare the properties for this paragraph strux
	UT_DEBUGMSG(("PSION: Paragraph: %s\n",props.utf8_str()));
	PP_PropertyVector propsArray = {
		"props", props.utf8_str(),
		"style", stylename
	};

	// Bullets need the listid too.
	if (layout->bullet->on) {
		propsArray.push_back("listid");
		propsArray.push_back(global_listid);
	}

	if (!(appendStrux(PTX_Block, propsArray))) {
		return UT_IE_IMPORTERROR;
	}

	// HACK: there is no real setting to do this. Yet.
	if (layout->on_next_page) {
		UT_UCSChar ucs = UCS_FF;
		if (!(appendSpan(&ucs,1)))
			return UT_IE_IMPORTERROR;
	}

	// We need to append a field and some other stuff...
	if (layout->bullet->on) {
		propsArray.resize(2);
		propsArray.push_back("type");
		propsArray.push_back("list_label");

		if (!(appendObject(PTO_Field,propsArray)))
			return UT_IE_IMPORTERROR;

		// In some cases, but not in all, we need a tab after the bullet.
		// See the Psiconv docs for the (ugly) details.
		if ((!layout->bullet->indent && (layout->indent_first > 0)) || 
			layout->bullet->indent) {
			UT_UCSChar uc = (UT_UCSChar) UCS_TAB;
			if (!(appendSpan(&uc,1)))
				return UT_IE_IMPORTERROR;
		}
	}
	return UT_OK;
}

/*!  
 * Get all character-related attributes and append them to props.
 *
 * If props is not empty, we start with '; ', else we do not.
 */
UT_Error IE_Imp_Psion::getCharacterAttributes(const psiconv_character_layout layout,
                                             UT_UTF8String &props)
{
	UT_return_val_if_fail(layout != NULL, true /* perhaps should be false, but we want loading to proceed */);

	UT_UTF8String buffer;
	int fontsize;
	UT_UCS4Char ucs4char;
	int i;

	// Append a semicolon if there is already text in the props
	if (props.length())
		props += "; ";
		
	// font family
	// BUG: No checking is done yet whether this family is known to AbiWord.
	// We need to sanitize the font name first, or we could confuse the
	// properties parser.
	props += "font-family:";
	for (i = 0; i < psiconv_unicode_strlen(layout->font->name); i++) {
		ucs4char = layout->font->name[i];
		if ((ucs4char < 0x20) || (ucs4char == ';') || (ucs4char == ':'))
			ucs4char = '?';
		props.appendUCS4(&ucs4char,1);
	}
	
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
	UT_UTF8String_sprintf(buffer,"; font-size:%dpt",fontsize);
	props += buffer;
	
	// bold
	UT_UTF8String_sprintf(buffer, "; font-weight:%s", layout->bold ? "bold" : "normal");
	props += buffer;
	
	// italic
	UT_UTF8String_sprintf(buffer, "; font-style:%s",layout->italic ? "italic" : "normal");
	props += buffer;
	
	// underline and strike-through
	UT_UTF8String_sprintf(buffer, "; text-decoration:%s",
	        layout->underline && layout->strikethrough?"underline line-through":
	        layout->underline && !layout->strikethrough?"underline":
	        !layout->underline && layout->strikethrough?"line-through":
		                                                "none");
	props += buffer;
	
	// superscript and subscript
	UT_UTF8String_sprintf(buffer, "; text-position:%s",
	       layout->super_sub == psiconv_superscript?"superscript":
	       layout->super_sub == psiconv_subscript  ?"subscript":
		                                            "normal");
	props += buffer;
	
	// text color
	UT_UTF8String_sprintf(buffer, "; color:%02x%02x%02x", (layout->color->red),
	                                        (layout->color->green),
	                                        (layout->color->blue));
	props += buffer;
	
	// background color
	UT_UTF8String_sprintf(buffer, "; bgcolor:%02x%02x%02x", layout->back_color->red,
	                                          layout->back_color->green,
	                                          layout->back_color->blue);
	props += buffer;
	return UT_OK;
}


/*!  
 * Get all character-related attributes and add them to the document.
 *
 * It does an appendFmt setting the current character attributes. The next
 * appendSpan will use these settings.
 */
UT_Error IE_Imp_Psion::applyCharacterAttributes(const psiconv_character_layout layout)
{
	UT_return_val_if_fail(layout != NULL, true /* perhaps should be false, but we want loading to proceed */);
	UT_Error res;

	class UT_UTF8String props;

	// Get all attributes into prop
	if ((res = getCharacterAttributes(layout,props)))
		return res;

	UT_DEBUGMSG(("PSION: Character: %s\n",props.utf8_str()));

	// Propare the Fmt properties
	const PP_PropertyVector propsArray = {
		"props", props.utf8_str(),
	};

	if (!(appendFmt(propsArray)))
		return UT_IE_IMPORTERROR;
	return UT_OK;
}


/* Read characters from input and append them to text
 *
 * You must insure the input has at least length characters!
 * We handle special Psion markup tokens here. Except object markers,
 * they are handled in readParagraphs.
 */
UT_Error IE_Imp_Psion::prepareCharacters(const psiconv_ucs2 *input, int length,
                                        UT_UCS4String &text)
//                                        psiconv_list embobjlst)
{
	int i;
	UT_UCS4Char uc;

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
		else if (input[i] == '\017') // Visible space. Handle as normal space.
			uc = UCS_SPACE;
		else if (input[i] < 32) // Not implemented
			continue;
		else // More or less normal character
			uc = input[i];
		text += uc;
	}
	UT_DEBUGMSG(("PSION: text: %s\n",text.utf8_str()));
	return UT_OK;
}

/*!
 * Insert an image in the current document.
 *
 * The image is found in an in_line Psiconv element.
 */
UT_Error IE_Imp_Psion::insertImage(const psiconv_in_line_layout in_line)
{
	psiconv_sketch_f sketch_file;
	psiconv_paint_data_section paint_data;
	UT_ByteBuf image_buffer;
	png_byte *row;
	UT_UTF8String props,iname,buffer;
	int x,y,xsize,ysize;
	UT_uint32 iid;
	
	// Get the sketch file
	sketch_file = (psiconv_sketch_f) (in_line->object->object->file);
	paint_data = sketch_file->sketch_sec->picture;
	xsize = paint_data->xsize;
	ysize = paint_data->ysize;
	UT_DEBUGMSG(("PSION: Picture %d x %d\n",xsize,ysize));
	
	// Prepare the PNG structure for writing
	png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, 
	                                               NULL, NULL, NULL);
    if (!png_ptr)
       return UT_IE_IMPORTERROR;

	// Prepare the PNG structure for info
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
       png_destroy_write_struct(&png_ptr,NULL);
       return UT_IE_IMPORTERROR;
    }	

	// Prepare PNG error handling
	if (setjmp(png_jmpbuf(png_ptr)))
    {
		UT_DEBUGMSG(("PSION: PNG Error handler"));
     	png_destroy_write_struct(&png_ptr, &info_ptr);
     	return (UT_IE_IMPORTERROR);
    }
	
	// Use our own functions for writing the PNG data stream
	png_set_write_fn(png_ptr,(void *) &image_buffer,write_png_data,
	                 write_png_flush);
	
	// Set picture data
	png_set_IHDR(png_ptr,info_ptr,xsize,
	             ysize,8,PNG_COLOR_TYPE_RGB,
	             PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT,
	             PNG_FILTER_TYPE_DEFAULT);
	png_set_oFFs(png_ptr,info_ptr,
	             sketch_file->sketch_sec->picture_data_x_offset,
	             sketch_file->sketch_sec->picture_data_y_offset,
	             PNG_OFFSET_PIXEL);
	// Not yet implemented: magnification, cuts
	
	// Allocate one row of pixels
	if (!(row = (png_byte *) malloc(sizeof(png_byte) * xsize * 3))) {
		png_destroy_write_struct(&png_ptr,&info_ptr);
		return UT_IE_IMPORTERROR;
	}		
		
	// Writing the PNG file
	png_write_info(png_ptr,info_ptr);
	for (y = 0; y < ysize; y++) {
		for (x = 0; x < xsize; x++) {
			row[3*x] = (png_byte) (paint_data->red[y * xsize + x] * 255.0);
			row[3*x+1] = (png_byte) (paint_data->green[y * xsize + x] * 255.0);
			row[3*x+2] = (png_byte) (paint_data->blue[y * xsize + x] * 255.0);
			UT_DEBUGMSG(("PSION: Pixels %d %d %d\n",row[3*x],row[3*x+1],row[3*x+2]));
		}
		UT_DEBUGMSG(("PSION: Row %d\n",y));
		png_write_row(png_ptr,row);
	}
	png_write_end(png_ptr,info_ptr);
	free(row);
	png_destroy_write_struct(&png_ptr,&info_ptr);

	// Width
	UT_UTF8String_sprintf(buffer,"width:%dpt",xsize);
	props += buffer;
	
	// Height
	UT_UTF8String_sprintf(buffer,"; height:%dpt",ysize);
	props += buffer;
	
	// Unique ID
	iid = getDoc()->getUID(UT_UniqueId::Image);
	UT_UTF8String_sprintf(iname,"image_%d",iid);
	
	// Set the properties
	// Note that we both have to add a Data Item (containing the image) and 
	// the object (just a reference to the Data Item).
	const PP_PropertyVector propsArray = {
		"dataid", iname.utf8_str(),
		"props", props.utf8_str()
	};
	if (!(getDoc()->appendObject(PTO_Image,propsArray)))
		return UT_IE_IMPORTERROR;
	if (!(getDoc()->createDataItem(iname.utf8_str(),false,&image_buffer,
								   "image/png",NULL)))
		return UT_IE_IMPORTERROR;
	return UT_OK;
}

/*! 
 * Insert an object.
 *
 * At the moment, we only handle images. All other objects are ignored.
 */
UT_Error IE_Imp_Psion::insertObject(const psiconv_in_line_layout in_line)
{
	// Not yet implemented: object_display_section, object_icon_section,
	// Not yet implemented: object width and height
	
	// We only accept Sketch objects (pictures) for now.
	if (!in_line || !in_line->object || !in_line->object->object ||
		(in_line->object->object->type != psiconv_sketch_file)) {
		UT_DEBUGMSG(("PSION: Unsupported object (ignored)\n"));
		return UT_OK;
	}
	return insertImage(in_line);
}

/*!
 * Read all Psion paragraphs and add them to the document.
 */
UT_Error IE_Imp_Psion::readParagraphs(const psiconv_text_and_layout psiontext,
                                      const psiconv_word_styles_section style_sec)
//                                      psiconv_list embobjlst)
{
	unsigned int i,inline_nr;
	int loc;
	psiconv_paragraph paragraph;
	psiconv_in_line_layout in_line;
	UT_UCS4String text;
	psiconv_word_style style;
	const gchar *stylename;
	UT_Error res;

	// Iterate through all paragraphs
	for (i=0; i < psiconv_list_length(psiontext); i++) {

		UT_DEBUGMSG(("PSION: Importing paragraph %d\n",i));
		if (!(paragraph = (psiconv_paragraph) psiconv_list_get(psiontext,i))) {
			// Something is really wrong...
			UT_ASSERT(paragraph != NULL);
			return UT_IE_IMPORTERROR;
		}
	
		// Determine the style name; set it to Normal if it is not available
		if (!style_sec ||
		    !(style = psiconv_get_style(style_sec,paragraph->base_style)) ||
			(!style->name) || 
		  	!(stylename = prepare_style_name(style->name)))
			stylename = (const gchar *) strdup("Normal");
		if (!stylename)
			return UT_IE_NOMEMORY;
		UT_DEBUGMSG(("PSION: paragraph %d: style %s\n",i,stylename));
		
		// Add all paragraph attributes to the document
		if ((res = applyParagraphAttributes(paragraph->base_paragraph,stylename)))
			return res;
		
		// Iterate through all Psion inlines. These contain the character
		// layout information, together with the number of characters they
		// apply to.
		loc = 0;
		for(inline_nr=0; inline_nr < psiconv_list_length(paragraph->in_lines);
		    inline_nr++) {
			UT_DEBUGMSG(("Psion: paragraph %d inline %d\n",i,inline_nr));
			if (!(in_line = (psiconv_in_line_layout) psiconv_list_get(paragraph->in_lines,inline_nr))) {
				// Something is really wrong...
				UT_ASSERT(in_line != NULL);
				return UT_IE_IMPORTERROR;
			}
			// This may be an object, which needs special handling.
		 	// Objects have layout associated with them, but we will ignore
			// it. I am not sure how it would apply anyway. We will also ignore
			// all text. It should just be a single character \016, which is the
			// object marker.
			if (in_line->object) { 
				if ((res = insertObject(in_line)))
					return res;
			} else {		
				// Put all characters belonging to the current inline into text
				text.clear();
				if ((res = prepareCharacters(paragraph->text + loc,in_line->length,
					  text)))
					return res;
				// Yes, text may be empty!
				if (text.length()) {
					// Add the character layout and the text itself to the document
					if ((res = applyCharacterAttributes(in_line->layout)))
						return res;
					if (!(appendSpan((text.ucs4_str()),text.length())))
						return UT_IE_IMPORTERROR;
				}
			}
			loc += in_line->length;
		}

		// There may be text left after iterating through all inlines.
		// This remaining text gets the paragraph base_character layout.
		if (loc < psiconv_unicode_strlen(paragraph->text)) {
			// Get the remaining characters into text
			text.clear();
			if ((res = prepareCharacters(paragraph->text+loc,
			                       psiconv_unicode_strlen(paragraph->text - loc),text)))
				return res;

			// Yes, text may be empty!
			if (text.length()) {
				// Add the character layout and the text itself to the document.

				if ((res = applyCharacterAttributes(paragraph->base_character)))
					return res;

				if (!appendSpan(text.ucs4_str(),text.length()))
					return UT_IE_IMPORTERROR;
			}
		}
	}
	return UT_OK;
}

/***************************
 * class IE_Imp_Psion_Word *
 ***************************/

/*!  
 * Translate a psiconv Word file representation into an AbiWord document
 */
UT_Error IE_Imp_Psion_Word::parseFile(const psiconv_file psionfile)
{
	UT_Error res;
	bool header,footer;
	UT_DEBUGMSG(("PSION: Parsing Psion Word file\n"));

	// It really should be a Word file!
	if (psionfile->type != psiconv_word_file)
		return UT_IE_BOGUSDOCUMENT;
	psiconv_word_f file = (psiconv_word_f) (psionfile->file);
	
	// Handle all styles
	if ((res = applyStyles(file->styles_sec)))
		return res;

	// Handle the page settings (they always apply to the whole document
	if ((res = applyPageAttributes(file->page_sec,header,footer)))
		return res;

	// Handle all paragraphs with text and layout
	if ((res = readParagraphs(file->paragraphs,file->styles_sec)))
		return res;
	
	// Handle the headers and footers
	if ((res = processHeaderFooter(file->page_sec,header,footer)))
		return res;
	
	return UT_OK;
}


/*****************************
 * class IE_Imp_Psion_TextEd *
 *****************************/

/*!  
 * Translate a psiconv TextEd file representation into an AbiWord document
 */
UT_Error IE_Imp_Psion_TextEd::parseFile(const psiconv_file psionfile)
{
	UT_Error res;
	bool header,footer;
	
	UT_DEBUGMSG(("PSION: Parsing Psion Texted file\n"));
	// It really should be a TextEd file!
	if (psionfile->type != psiconv_texted_file)
		return UT_IE_BOGUSDOCUMENT;
	psiconv_texted_f file = (psiconv_texted_f) (psionfile->file);

	// Note that a TextEd document has no styles
	
	// Handle the page settings (they always apply to the whole document
	if ((res = applyPageAttributes(file->page_sec,header,footer)))
		return res;
	// Handle all paragraphs with text and layout
	if ((res = readParagraphs(file->texted_sec->paragraphs, NULL)))
		return res;
	
	// Handle the headers and footers
	if ((res = processHeaderFooter(file->page_sec,header,footer)))
		return res;
	
	return UT_OK;
}
