/* -*- mode: C++; tab-width: 2; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/* AbiSource Program Utilities
 * Copyright (C) 2002-2003 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2005 Robert Staudinger <robsta@stereolyzer.net>
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

#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-zip.h>

#include "ut_locale.h"
#include <math.h>

#include "ie_impexp_OpenWriter.h"
#include "ie_impGraphic.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"

#include "pd_Document.h"
#include "pd_Style.h"

// abiword stuff
#include "ut_xml.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_std_string.h"
#include "ut_bytebuf.h"
#include "ut_hash.h"
#include "ut_vector.h"
#include "ut_stack.h"
#include "ut_math.h"

#include "ut_debugmsg.h"

class OpenWriter_StylesStream_Listener;

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Class representing OOo page setup properties.
 * Imported values are separated for use within Abi's
 * <pagesize> and <section> tags.
 */
class OO_PageStyle
{
public:

  OO_PageStyle() : m_name("")
  {
  }

  ~OO_PageStyle()
  {
  }

  /*!
  * Parse attributes array and map keys and values
  * to Abi's.
  */
  void appendPageMaster(const std::string & name, const gchar ** atts) 
  {
    // FIXME ATM only one page setup per document can be imported
    if (!m_name.empty())
    {
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }

    m_name = name;
    parse(atts);
  }

  /*!
  * Returns the name of the current page setup style.
  */
  const gchar * getName() const 
  {
    return reinterpret_cast<const gchar *>(m_name.c_str());
  }

  /*!
  * Returns attribute array for the <pagesize> tag.
  */
  const gchar ** getAbiPageAtts(const gchar * masterName)
  {
    UT_return_val_if_fail(masterName != NULL, (const gchar **)m_pageAtts);

    if (strcmp (m_name.c_str(), masterName))
    {
      // FIXME can there be more than one master-page?
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }

    return (const gchar **)m_pageAtts;
  }
  
  /*!
  * Returns props string for the <section> tag.
  */
  const gchar * getAbiSectionProps() const
  {
    return reinterpret_cast<const gchar *>(m_sectionProps.c_str());
  }
  
private: 
	
  void parse (const gchar ** props)
  {
    const gchar * val = NULL;
    int propCtr = 0;
    double width = 0;
    double height = 0;
    
    // will go into the <pagesize> tag
    val = UT_getAttribute ("fo:page-width", props);
    if (val)
    {
      width = rint(UT_convertToDimension(val, DIM_MM));
      m_width = UT_std_string_sprintf("%f", width);
      m_pageAtts[propCtr++] = "width";
      m_pageAtts[propCtr++] = m_width.c_str();
    }
    
    val = UT_getAttribute ("fo:page-height", props);
    if (val)
    {
      height = rint(UT_convertToDimension(val, DIM_MM));
      m_height = UT_std_string_sprintf("%f", height);
      m_pageAtts[propCtr++] = "height";
      m_pageAtts[propCtr++] = m_height.c_str();
    }
    
    m_pageAtts[propCtr++] = "units";
    m_pageAtts[propCtr++] = "mm";    
    
    val = UT_getAttribute ("style:print-orientation", props);
    if (val)
    {
        m_orientation = val;
        m_pageAtts[propCtr++] = "orientation";
        m_pageAtts[propCtr++] = m_orientation.c_str();
    }
    
    m_pageAtts[propCtr++] = "page-scale";
    m_pageAtts[propCtr++] = "1.0";    
        
    // width and height are rounded to full mm because that's how they are
    // predefined in Abi and there seem to be rounding errors in oow's exporter
    fp_PageSize ps(width, height, DIM_MM);
    m_pageAtts[propCtr++] = "pagetype";
    m_pageAtts[propCtr++] = ps.getPredefinedName();

    m_pageAtts[propCtr] = 0; 

    // will go as props into the <section> tag
    val = UT_getAttribute ("fo:margin-left", props);
    if (val)
      m_marginLeft = UT_String_sprintf("page-margin-left: %s;", val);
	
    val = UT_getAttribute ("fo:margin-top", props);
    if (val)
      m_marginTop = UT_String_sprintf("page-margin-top: %s;", val);
	
    val = UT_getAttribute ("fo:margin-right", props);
    if (val)
      m_marginRight = UT_String_sprintf("page-margin-right: %s;", val);	
	
    val = UT_getAttribute ("fo:margin-bottom", props);
    if (val)
      m_marginBottom = UT_String_sprintf("page-margin-bottom: %s;", val);

    val = UT_getAttribute ("fo:background-color", props);
    if (val)
      m_backgroundColor = UT_String_sprintf("background-color: %s;", val);
	
#define APPEND_STYLE(sty) if (sty.size()) m_sectionProps += sty;
    APPEND_STYLE(m_marginLeft);
    APPEND_STYLE(m_marginTop);
    APPEND_STYLE(m_marginRight);
    APPEND_STYLE(m_marginBottom);
    APPEND_STYLE(m_backgroundColor);
#undef APPEND_STYLE
    if (m_sectionProps.size () > 0)
      m_sectionProps [m_sectionProps.size()-1] = 0; 
  }
  
  std::string m_name;

  std::string m_width;
  std::string m_height;
  std::string m_orientation;
  
  UT_String m_marginLeft;
  UT_String m_marginTop;
  UT_String m_marginRight;
  UT_String m_marginBottom;
  UT_String m_backgroundColor;
  
  static const int MAX_PAGE_ATTS = 13; // 2*(width height orientation pagetype units page-scale) 0 
  const gchar * m_pageAtts[MAX_PAGE_ATTS];
  UT_String m_sectionProps;
}; // class OO_PageStyle

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Class representing any and all OO styles
 * maps from OO->Abi styles. Used for at least 
 * all Abi section, paragraph, and span styles
 */
class OO_Style 
{
public:
  
  OO_Style (const gchar ** props, const PD_Style * pParentStyle, const bool bOpenDocument)
    : m_bColBreakBefore (false), m_bPageBreakBefore (false), m_pParentStyle (pParentStyle), m_bOpenDocument (bOpenDocument)
  {
    parse (props);
  }

  OO_Style (const OO_Style * other, const gchar ** props, PD_Style * pParentStyle, const bool bOpenDocument)
    : m_bColBreakBefore (false), m_bPageBreakBefore (false), m_pParentStyle (pParentStyle), m_bOpenDocument (bOpenDocument)
  {
    if (other)
      *this = *other;
    parse (props);
  }
  
  ~OO_Style ()
  {
  }
  
  const gchar * getAbiStyle () const
  {
    return reinterpret_cast<const gchar *>(m_styleProps.c_str());
  }
  
  /*!
   * Paragraph style, generate a column break before
   * creating a paragraph in this style
   */
  bool getColBreakBefore () const
  {
    return m_bColBreakBefore;
  }

  bool getPageBreakBefore () const
  {
    return m_bPageBreakBefore;
  }

  void parse (const gchar ** props)
  {
    const gchar * val = NULL;
    const gchar * val2 = NULL;

    val = UT_getAttribute ("fo:text-align", props);
    if (val) {
        if (!strcmp(val, "end"))
            m_align = "text-align: right;";
        else if (!strcmp(val, "center"))
            m_align = "text-align: center;";
        else if (!strcmp(val, "justify"))
            m_align = "text-align: justify;";
        else
            m_align = "text-align: left;";
    }

    val = UT_getAttribute ("fo:font-weight", props);
    if(val) {
        if (!strcmp(val, "bold"))
            m_fontWeight = "font-weight: bold;";
        else
            m_fontWeight = "font-weight: normal;";
    }

    val = UT_getAttribute("fo:font-style", props);
    if (val)
        if (!strcmp(val, "italic"))
            m_fontStyle = "font-style: italic;";
      
    val = UT_getAttribute("fo:color", props);
    if (val)
      m_color = UT_String_sprintf ("color: %s;", val);      
    
	if (m_bOpenDocument)
	    val = UT_getAttribute ("fo:background-color", props);
	else
	    val = UT_getAttribute ("style:text-background-color", props);
    if(val)
      m_bgcolor = UT_String_sprintf ("bgcolor: %s;", val);      
    
    val = UT_getAttribute("style:font-name", props);
    if(val) 
      m_fontName = UT_String_sprintf ("font-family: %s;", val);

    val = UT_getAttribute("fo:font-size", props);
    if(val) {
	  UT_Dimension dim = UT_determineDimension(val, DIM_none);
	  if (dim == DIM_PERCENT && !m_pParentStyle) {
		UT_DEBUGMSG(("*** [open-writer] no parent style to resolve '%s'\n", val));
	  }
      else if (dim == DIM_PERCENT && m_pParentStyle) {
        // calculate font-size based on parent's 
        const gchar * parentFontSize = NULL;
        double fontSize = 12;
		if (m_pParentStyle->getProperty("font-size", parentFontSize)) {
          fontSize = atoi(parentFontSize) * atoi(val) / 100.0;
        }
		else {
		  UT_DEBUGMSG(("*** [open-writer] using fallback font-size '%f'\n", fontSize));
		}
		m_fontSize = UT_String_sprintf ("font-size: %gpt;", rint(fontSize));
      }
      else
        m_fontSize = UT_String_sprintf ("font-size: %s;", val);
    }

    val = UT_getAttribute("fo:language", props);
    val2 = UT_getAttribute("fo:country", props);
    if (val && val2 && *val && *val2) {
      if (!strcmp(val, "zxx") && !strcmp(val2, "none")) {
        m_lang = "lang:-none-;"; // no proofing
      } else {
        m_lang = UT_String_sprintf ("lang:%s-%s;", val, val2);
      }
    }

    val = UT_getAttribute("style:text-position", props);
    if(val) {
      m_textPos = "text-position: ";
      if (strstr(val, "sup")) {
        m_textPos += "superscript;";
      } else if (strstr(val, "sub")) {
        m_textPos += "subscript;";
      } else {
        m_textPos += "normal;";
      }
    }

    const gchar * undr = nullptr;
    const gchar * strk = nullptr;
    if (m_bOpenDocument) {
      undr = const_cast<const gchar *>(UT_getAttribute("style:text-underline-style", props));
      strk = const_cast<const gchar *>(UT_getAttribute("style:text-line-through-style", props));
    }
    else {
      undr = const_cast<const gchar *>(UT_getAttribute("style:text-underline", props));
      strk = const_cast<const gchar *>(UT_getAttribute("style:text-crossing-out", props));
    }
    if (undr || strk) {
      m_textDecoration = "text-decoration: ";

      if(undr) {
        if (strcmp(undr, "none") != 0) {
          m_textDecoration += "underline";
        }
      }

      if (undr && strk) {
        m_textDecoration += ",";
      }

      if(strk) {
        if (strcmp(strk, "none") != 0) {
          m_textDecoration += "line-through";
        }
      }

      m_textDecoration += ";";
    }

    val = UT_getAttribute ("fo:margin-left", props);
    if(val) {
      m_marginLeft = UT_String_sprintf ("margin-left: %s;", val);
    }

    val = UT_getAttribute ("fo:margin-top", props);
    if(val) {
      m_marginTop = UT_String_sprintf ("margin-top: %s;", val);
    }

    val = UT_getAttribute ("fo:margin-right", props);
    if(val) {
      m_marginRight = UT_String_sprintf ("margin-right: %s;", val);
    }

    val = UT_getAttribute ("fo:margin-bottom", props);
    if(val) {
      m_marginBottom = UT_String_sprintf ("margin-bottom: %s;", val);
    }

    val = UT_getAttribute ("style:line-height-at-least", props);
    if (val) {
      m_lineHeight = UT_String_sprintf ("line-height: %s+;", val);
    }

    val = UT_getAttribute ("fo:line-height", props);
    if (val) {
      if (strstr(val, "%") != nullptr) {
        int spacing;

        sscanf (val, "%d%%", &spacing);
        UT_LocaleTransactor lt(LC_NUMERIC, "C");
        m_lineHeight = UT_String_sprintf ("line-height: %f;", (double)spacing/100.);
      }
      else {
        m_lineHeight = UT_String_sprintf ("line-height: %s;", val);
      }
    }
    val = UT_getAttribute("fo:keep-with-next", props);
    if (val) {
      m_keepWithNext = UT_String_sprintf ("keep-with-next: %s;", !strcmp(val, "true") ? "yes" : "no");
    }

    val = UT_getAttribute("style:break-inside", props);
    if (val) {
      m_keepTogether = UT_String_sprintf ("keep-together: %s;", !strcmp(val, "avoid") ? "yes" : "no" );
    }

    val = UT_getAttribute("fo:widows", props);
    if (val) {
      int widows = 0;
      sscanf(val, "%d", &widows);
      m_widows = UT_String_sprintf ("widows: %d", widows);
    }

    val = UT_getAttribute("fo:orphans", props);
    if (val) {
      int orphans = 0;
      sscanf (val, "%d", &orphans);
      m_orphans = UT_String_sprintf ("orphans: %d", orphans);
    }

    val = UT_getAttribute("fo:column-count", props);
    if (val) {
      int columns = 0;
      sscanf (val, "%d", &columns);

      m_columns = UT_String_sprintf ("columns: %d;", columns);
    }

    val = UT_getAttribute ("fo:break-before", props);
    if (val) {
      if (!strcmp (val, "column")) {
        m_bColBreakBefore = true;
      }
      else if (!strcmp (val, "page")) {
        m_bPageBreakBefore = true;
      }
    }

    m_styleProps = "";

#define APPEND_STYLE(sty) if (sty.size()) { m_styleProps += sty; }

    APPEND_STYLE(m_align);
    APPEND_STYLE(m_fontWeight);
    APPEND_STYLE(m_fontStyle);
    APPEND_STYLE(m_color);
    APPEND_STYLE(m_bgcolor);
    APPEND_STYLE(m_fontName);
    APPEND_STYLE(m_fontSize);
    APPEND_STYLE(m_lang);
    APPEND_STYLE(m_textPos);
    APPEND_STYLE(m_textDecoration);
    APPEND_STYLE(m_marginLeft);
    APPEND_STYLE(m_marginTop);
    APPEND_STYLE(m_marginRight);
    APPEND_STYLE(m_marginBottom);
    APPEND_STYLE(m_lineHeight);
    APPEND_STYLE(m_keepWithNext);
    APPEND_STYLE(m_keepTogether);
    APPEND_STYLE(m_widows);
    APPEND_STYLE(m_orphans);
    APPEND_STYLE(m_columns);

#undef APPEND_STYLE

    if (m_styleProps.size () > 0) {
      m_styleProps [m_styleProps.size()-1] = 0;
    }
  }

private:

  OO_Style (); // no impl

  UT_String m_align;
  UT_String m_fontWeight;
  UT_String m_fontStyle;
  UT_String m_color;
  UT_String m_bgcolor;
  UT_String m_fontName;
  UT_String m_fontSize;
  UT_String m_lang;
  UT_String m_textPos;
  UT_String m_textDecoration;
  UT_String m_marginLeft;
  UT_String m_marginTop;
  UT_String m_marginRight;
  UT_String m_marginBottom;
  UT_String m_lineHeight;
  UT_String m_keepWithNext;
  UT_String m_keepTogether;
  UT_String m_widows;
  UT_String m_orphans;
  UT_String m_columns;

  UT_String m_styleProps;

  bool m_bColBreakBefore;
  bool m_bPageBreakBefore;
  const PD_Style * m_pParentStyle;
  bool m_bOpenDocument;
}; // class OO_Style

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Class used to import OpenWriter documents
 */
class IE_Imp_OpenWriter : public IE_Imp
{
public:
  IE_Imp_OpenWriter (PD_Document * pDocument);
  virtual ~IE_Imp_OpenWriter ();

  PD_Document * getDocument () const;

  GsfInfile * getOO () const { return m_oo; }

  void defineSimpleStyle (const UT_UTF8String & name, const gchar **props);
  const gchar* mapStyle (const gchar * name) const;
  const OO_Style * mapStyleObj (const gchar * name) const;
  OpenWriter_StylesStream_Listener * m_pSSListener;

protected:

	virtual UT_Error _loadFile(GsfInput * input);

private:

  UT_Error _handleMimetype ();
  UT_Error _handleMetaStream ();
  UT_Error _handleSettingsStream ();
  UT_Error _handleStylesStream ();
  UT_Error _handleContentStream ();

  GsfInfile * m_oo;
  UT_GenericStringMap<OO_Style *> m_styleBucket;
  //! TRUE ... OASIS OpenDocument, FALSE ... OpenOffice.org 1.0 OpenWriter
  bool m_bOpenDocument;
};

/*****************************************************************************/
/*****************************************************************************/

IE_Imp_OpenWriter_Sniffer::IE_Imp_OpenWriter_Sniffer () :
  IE_ImpSniffer("OpenWriter::SXW")
{
}

IE_Imp_OpenWriter_Sniffer::~IE_Imp_OpenWriter_Sniffer ()
{
}

// supported suffixes
static IE_SuffixConfidence IE_Imp_OpenWriter_Sniffer__SuffixConfidence[] = {
	{ "stw", 	UT_CONFIDENCE_PERFECT 	},
	{ "sxw", 	UT_CONFIDENCE_PERFECT 	},
	{ "", 	UT_CONFIDENCE_ZILCH 	}
};

const IE_SuffixConfidence * IE_Imp_OpenWriter_Sniffer::getSuffixConfidence ()
{
	return IE_Imp_OpenWriter_Sniffer__SuffixConfidence;
}

/*!
 * Recognize the contents as best we can
 */
UT_Confidence_t IE_Imp_OpenWriter_Sniffer::recognizeContents (GsfInput * input)
{
    UT_Confidence_t confidence = UT_CONFIDENCE_ZILCH;

	GsfInfile * zip;

	zip = gsf_infile_zip_new (input, NULL);
	if (zip != NULL)
		{
			GsfInput* pInput = gsf_infile_child_by_name(zip, "mimetype");

			if (pInput) 
				{
					UT_UTF8String mimetype;
    
					if (gsf_input_size (pInput) > 0) {
						mimetype.append(
										(const char *)gsf_input_read(pInput, gsf_input_size (pInput), NULL),
										gsf_input_size (pInput));
					}

					if ((strcmp("application/vnd.sun.xml.writer", mimetype.utf8_str()) == 0) ||
						(strcmp("application/vnd.sun.xml.writer.template", mimetype.utf8_str()) == 0))
						confidence = UT_CONFIDENCE_PERFECT;

					g_object_unref (G_OBJECT (pInput));
				}
			// there's no mimetype stream, so let's check for a content.xml file instead
			else
				{
					pInput = gsf_infile_child_by_name(zip, "content.xml");

					gsf_off_t size = 0;
					if (pInput)
						size = gsf_input_size(pInput);

					if (size > 0)
						{
							int min = UT_MIN(size, 150);

							UT_UTF8String content;
							content.append((const char *)gsf_input_read(pInput, min, NULL));

							if (strstr(content.utf8_str(), "<!DOCTYPE office:document-content PUBLIC"))
								confidence = UT_CONFIDENCE_GOOD;

						}

					if (pInput)
						g_object_unref (G_OBJECT (pInput));

				}
			g_object_unref (G_OBJECT (zip));
		}

	return confidence;
}

/*!
 * Construct an importer for ourselves
 */
UT_Error IE_Imp_OpenWriter_Sniffer::constructImporter (PD_Document * pDocument,
						       IE_Imp ** ppie)
{
  IE_Imp_OpenWriter * p = new IE_Imp_OpenWriter(pDocument);
  *ppie = p;
  return UT_OK;
}

/*!
 * Get the dialog labels
 */
bool IE_Imp_OpenWriter_Sniffer::getDlgLabels (const char ** szDesc,
					      const char ** szSuffixList,
					      IEFileType * ft)
{
  *szDesc = "OpenOffice Writer (.stw, .sxw)";
  *szSuffixList = "*.stw; *.sxw";
  *ft = getFileType();
  return true;
}

void IE_Imp_OpenWriter::defineSimpleStyle (const UT_UTF8String & name, const gchar **props)
{
  if (!name.size() || !props)
    return;
  
  OO_Style * style = new OO_Style (props, NULL, m_bOpenDocument);
  m_styleBucket.insert (name.utf8_str(), style);
}

const gchar* IE_Imp_OpenWriter::mapStyle (const gchar * name) const
{
  OO_Style * style = m_styleBucket.pick((const char *)name);
  if (NULL == style)
    return "";
  return style->getAbiStyle (); 
}

const OO_Style * IE_Imp_OpenWriter::mapStyleObj (const gchar * name) const
{
  if (!name)
    return NULL;
  return m_styleBucket.pick((const char *)name);
}

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Create a new OpenWriter importer object
 */
IE_Imp_OpenWriter::IE_Imp_OpenWriter (PD_Document * pDocument)
  : IE_Imp (pDocument), m_pSSListener(0), m_oo (0), m_bOpenDocument(false)
{
}


/*!
 * Import the given file
 */
UT_Error IE_Imp_OpenWriter::_loadFile (GsfInput * oo_src)
{
  m_oo = GSF_INFILE (gsf_infile_zip_new (oo_src, NULL));

  if (m_oo == NULL)
    return UT_ERROR;
  
  UT_Error err = UT_OK;

  if ( UT_OK != (err = _handleMimetype ()))
    return err; // we require a mimetype
  if ( UT_OK != _handleMetaStream ()) {
    UT_DEBUGMSG(("IE_Imp_OpenWriter::_loadFile(): missing meta stream\n"));
  }
  if ( UT_OK != _handleStylesStream ()) {
    UT_DEBUGMSG(("IE_Imp_OpenWriter::_loadFile(): missing styles stream\n"));
  }
  if ( UT_OK != (err = _handleContentStream ()))
    return err; // abort because content.xml stream is compulsory
    
  return UT_OK;
}

/*!
 *
 */
PD_Document * IE_Imp_OpenWriter::getDocument () const
{
  return getDoc ();
}

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Baseclass for all OpenWriter listeners, basically a shim class
 * to expose a GetDocument() and a GetImporter() method
 */
class OpenWriter_Stream_Listener : public virtual UT_XML::Listener
{
private:
  IE_Imp_OpenWriter * m_pImporter;
  
protected:
  OpenWriter_Stream_Listener ( IE_Imp_OpenWriter * importer )
    : m_pImporter ( importer )
  {
  }

  inline IE_Imp_OpenWriter * getImporter () const { return m_pImporter; }
  inline PD_Document * getDocument() const { return m_pImporter->getDocument(); }
  
public:
  
  virtual ~OpenWriter_Stream_Listener ()
  {
  }
};

/*****************************************************************************/
/*****************************************************************************/

static const size_t BUF_SZ = 4096;

static UT_Error loadStream ( GsfInfile * oo,
			     const char * stream,
			     UT_ByteBuf & buf )
{
  guint8 const *data = NULL;
  size_t len = 0;
  
  buf.truncate (0);
  GsfInput * input = gsf_infile_child_by_name(oo, stream);

  if (!input)
    return UT_ERROR;
  
  if (gsf_input_size (input) > 0) {
    while ((len = gsf_input_remaining (input)) > 0) {
      len = UT_MIN (len, BUF_SZ);
      if (NULL == (data = gsf_input_read (input, len, NULL))) {
	g_object_unref (G_OBJECT (input));
	return UT_ERROR;
      }
      buf.append ((const UT_Byte *)data, len);
    }
  }
  
  g_object_unref (G_OBJECT (input));
  return UT_OK;
}

/*!
 * Static utility method to read a file/stream embedded inside of the
 * zipfile into an xml parser
 */
static UT_Error parseStream ( GsfInfile * oo, 
			      const char * stream,
			      UT_XML & parser )
{
  guint8 const *data = NULL;
  size_t len = 0;

  GsfInput * input = gsf_infile_child_by_name(oo, stream);

  if (!input)
    return UT_ERROR;

  if (gsf_input_size (input) > 0) {
    while ((len = gsf_input_remaining (input)) > 0) {
      // FIXME: we want to pass the stream in chunks, but libXML2 finds this disagreeable.
      // we probably need to pass some magic to our XML parser? 
      // len = UT_MIN (len, BUF_SZ);
      if (NULL == (data = gsf_input_read (input, len, NULL))) {
	g_object_unref (G_OBJECT (input));
	return UT_ERROR;
      }
      parser.parse ((const char *)data, len);
    }
  }
  
  g_object_unref (G_OBJECT (input));
  return UT_OK;
}

/*!
 * Handle the stream @stream using the listener @listener.
 * Tries to abstract away how we're actually going to handle
 * how we read the stream, so that the underlying implementation
 * can easily adapt or change
 */
static UT_Error handleStream ( GsfInfile * oo,
			       const char * stream,
			       OpenWriter_Stream_Listener & listener )
{
  UT_XML reader;
  reader.setListener ( &listener );
  return parseStream (oo, stream, reader);
}

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Class to handle meta-streams
 */
class OpenWriter_MetaStream_Listener : public OpenWriter_Stream_Listener
{
public:
  OpenWriter_MetaStream_Listener ( IE_Imp_OpenWriter * importer, bool bOpenDocument )
    : OpenWriter_Stream_Listener ( importer ), 
      m_bOpenDocument ( bOpenDocument )
  {
    if (m_bOpenDocument) {  
      getDocument()->setMetaDataProp(PD_META_KEY_FORMAT, "OpenWriter::ODT");
    }
	else {
      getDocument()->setMetaDataProp(PD_META_KEY_FORMAT, "OpenWriter::SXW");
    }
  }
  
  virtual ~OpenWriter_MetaStream_Listener ()
  {
  }
  
  virtual void startElement (const gchar * name, const gchar ** atts) 
  {
    mCharData.clear ();
    mAttrib.clear ();
    
    if (!strcmp (name, "meta:user-defined"))
	{
	  const gchar * attrib = UT_getAttribute ("meta:name", atts);
	  if (attrib != NULL)
	    mAttrib = attrib;
	}
  }
  
  virtual void endElement (const gchar * name)
  {
    if (mCharData.size()) {
      if (!strcmp (name, "dc:language"))
	getDocument()->setMetaDataProp (PD_META_KEY_LANGUAGE, mCharData);
      else if (!strcmp (name, "dc:date"))
	getDocument()->setMetaDataProp (PD_META_KEY_DATE, mCharData);
      else if (!strcmp (name, "meta:user-defined"))
	if (mAttrib.size())
	  getDocument()->setMetaDataProp (mAttrib, mCharData);
    }
    mCharData.clear ();
    mAttrib.clear ();
  }
  
  virtual void charData (const gchar * buffer, int length)
  {
    if (buffer && length)      
		mCharData += std::string (buffer, length);
  }
  
private:

	std::string mCharData;
	std::string mAttrib;
  const bool m_bOpenDocument;
};

/*!
 * Determine mimetype
 */
UT_Error IE_Imp_OpenWriter::_handleMimetype ()
{
  UT_Error err = UT_OK;

  GsfInput * input = gsf_infile_child_by_name(m_oo, "mimetype");

  if (!input) {
    // not all sxw and stw files have a mimetype stream (see Bug 11686)
    return UT_OK;
  }

  UT_UTF8String mimetype;
  if (gsf_input_size (input) > 0) {
    mimetype.append((const char *)gsf_input_read(input, gsf_input_size (input), NULL), gsf_input_size (input));
  }
  
  if (strcmp("application/vnd.sun.xml.writer", mimetype.utf8_str()) &&
   strcmp("application/vnd.sun.xml.writer.template", mimetype.utf8_str()))
  {
	UT_DEBUGMSG(("*** Unknown mimetype '%s'\n", mimetype.utf8_str()));  
    err = UT_ERROR;
  }	  
  g_object_unref (G_OBJECT (input));
  return err;
}

/*!
 * Handle the meta-stream
 */
UT_Error IE_Imp_OpenWriter::_handleMetaStream ()
{
  OpenWriter_MetaStream_Listener listener ( this, m_bOpenDocument );
  return handleStream (m_oo, "meta.xml", listener);
}

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Class to handle the settings stream
 */
class OpenWriter_SettingsStream_Listener : public OpenWriter_Stream_Listener
{
public:
  OpenWriter_SettingsStream_Listener (IE_Imp_OpenWriter * importer)
    : OpenWriter_Stream_Listener ( importer )
  {
  }

  virtual ~OpenWriter_SettingsStream_Listener ()
  {
  }

  virtual void startElement (const gchar * /*name*/, const gchar ** /*atts*/) 
  {
  }

  virtual void endElement (const gchar * /*name*/)
  {
  }

  virtual void charData (const gchar * /*buffer*/, int /*length*/)
  {
  }

private:
};

/*!
 * Handle the setting-stream
 */
UT_Error IE_Imp_OpenWriter::_handleSettingsStream ()
{
  OpenWriter_SettingsStream_Listener listener ( this );
  return handleStream (m_oo, "settings.xml", listener);
}

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Class to handle the styles stream
 */
class OpenWriter_StylesStream_Listener : public OpenWriter_Stream_Listener
{
public:
  OpenWriter_StylesStream_Listener ( IE_Imp_OpenWriter * importer, bool bOpenDocument )
    : OpenWriter_Stream_Listener ( importer ), m_ooStyle (0), m_bOpenDocument(bOpenDocument)
  {
  }
  
  virtual ~OpenWriter_StylesStream_Listener ()
  {
    m_styleNameMap.purgeData();
    DELETEP(m_ooStyle);
  }

  UT_UTF8String getStyleName(const UT_UTF8String & in) const
  {
    UT_UTF8String * name = m_styleNameMap.pick(in.utf8_str());
    if (!name)
      return UT_UTF8String(in);
    else
      return *name;
  }
  
  virtual void startElement (const gchar * name, const gchar ** atts) 
  {
	/* SXW || ODT */
    if (!strcmp (name, "style:page-master") || !strcmp (name, "style:page-layout")) {
      m_pageMaster = UT_getAttribute("style:name", atts);
    }
    else if (!strcmp (name, "style:master-page")) {
      const gchar * masterName = UT_getAttribute("style:page-master-name", atts);
      const gchar ** pageAtts = m_ooPageStyle.getAbiPageAtts(masterName);
      getDocument()->setPageSizeFromFile(PP_std_copyProps(pageAtts));
    }
    else if (!strcmp (name, "style:style")) {
      const gchar * attr = NULL;

      attr = UT_getAttribute("style:name",atts);
      if (attr)
	m_name = attr;

      attr = UT_getAttribute("style:display-name",atts);
      if (attr)
	m_displayName = attr;

      if (m_name != "Standard") {
	attr = UT_getAttribute("style:parent-style-name",atts);
	if (attr) {
	  if (!strcmp(attr, "Standard")) {
	    m_parent = "Normal";
	  } else {
	    m_parent = attr;
	  }
	}
	
	attr = UT_getAttribute("style:next-style-name",atts);
	if (attr) {
	  if (!strcmp(attr, "Standard")) {
	    m_next = "Normal";
	  } else {
	    m_next = attr;
	  }
	}

	attr = UT_getAttribute("style:family",atts);
	if (attr && strcmp (attr, "paragraph") != 0)
	  m_type = CHARACTER;
	else
	  m_type = PARAGRAPH;
      } 
      else {
	// importantly map standard back to "Normal"
	m_parent = "Normal";
	m_next = "Normal";
	m_type = PARAGRAPH;
      }
      DELETEP(m_ooStyle);
      m_ooStyle = NULL;
    }
    else if (/* SXW || ODT */
	     (!strcmp (name, "style:properties") || !strcmp (name, "style:page-layout-properties")) && 
	     !m_pageMaster.empty()) {
      // page setup, because we are in page-master subtree
      m_ooPageStyle.appendPageMaster(m_pageMaster, atts);
    }
    else if (!strcmp (name, "style:properties") || 				/* SXW */
	     !strcmp (name, "style:text-properties") || 		/* ODT */
	     !strcmp (name, "style:paragraph-properties")) {
      
      if (m_ooStyle == NULL) {
	getDocument()->getStyle (m_parent.utf8_str(), &m_pParentStyle);
	m_ooStyle = new OO_Style (atts, m_pParentStyle, m_bOpenDocument);
      }
      else {
	m_ooStyle->parse(atts);
      }
    }
  } 

  virtual void endElement (const gchar * name)
  {
    if (!strcmp (name, "style:page-master")) {
      m_pageMaster.clear();
    }
    else if (!strcmp (name, "style:style")) {
      if (m_name.size ()) {
	int propCtr = 0;
	
	const gchar * atts[11];
	atts[propCtr++] = "type";
	atts[propCtr++] = (m_type == PARAGRAPH ? "P" : "C");

	atts[propCtr++] = "name";
	if (m_displayName.size()) {	 
	  atts[propCtr++] = m_displayName.utf8_str();
	  m_styleNameMap.insert (m_name.utf8_str(), new UT_UTF8String(m_displayName));
	}
	else {
	  atts[propCtr++] = m_name.utf8_str();
	  m_styleNameMap.insert (m_name.utf8_str(), new UT_UTF8String(m_name));
	}

	if (m_ooStyle) {
	  atts[propCtr++] = "props";
	  atts[propCtr++] = m_ooStyle->getAbiStyle ();
	}

	if (m_parent.size ()) {
	  atts[propCtr++] = "basedon";
	  atts[propCtr++] = m_parent.utf8_str();
	}

	if (m_next.size ()) {
	  atts[propCtr++] = "followedby";
	  atts[propCtr++] = m_next.utf8_str();
	}

	// must be last
	atts[propCtr] = 0;
	getDocument()->appendStyle(PP_std_copyProps(atts));
      }

      m_name.clear ();
      m_displayName.clear ();
      m_parent.clear ();
      m_next.clear ();
      DELETEP(m_ooStyle);
      m_ooStyle = NULL;
    }
  }
  
  virtual void charData (const gchar * /*buffer*/, int /*length*/)
  {
  }  

  const gchar * getSectionProps() const 
  {
    if (!strcmp (m_ooPageStyle.getName(), "")) 
    {
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
      return NULL;
    }
    return m_ooPageStyle.getAbiSectionProps();
  }
  
private:

  UT_UTF8String m_name;
  UT_UTF8String m_displayName;
  UT_UTF8String m_parent;
  UT_UTF8String m_next;
  enum { CHARACTER, PARAGRAPH } m_type;
  OO_Style *m_ooStyle;
  PD_Style *m_pParentStyle;
  OO_PageStyle m_ooPageStyle;
  std::string m_pageMaster;
  const bool m_bOpenDocument;

  UT_GenericStringMap<UT_UTF8String *> m_styleNameMap;
};

/*!
 * Handle the styles-stream
 */
UT_Error IE_Imp_OpenWriter::_handleStylesStream ()
{
  m_pSSListener = new OpenWriter_StylesStream_Listener(this, m_bOpenDocument);
  // Collect styles from both styles.xml and content.xml streams
  UT_Error result1 = handleStream (m_oo, "styles.xml", *m_pSSListener);
  UT_Error result2 = handleStream (m_oo, "content.xml", *m_pSSListener);
  // return the biggest negative value or 0 if both are UT_OK
  return (result1 < result2 ? result1 : result2);
}

/*****************************************************************************/
/*****************************************************************************/

/*!
 * Class to handle the content stream
 */
class OpenWriter_ContentStream_Listener : public OpenWriter_Stream_Listener
{
private:
  UT_UCS4String m_charData;
  bool m_bAcceptingText;
  bool m_bInSection;
  bool m_bInTOC;

  UT_UTF8String m_curStyleName;

  PP_PropertyVector	m_vecInlineFmt;
  UT_NumberStack		m_stackFmtStartIndex;
  const OpenWriter_StylesStream_Listener * m_pSSListener;

  UT_uint32 m_imgCnt;

  int m_row;
  int m_col;
  int m_cel;

  const bool m_bOpenDocument;

public:
  OpenWriter_ContentStream_Listener ( IE_Imp_OpenWriter * importer, OpenWriter_StylesStream_Listener * pSSListener, bool bOpenDocument )
    : OpenWriter_Stream_Listener ( importer ), m_bAcceptingText(false), m_bInSection(false), m_bInTOC(false),
      m_pSSListener(pSSListener), m_imgCnt(0), m_row(0), m_col(0), m_cel(0), m_bOpenDocument(bOpenDocument)
  {
  }

  virtual ~OpenWriter_ContentStream_Listener ()
  {
  }

  virtual void startElement (const gchar * name, const gchar ** atts) 
  {
    // if we're inside a TOC, ignore the contents since SXW/ODT include a copy of the TOC
    if (m_bInTOC)
      return;

    if (!strcmp(name, "office:body"))
      {
      }
    else if (!strcmp(name, "text:section" ))
      {	
	const gchar * oo_sty = UT_getAttribute ("text:style-name", atts);
	const gchar * abi_sty = _mapStyle(oo_sty);

	_insureInSection(abi_sty);
      }
    else if (!strcmp(name, "text:p" ) || !strcmp(name, "text:h" ))
      {
	UT_UTF8String oo_sty;
	const OO_Style * abi_sty = _mapStyleObj (UT_getAttribute ("text:style-name", atts), oo_sty);

	if (abi_sty) {
	  // append an empty paragraph with this one char
	  if (abi_sty->getColBreakBefore () || abi_sty->getPageBreakBefore ()) {
	    _insureInBlock(NULL);
	    UT_UCSChar ucs = (abi_sty->getColBreakBefore () ? UCS_VTAB : UCS_FF);
	    getDocument()->appendSpan (&ucs, 1);
	  }
	  
	  const gchar * props[3];
	  props[0] = "props";
	  props[1] = abi_sty->getAbiStyle ();
	  props[2] = 0;
	  
	  _insureInBlock((const gchar **)props);
	}
	else if (oo_sty.size()) {
	  const gchar * props[3];
	  props[0] = "style";
	  props[1] = oo_sty.utf8_str();
	  props[2] = 0;

	  _insureInBlock((const gchar **)props);
	}
	else
	  _insureInBlock(NULL);
	m_bAcceptingText = true;
      }
    else if (!strcmp(name, "text:span"))
      {
	_flush ();

	const gchar * oo_sty = UT_getAttribute ("text:style-name", atts);
	const gchar * abi_sty = _mapStyle(oo_sty);

	const gchar * props[3];

	if(abi_sty && *abi_sty) {
	  props[0] = "props";
	  props[1] = abi_sty;
	  props[2] = 0;
	} else {
	  props[0] = 0;
	}
	  
	_pushInlineFmt(props);
	getDocument()->appendFmt(m_vecInlineFmt);
      }
    else if (!strcmp(name, "text:line-break"))
      {
	m_charData += UCS_LF;
	_flush ();
      }
    else if (!strcmp(name, "text:tab-stop"))
      {
	m_charData += UCS_TAB;
	_flush ();
      }
    else if (!strcmp(name, "text:ordered-list") || !strcmp(name, "text:unordered-list"))
      {
        PP_PropertyVector list_atts = {
          // list type
          "type", "",
          "id", "0"
        };
        if (!strcmp(name, "text:ordered-list")) {
          list_atts[3] = "Numbered List";
        } else {
          list_atts[3] = "Bullet List";
        }
        getDocument()->appendList(list_atts);

        UT_DEBUGMSG(("DOM: appended a list\n"));
      }
    else if (!strcmp(name, "style:style"))
      {
	m_curStyleName.clear ();
	m_curStyleName = UT_getAttribute ("style:name", atts);
      }
    else if (/* SXW */
			!strcmp(name, "style:properties") || 
			!strcmp(name, "style:columns") ||
			/* ODT */
			!strcmp(name, "style:text-properties"))
      {
	_defineSimpleStyle (atts);
      }
    else if (!strcmp(name, "text:a"))
      {
        _flush();
        const PP_PropertyVector xlink_atts = {
          "xlink:href", UT_getAttribute("xlink:href", atts)
        };
        getDocument()->appendObject(PTO_Hyperlink, xlink_atts);
      }
    else if (!strcmp(name, "text:bookmark"))
    {
      _flush ();
      const gchar * attr = UT_getAttribute ("text:name", atts);

      if(attr)
      {
        _insertBookmark (attr, "start");
        _insertBookmark (attr, "end");
      }
      else
      {
        UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
      }
    }
    else if (!strcmp(name, "text:bookmark-start"))
    {
      _flush ();
      const gchar * attr = UT_getAttribute ("text:name", atts);

      if(attr)
      {
        _insertBookmark (attr, "start");
      }
      else
      {
        UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
      }
    }
    else if (!strcmp(name, "text:bookmark-end"))
    {
      _flush ();
      const gchar * attr = UT_getAttribute ("text:name", atts);

      if(attr)
      {
        _insertBookmark (attr, "end");
      }
      else
      {
        UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
      }
    }
    else if (!strcmp(name, "text:table-of-content"))
    {
      _flush ();
      _insureInBlock(NULL);
      
      getDocument()->appendStrux(PTX_SectionTOC, PP_NOPROPS);
      getDocument()->appendStrux(PTX_EndTOC, PP_NOPROPS);

      m_bInTOC = true;
    }
    else if (!strcmp(name, "draw:image"))
      {
	_flush ();
	_insureInBlock(NULL);
	_insertImage (atts);
      }
    else if (!strcmp(name, "table:table"))
      {
	_insureInSection(NULL);
	_openTable (atts);
      }
    else if (!strcmp(name, "table:table-column"))
      {
	_openColumn (atts);
      }
    else if (!strcmp(name, "table:table-row"))
      {
	_openRow (atts);
      }
    else if (!strcmp(name, "table:table-cell"))
      {
	_openCell (atts);
      }
    else if (!strcmp(name, "text:date") ||
	     !strcmp(name, "text:time") || 
	     !strcmp(name, "text:page-number") ||
	     !strcmp(name, "text:page-count") || 
	     !strcmp(name, "text:file-name") ||
	     !strcmp(name, "text:paragraph-count") ||
	     !strcmp(name, "text:word-count") ||
	     !strcmp(name, "text:character-count") ||
	     !strcmp(name, "text:initial-creator") ||
	     !strcmp(name, "text:author-name") ||
	     !strcmp(name, "text:description") ||
	     !strcmp(name, "text:keywords") ||
	     !strcmp(name, "text:subject") ||
	     !strcmp(name, "text:title"))
    {
      _flush ();

      const gchar * type = "";
      if(!strcmp(name, "text:date"))
	type = "date_ddmmyy";
      else if(!strcmp(name, "text:time"))
	type = "time";
      else if(!strcmp(name, "text:page-number"))
	type = "page_number";
      else if(!strcmp(name, "text:page-count"))
	type = "page_count";
      else if(!strcmp(name, "text:file-name"))
	type = "file_name";
      else if(!strcmp(name, "text:paragraph-count"))
	type = "para_count";
      else if(!strcmp(name, "text:word-count"))
	type = "word_count";
      else if(!strcmp(name, "text:character-count"))
	type = "char_count";
      else if(!strcmp(name, "text:initial-creator") || !strcmp(name, "text:author-name"))
	type = "meta_creator";
      else if(!strcmp(name, "text:description"))
	type = "meta_description";
      else if(!strcmp(name, "text:keywords"))
	type = "meta_keywords";
      else if(!strcmp(name, "text:subject"))
	type = "meta_subject";
      else if(!strcmp(name, "text:title"))
	type = "meta_title";

      const PP_PropertyVector field_fmt = {
        "type", type
      };
      getDocument()->appendObject(PTO_Field, field_fmt);
      m_bAcceptingText = false;
    }
  }

  virtual void endElement (const gchar * name)
  {
    if (!strcmp(name, "text:section" ))
      {
	m_bInSection = false;
      }
    else if (!strcmp(name, "text:p" ) || !strcmp(name, "text:h" ))
      {
	_flush ();
	m_bAcceptingText = false;
      }
    else if (!strcmp(name, "text:span"))
      {
	_flush ();
	_popInlineFmt();
	getDocument()->appendFmt(m_vecInlineFmt);
      }
    else if (!strcmp(name, "text:ordered-list") || !strcmp(name, "text:unordered-list"))
      {
      }
    else if (!strcmp(name, "text:a"))
      {
	_flush ();
	getDocument()->appendObject(PTO_Hyperlink, PP_NOPROPS);
      }
    else if (!strcmp(name, "text:table-of-content")) {
      m_bInTOC = false;
    }
    else if (!strcmp(name, "table:table"))
      {
	_closeTable ();
      }
    else if (!strcmp(name, "table:table-column"))
      {
	_closeColumn ();
      }
    else if (!strcmp(name, "table:table-row"))
      {
	_closeRow ();
      }
    else if (!strcmp(name, "table:table-cell"))
      {
	_closeCell ();
      }
    else if (!strcmp(name, "text:date") ||
	     !strcmp(name, "text:time") || 
	     !strcmp(name, "text:page-number") ||
	     !strcmp(name, "text:page-count") || 
	     !strcmp(name, "text:file-name") ||
	     !strcmp(name, "text:paragraph-count") ||
	     !strcmp(name, "text:word-count") ||
	     !strcmp(name, "text:character-count") ||
	     !strcmp(name, "text:initial-creator") ||
	     !strcmp(name, "text:author-name") ||
	     !strcmp(name, "text:description") ||
	     !strcmp(name, "text:keywords") ||
	     !strcmp(name, "text:subject") ||
	     !strcmp(name, "text:title"))
    {
      m_bAcceptingText = true;
    }
  }

  virtual void charData (const gchar * buffer, int length)
  {
    if (buffer && length && m_bAcceptingText && !m_bInTOC)
      m_charData += UT_UCS4String (buffer, length, true);
  }

private:

  void _insertImage (const gchar ** atts)
  {
    UT_Error error		= UT_OK;
    const gchar * width  = UT_getAttribute ("svg:width", atts);
    const gchar * height = UT_getAttribute ("svg:height", atts);
    const gchar * href   = UT_getAttribute ("xlink:href", atts);

    if (width == NULL || height == NULL || href == NULL)
        return; //don't crash on invalid images

    m_imgCnt++;

    UT_ByteBuf img_buf;

    GsfInfile * pictures_dir = GSF_INFILE(gsf_infile_child_by_name(getImporter()->getOO(), "Pictures"));

	if (m_bOpenDocument) {
	    // 9 == strlen("Pictures/");
	    error = loadStream(pictures_dir, href+9, img_buf);
	    g_object_unref (G_OBJECT (pictures_dir));
	}
	else {
	    // 10 == strlen("#Pictures/");
	    error = loadStream(pictures_dir, href+10, img_buf);
	    g_object_unref (G_OBJECT (pictures_dir));
	}

    if (error != UT_OK)
      return;

    FG_ConstGraphicPtr pFG;
    const UT_ByteBuf * pictData       = 0;
    
    UT_String propBuffer;
    UT_String propsName;
    
    error = IE_ImpGraphic::loadGraphic (img_buf, IEGFT_Unknown, pFG);
	if ((error != UT_OK) || !pFG)
      {
		  // pictData is already freed in ~FG_Graphic
		  return;
      }

    // TODO: can we get back a vector graphic?
    pictData = pFG->getBuffer();
    
    if (!pictData)
      {
		  // i don't think that this could ever happen, but...
		  error = UT_ERROR;
		  return;
      }

    //
    // This next bit of code will set up our properties based on the image attributes
    //

    UT_String_sprintf(propBuffer, "width:%s; height:%s", width, height);
    UT_String_sprintf(propsName, "image%d", m_imgCnt);

    PP_PropertyVector propsArray = {
      "props", propBuffer.c_str(),
      "dataid", propsName.c_str()
    };

    if (!getDocument()->appendObject (PTO_Image, propsArray))
      {
		  return;
      }

    if (!getDocument()->createDataItem(propsName.c_str(), false,
                                       pictData, pFG->getMimeType(), NULL))
      {
		  return;
      }
    
  }

  void _insureInBlock(const gchar ** atts)
  {
    if (m_bAcceptingText)
      return;

    _insureInSection(NULL);

    if (!m_bAcceptingText) {
      getDocument()->appendStrux(PTX_Block, PP_std_copyProps(atts));
      m_bAcceptingText = true;
    }
  }

  void _insureInSection(const gchar * props)
  {
    if (m_bInSection)
      return;

    UT_String allProps(props);
    allProps += m_pSSListener->getSectionProps();

    const PP_PropertyVector atts = {
      "props", allProps.c_str()
    };
    getDocument()->appendStrux(PTX_Section, atts);

    m_bInSection = true;
    m_bAcceptingText = false;
  }
  
  void _insertBookmark (const gchar * name, const gchar * type)
    {
	UT_return_if_fail(name && type);

	const PP_PropertyVector propsArray = {
		"name",	name,	"type",	type
	};
	getDocument()->appendObject (PTO_Bookmark, propsArray);
    }

  void _flush ()
  {
    if (m_charData.size () > 0)
      {
	getDocument()->appendSpan (m_charData.ucs4_str(), m_charData.size ());
	m_charData.clear ();
      }	
  }

  void _openTable (const gchar ** /*props*/)
  {
    getDocument()->appendStrux(PTX_SectionTable,PP_NOPROPS);
  }

  void _openColumn (const gchar ** /*props*/)
  {
    m_col++;
  }

  void _openRow (const gchar ** /*props*/)
  {
    m_row++; m_cel = 0;
  }

  void _openCell (const gchar ** /*props*/)
  {
    std::string attach = UT_std_string_sprintf("left-attach: %d; top-attach: %d; right-attach: %d; bot-attach: %d", m_cel, m_row-1, m_cel+1, m_row);

    m_cel++;

    const PP_PropertyVector cell_props = {
      "props", attach
    };

    getDocument()->appendStrux(PTX_SectionCell,cell_props);
  }

  void _closeTable ()
  {
    getDocument()->appendStrux(PTX_EndTable, PP_NOPROPS);

    m_row = m_cel = m_col = 0;
  }

  void _closeColumn ()
  {
    m_col--;
  }

  void _closeRow ()
  {
    m_col--;
  }

  void _closeCell ()
  {
    getDocument()->appendStrux(PTX_EndCell,PP_NOPROPS);
  }
  
  void _defineSimpleStyle (const gchar **props)
  {
    getImporter ()->defineSimpleStyle (m_curStyleName, props);
  }

  const gchar* _mapStyle (const gchar * name) const
  {
    UT_UTF8String styleName = m_pSSListener->getStyleName(name);
    return getImporter ()->mapStyle(styleName.utf8_str());
  }

  const OO_Style * _mapStyleObj (const gchar * name, UT_UTF8String & oo_sty) const
  {
    oo_sty = m_pSSListener->getStyleName(name);
    return getImporter ()->mapStyleObj (oo_sty.utf8_str());
  }

  bool _pushInlineFmt(const gchar ** atts)
  {
    UT_uint32 start = m_vecInlineFmt.size() + 1;
    UT_uint32 k;
    
    for (k=0; (atts[k]); k++)
      {
          m_vecInlineFmt.push_back(atts[k]);
      }
    if (!m_stackFmtStartIndex.push(start))
      return false;
    return true;
  }
  
  void _popInlineFmt(void)
  {
    UT_sint32 start;
    if (!m_stackFmtStartIndex.pop(&start))
      return;

    m_vecInlineFmt.erase(m_vecInlineFmt.begin() + (start - 1),
                         m_vecInlineFmt.end());
  }
  
};

/*!
 * Handle the content-stream
 */
UT_Error IE_Imp_OpenWriter::_handleContentStream ()
{
  OpenWriter_ContentStream_Listener listener (this, m_pSSListener, m_bOpenDocument);
  return handleStream (m_oo, "content.xml", listener);
}


/*!
 * Destroy an OpenWriter importer object
 */
IE_Imp_OpenWriter::~IE_Imp_OpenWriter ()
{
  if (m_oo)
    g_object_unref (G_OBJECT(m_oo));

  DELETEP(m_pSSListener);
  m_styleBucket.purgeData();
}
