/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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

// Class definition include
#include <OXMLi_ListenerState_Common.h>

// Internal includes
#include <OXMLi_Types.h>
#include <OXMLi_PackageManager.h>
#include <OXML_Document.h>
#include <OXML_Element.h>
#include <OXML_Element_Run.h>
#include <OXML_Element_Text.h>
#include <OXML_Types.h>
#include <OXML_Theme.h>
#include <OXML_Style.h>
#include <OXML_Section.h>
#include <OXML_FontManager.h>

// AbiWord includes
#include <ut_units.h>
#include <ut_misc.h>
#include <ut_debugmsg.h>

// External includes
#include <cstring>

OXMLi_ListenerState_Common::OXMLi_ListenerState_Common() : 
	OXMLi_ListenerState(), 
	m_pendingSectBreak(false)
{

}

OXMLi_ListenerState_Common::~OXMLi_ListenerState_Common()
{
}

void OXMLi_ListenerState_Common::startElement (OXMLi_StartElementRequest * rqst)
{
	UT_return_if_fail( this->_error_if_fail(rqst != NULL) );

	if (!strcmp(rqst->pName, "p")) {
		//New paragraph...
		OXML_SharedElement elem(new OXML_Element("", P_TAG, BLOCK));
		rqst->stck->push(elem);

		rqst->handled = true;
	} else if (!strcmp(rqst->pName, "r")) {
		//New text run...
		OXML_SharedElement elem(new OXML_Element_Run(""));
		rqst->stck->push(elem);

		rqst->handled = true;

	} else if (!strcmp(rqst->pName, "sectPr")) {
		//Verify the context...
		std::string contextTag = rqst->context->back();
		if (!contextTag.compare("pPr") || !contextTag.compare("body")) {
			OXML_SharedElement dummy(new OXML_Element("Dummy", P_TAG, BLOCK));
			rqst->stck->push(dummy);

			m_pendingSectBreak = true;
			rqst->handled = true;
		}

/********************************
 ****  PARAGRAPH FORMATTING  ****
 ********************************/

	} else if ( !strcmp(rqst->pName, "jc") ||
				!strcmp(rqst->pName, "ind") ||
				!strcmp(rqst->pName, "spacing") ||
				!strcmp(rqst->pName, "pStyle")) {
	//Verify the context...
	std::string contextTag = rqst->context->at(rqst->context->size() - 2);
	if (!contextTag.compare("p") || !contextTag.compare("pPrDefault") || !contextTag.compare("style")) { 
		OXML_SharedElement para = rqst->stck->top();

		if (!strcmp(rqst->pName, "jc")) {
			const gchar * val = UT_getAttribute("w:val", rqst->ppAtts);

			if (!val || !*val)
				return;

			if (!strcmp(val, "left")) {
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("text-align", "left") ));
			} else if (!strcmp(val, "center")) {
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("text-align", "center") ));
			} else if (!strcmp(val, "right")) {
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("text-align", "right") ));
			} else if (!strcmp(val, "numTab")) {
				//Deprecated; align left
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("text-align", "left") ));
			} else {
				//We justify for the values of "both", "distribute", "thaiDistribute", and the kashida variants
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("text-align", "justify") ));
			}

		} else if (!strcmp(rqst->pName, "ind")) {
			const gchar * left = UT_getAttribute("w:left", rqst->ppAtts);
			const gchar * right = UT_getAttribute("w:right", rqst->ppAtts);
			const gchar * fLine = UT_getAttribute("w:firstLine", rqst->ppAtts);
			const gchar * hanging = UT_getAttribute("w:hanging", rqst->ppAtts);

			std::string final = "";
			if (left != NULL) {
				final = _TwipsToPoints(left); //convert to points
				final += "pt";
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("margin-left", final.c_str()) ));
			}
			if (right != NULL) {
				final = _TwipsToPoints(right); //convert to points
				final += "pt";
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("margin-right", final.c_str()) ));
			}
			if (fLine != NULL) {
				final = _TwipsToPoints(fLine); //convert to points
				final += "pt";
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("text-indent", final.c_str()) ));
			} else if (hanging != NULL) {
				final = _TwipsToPoints(hanging); //convert to points
				//This is hanging, invert the sign
				if (final[0] == '-')
					final.erase(0,1);
				else
					final.insert(0,1,'-');
				final += "pt";
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("text-indent", final.c_str()) ));
			}

		} else if (!strcmp(rqst->pName, "spacing")) {
			const gchar * before = UT_getAttribute("w:before", rqst->ppAtts);
			const gchar * after = UT_getAttribute("w:after", rqst->ppAtts);
			const gchar * lineRule = UT_getAttribute("w:lineRule", rqst->ppAtts);

			std::string final = "";
			if (before != NULL) {
				final = _TwipsToPoints(before); //convert to points
				final += "pt";
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("margin-top", final.c_str()) ));
			}
			if (after != NULL) {
				final = _TwipsToPoints(after); //convert to points
				final += "pt";
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("margin-bottom", final.c_str()) ));
			}
			if (lineRule != NULL && !strcmp(lineRule, "auto")) {
				//For now, we only handle "auto".
				const gchar * line = UT_getAttribute("w:line", rqst->ppAtts);
				UT_return_if_fail( _error_if_fail(line != NULL) );
				double ln_spc = UT_convertDimensionless(line) / 240;
				final = UT_convertToDimensionlessString(ln_spc);
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("line-height", final.c_str()) ));
			}
		} else if (!strcmp(rqst->pName, "pStyle")) {
			const gchar * val = UT_getAttribute("w:val", rqst->ppAtts);
			UT_return_if_fail( _error_if_fail(val != NULL) );
			if (!strcmp(val, "Normal")) val = "_Normal"; //Cannot interfere with document defaults
			OXML_Document * doc = OXML_Document::getInstance();
			UT_return_if_fail( _error_if_fail(doc != NULL) );
			OXML_SharedStyle ref = doc->getStyleById(val);
			if (ref.get() != NULL && ref->getName().compare("")) {
				UT_return_if_fail( _error_if_fail( UT_OK == para->setAttribute(PT_STYLE_ATTRIBUTE_NAME, ref->getName().c_str()) ));
			}
			
		}

		rqst->handled = true;
	}

/******* END OF PARAGRAPH FORMATTING ********/

/**************************
 ****  RUN FORMATTING  ****
 **************************/
	} else if (	!strcmp(rqst->pName, "b") || 
				!strcmp(rqst->pName, "i") || 
				!strcmp(rqst->pName, "u") ||
				!strcmp(rqst->pName, "color") ||
				!strcmp(rqst->pName, "highlight") ||
				!strcmp(rqst->pName, "strike") ||
				!strcmp(rqst->pName, "dstrike") ||
				!strcmp(rqst->pName, "rFonts") ||
				!strcmp(rqst->pName, "lang") ||
				!strcmp(rqst->pName, "noProof") ||
				!strcmp(rqst->pName, "vanish") ||
				!strcmp(rqst->pName, "sz") ) {
		//Verify the context...
		std::string contextTag = rqst->context->at(rqst->context->size() - 2);
		if (!contextTag.compare("r") || !contextTag.compare("rPrDefault") || !contextTag.compare("style")) {
			OXML_SharedElement run = rqst->stck->top();

			if (!strcmp(rqst->pName, "b")) {
				const gchar * isOn = UT_getAttribute("w:val", rqst->ppAtts);
				if (isOn == NULL || !strcmp(isOn, "on") || !strcmp(isOn, "1") || !strcmp(isOn, "true") ) {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("font-weight", "bold") ));
				} else {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("font-weight", "normal") ));
				}

			} else if (!strcmp(rqst->pName, "i")) {
				const gchar * isOn = UT_getAttribute("w:val", rqst->ppAtts);
				if (isOn == NULL || !strcmp(isOn, "on") || !strcmp(isOn, "1") || !strcmp(isOn, "true") ) {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("font-style", "italic") ));
				} else {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("font-style", "normal") ));
				}

			} else if (!strcmp(rqst->pName, "u")) {
				const gchar * newVal = UT_getAttribute("w:val", rqst->ppAtts);
				UT_return_if_fail( this->_error_if_fail(newVal != NULL) );
				std::string final_val = "";
				const gchar * previousVal = NULL;
				if (UT_OK == run->getProperty("text-decoration", previousVal)) {
					final_val = previousVal;
				}
				if ( !strcmp(newVal, "none") ) { 
					final_val += " ";
					final_val += "none";
				} else { //if NOT "none", we add underline (no matter the underline style, we only support single line)
					final_val += " ";
					final_val += "underline";
				}
				UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("text-decoration", final_val.c_str()) ));

			} else if (!strcmp(rqst->pName, "color")) {
				const gchar * val = UT_getAttribute("w:val", rqst->ppAtts);
				if (val != NULL) {
					if (!strcmp(val, "auto")) val = "#000000";
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("color", val)));
				} else {
					val = UT_getAttribute("w:themeColor", rqst->ppAtts);
					UT_return_if_fail( this->_error_if_fail(val != NULL) );
					std::string color = "#000000"; //default color in case of illegal themeColor value.
					OXML_Document * doc = OXML_Document::getInstance();
					UT_return_if_fail( this->_error_if_fail(doc != NULL) );
					OXML_SharedTheme theme = doc->getTheme();
					if (!strcmp(val,"accent1")) {
						color = theme->getColor(ACCENT1);
					} else if (!strcmp(val,"accent2")) {
						color = theme->getColor(ACCENT2);
					} else if (!strcmp(val,"accent3")) {
						color = theme->getColor(ACCENT3);
					} else if (!strcmp(val,"accent4")) {
						color = theme->getColor(ACCENT4);
					} else if (!strcmp(val,"accent5")) {
						color = theme->getColor(ACCENT5);
					} else if (!strcmp(val,"accent6")) {
						color = theme->getColor(ACCENT6);
					} else if (!strcmp(val,"dark1")) {
						color = theme->getColor(DARK1);
					} else if (!strcmp(val,"dark2")) {
						color = theme->getColor(DARK2);
					} else if (!strcmp(val,"light1")) {
						color = theme->getColor(LIGHT1);
					} else if (!strcmp(val,"light2")) {
						color = theme->getColor(LIGHT2);
					} else if (!strcmp(val,"hlink")) {
						color = theme->getColor(HYPERLINK);
					} else if (!strcmp(val,"folHlink")) {
						color = theme->getColor(FOLLOWED_HYPERLINK);
					} else if (!strcmp(val,"none")) {
						color = "#000000";
					}
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("color", color.c_str())));
				}

			} else if (!strcmp(rqst->pName, "highlight")) {
				const gchar * val = UT_getAttribute("w:val", rqst->ppAtts);
				UT_return_if_fail( this->_error_if_fail(val != NULL) );
				if (!strcmp(val, "darkYellow")) val = "olive"; //the only value not supported by CSS (equivalent to Olive)
				else if (!strcmp(val, "none")) val = "black"; //bypass inherited color value when "none"
				std::string hex = "";
				for (UT_uint32 i = 0; i <= strlen(val); i++) {
					hex += tolower(val[i]);
				}
				UT_HashColor conv;
				val = conv.setColor(hex.c_str());
				UT_return_if_fail( this->_error_if_fail( NULL != val ) );
				UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("bgcolor", val)));

			} else if (!strcmp(rqst->pName, "strike") || !strcmp(rqst->pName, "dstrike")) {
				const gchar * isOn = UT_getAttribute("w:val", rqst->ppAtts);
				std::string final_val = "";
				const gchar * previousVal = NULL;
				if (UT_OK == run->getProperty("text-decoration", previousVal)) {
					final_val = previousVal;
				}
				if ( isOn == NULL || !strcmp(isOn, "on") || !strcmp(isOn, "1") || !strcmp(isOn, "true")  ) { 
					final_val += " ";
					final_val += "line-through";
				} else {
					final_val += " ";
					final_val += "none";
				}
				UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("text-decoration", final_val.c_str()) ));

			} else if (!strcmp(rqst->pName, "rFonts")) {
				OXML_Document * doc = OXML_Document::getInstance();
				UT_return_if_fail( this->_error_if_fail(doc != NULL) );
				OXML_SharedFontManager fmgr = doc->getFontManager();
				UT_return_if_fail( this->_error_if_fail(fmgr.get() != NULL) );

				std::string fontName;
				OXML_FontLevel level = UNKNOWN_LEVEL;
				OXML_CharRange range = UNKNOWN_RANGE;

				const gchar * ascii = NULL; //TODO: add support for eastAsia, bidi, and hAnsi
				if (NULL != (ascii = UT_getAttribute("w:asciiTheme", rqst->ppAtts))) {
					this->getFontLevelRange(ascii, level, range);
					fontName = fmgr->getValidFont(level, range); //Retrieve valid font name from Theme
				} else if (NULL != (ascii = UT_getAttribute("w:ascii", rqst->ppAtts))) {
					fontName = ascii;
					fontName = fmgr->getValidFont(fontName); //Make sure the name is valid
				} else {
					fontName = fmgr->getDefaultFont();
				}
				UT_return_if_fail( _error_if_fail( UT_OK == run->setProperty("font-family", fontName.c_str()) ));

			} else if (!strcmp(rqst->pName, "lang")) {
				const gchar * val = UT_getAttribute("w:val", rqst->ppAtts); //TODO: add support for eastAsia and bidi attributes
				const gchar * previousVal = NULL;
				if (UT_OK == run->getProperty("lang", previousVal)) {
					if ( 0 != strcmp(previousVal, "-none-"))
						val = previousVal;
				}
				if ( val != NULL)
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("lang", val) ));

			} else if (!strcmp(rqst->pName, "noProof")) {
				//noProof has priority over lang, so no need to check for previous values
				const gchar * isOn = UT_getAttribute("w:val", rqst->ppAtts);
				if (isOn == NULL || !strcmp(isOn, "on") || !strcmp(isOn, "1") || !strcmp(isOn, "true") )
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("lang", "-none-") ));

			} else if (!strcmp(rqst->pName, "vanish")) {
				const gchar * isOn = UT_getAttribute("w:val", rqst->ppAtts);
				if (isOn == NULL || !strcmp(isOn, "on") || !strcmp(isOn, "1") || !strcmp(isOn, "true") ) {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("display", "none") ));
				} else {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("display", "inline") ));
				}

			} else if (!strcmp(rqst->pName, "sz")) {
				const gchar * szStr = UT_getAttribute("w:val", rqst->ppAtts);
				UT_return_if_fail( this->_error_if_fail(szStr != NULL) );
				double sz = UT_convertDimensionless(szStr) / 2; //TODO: error-check this
				std::string pt_value = UT_convertToDimensionlessString(sz);
				pt_value += "pt";
				UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("font-size", pt_value.c_str()) ));
			}
			rqst->handled = true;
		}

/******* END OF RUN FORMATTING ********/

/******************************
 ****  SECTION FORMATTING  ****
 ******************************/

	} else if (	!strcmp(rqst->pName, "type") ||
				!strcmp(rqst->pName, "footerReference") ||
				!strcmp(rqst->pName, "headerReference")) {
		//Verify the context...
		std::string contextTag = rqst->context->back();
		if (!contextTag.compare("sectPr")) {
			if (!strcmp(rqst->pName, "type")) {
				const gchar * val = UT_getAttribute("w:val", rqst->ppAtts);
				UT_return_if_fail( this->_error_if_fail(val != NULL) );

				OXML_SharedSection last = OXML_Document::getCurrentSection();
				if (!strcmp(val, "continuous")) {
					last->setBreakType(CONTINUOUS_BREAK);
				} else if (!strcmp(val, "evenPage")) {
					last->setBreakType(EVENPAGE_BREAK);
				} else if (!strcmp(val, "oddPage")) {
					last->setBreakType(ODDPAGE_BREAK);
				} else { //nextPage and nextColumn
					last->setBreakType(NEXTPAGE_BREAK);
				}
				rqst->handled = true;

			} else if (!strcmp(rqst->pName, "footerReference")) {
				const gchar * id = UT_getAttribute("r:id", rqst->ppAtts);
				UT_return_if_fail( this->_error_if_fail(id != NULL) );
				OXML_SharedSection last = OXML_Document::getCurrentSection();

				OXMLi_PackageManager * mgr = OXMLi_PackageManager::getInstance();
				UT_return_if_fail( _error_if_fail( UT_OK == mgr->parseDocumentHdrFtr(id) ) );

				OXML_Document * doc = OXML_Document::getInstance();
				UT_return_if_fail(_error_if_fail(doc != NULL));
				const gchar * type = UT_getAttribute("w:type", rqst->ppAtts);
				UT_return_if_fail( this->_error_if_fail(type != NULL) );

				if (!strcmp(type, "default")) {
					last->setFooterId(id, DEFAULT_HDRFTR);
					type = "footer";
				} else if (!strcmp(type, "even")) {
					last->setFooterId(id, EVENPAGE_HDRFTR);
					type = "footer-even";
				} else {
					last->setFooterId(id, FIRSTPAGE_HDRFTR);
					type = "footer-first";
				}

				OXML_SharedSection ftr = doc->getFooter(id);
				UT_return_if_fail(_error_if_fail( UT_OK == ftr->setAttribute("type", type) ));

			} else if (!strcmp(rqst->pName, "headerReference")) {
				const gchar * id = UT_getAttribute("r:id", rqst->ppAtts);
				UT_return_if_fail( this->_error_if_fail(id != NULL) );
				OXML_SharedSection last = OXML_Document::getCurrentSection();

				OXMLi_PackageManager * mgr = OXMLi_PackageManager::getInstance();
				UT_return_if_fail( _error_if_fail( UT_OK == mgr->parseDocumentHdrFtr(id) ) );

				OXML_Document * doc = OXML_Document::getInstance();
				UT_return_if_fail(_error_if_fail(doc != NULL));
				const gchar * type = UT_getAttribute("w:type", rqst->ppAtts);
				UT_return_if_fail( this->_error_if_fail(type != NULL) );

				if (!strcmp(type, "default")) {
					last->setHeaderId(id, DEFAULT_HDRFTR);
					type = "header";
				} else if (!strcmp(type, "even")) {
					last->setHeaderId(id, EVENPAGE_HDRFTR);
					type = "header-even";
				} else {
					last->setHeaderId(id, FIRSTPAGE_HDRFTR);
					type = "header-first";
				}

				OXML_SharedSection hdr = doc->getHeader(id);
				UT_return_if_fail(_error_if_fail( UT_OK == hdr->setAttribute("type", type) ));
			}
		}

/******* END OF SECTION FORMATTING ********/

	} else if (!strcmp(rqst->pName, "tab")) {
		//Verify the context...
		std::string contextTag = rqst->context->back();
		if (!contextTag.compare("r")) {
			//This is an actual tab to be inserted
			OXML_SharedElement tab ( new OXML_Element_Text("\t", 2) );
			rqst->stck->push(tab);

			rqst->handled = true;
		}
		
	} else if (!strcmp(rqst->pName, "br")) {
		const gchar * type = UT_getAttribute("w:type", rqst->ppAtts);
// The optional attribute can be missing. In that case a default 
// value is implied.
//		UT_return_if_fail( this->_error_if_fail(type != NULL) );

		OXML_ElementTag tag;
		if (type && !strcmp(type, "column")) {
			tag = CL_BREAK;
		} else if (type && !strcmp(type, "page")) {
			tag = PG_BREAK;
		} else { //textWrapping
			tag = LN_BREAK;
		}
		OXML_SharedElement br ( new OXML_Element("", tag, SPAN) );
		rqst->stck->push(br);

		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Common::endElement (OXMLi_EndElementRequest * rqst)
{
	UT_return_if_fail( this->_error_if_fail(rqst != NULL) );

	if (!strcmp(rqst->pName, "p")) {
		//Paragraph is done, appending it.
		if (rqst->stck->size() == 1) { //Only the paragraph is on the stack, append to section
			OXML_SharedElement elem = rqst->stck->top();
			UT_return_if_fail( this->_error_if_fail(elem.get() != NULL) );
			OXML_SharedSection sect = OXML_Document::getCurrentSection();
			UT_return_if_fail( this->_error_if_fail(sect.get() != NULL) );
			UT_return_if_fail( this->_error_if_fail(UT_OK == sect->appendElement(elem) ) );
			rqst->stck->pop();
		} else { //Append to next element on the stack
			UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck) ) );
		}

		//Perform the section break if any
		if (m_pendingSectBreak) {
			OXML_Document * doc = OXML_Document::getInstance();
			UT_return_if_fail(_error_if_fail(doc != NULL));
			OXML_SharedSection sect(new OXML_Section());
			UT_return_if_fail(_error_if_fail( UT_OK == doc->appendSection(sect) ));
			m_pendingSectBreak = false;
		}

		rqst->handled = true;
	} else if (!strcmp(rqst->pName, "r")) {
		//Run is done, appending it.
		UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck) ) );

		rqst->handled = true;
	} else if (!strcmp(rqst->pName, "sectPr")) {
		std::string contextTag = rqst->context->back();
		if (!contextTag.compare("pPr") || !contextTag.compare("body")) {
			OXML_SharedSection sect = OXML_Document::getCurrentSection();
			UT_return_if_fail(_error_if_fail(sect.get() != NULL));
			OXML_SharedElement dummy = rqst->stck->top();
			const gchar ** atts = dummy->getAttributes();
			if (atts != NULL) {
				UT_return_if_fail(_error_if_fail(UT_OK == sect->appendAttributes(atts)));
			}
			atts = dummy->getProperties();
			if (atts != NULL) {
				UT_return_if_fail(_error_if_fail(UT_OK == sect->appendProperties(atts)));
			}
			rqst->stck->pop();

			rqst->handled = true;
		}
	} else if (	!strcmp(rqst->pName, "jc") || 
				!strcmp(rqst->pName, "ind") ||
				!strcmp(rqst->pName, "spacing") ) {
		rqst->handled = true;
	} else if (	!strcmp(rqst->pName, "b") || 
				!strcmp(rqst->pName, "i") || 
				!strcmp(rqst->pName, "u") ||
				!strcmp(rqst->pName, "color") ||
				!strcmp(rqst->pName, "highlight") ||
				!strcmp(rqst->pName, "strike") ||
				!strcmp(rqst->pName, "dstrike") ||
				!strcmp(rqst->pName, "rFonts") ||
				!strcmp(rqst->pName, "lang") ||
				!strcmp(rqst->pName, "noProof") ||
				!strcmp(rqst->pName, "vanish") ||
				!strcmp(rqst->pName, "sz") ) {
		rqst->handled = true;
	} else if (	!strcmp(rqst->pName, "type") ||
				!strcmp(rqst->pName, "footerReference") ||
				!strcmp(rqst->pName, "headerReference")) {
		std::string contextTag = rqst->context->back();
		if (!contextTag.compare("sectPr")) {
			rqst->handled = true;
		}
	} else if (!strcmp(rqst->pName, "tab")) {
		std::string contextTag = rqst->context->back();
		if (!contextTag.compare("r")) {
			UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck) ) );
			rqst->handled = true;
		}
	} else if (!strcmp(rqst->pName, "br")) {
		UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck) ) );
		rqst->handled = true;
	}
}

void OXMLi_ListenerState_Common::charData (OXMLi_CharDataRequest * rqst)
{
	UT_return_if_fail( this->_error_if_fail(rqst != NULL) );

	OXML_SharedElement char_data (new OXML_Element_Text(rqst->buffer, rqst->length) );
	UT_return_if_fail( _error_if_fail( !rqst->stck->empty() ));
	OXML_SharedElement elem = rqst->stck->top();
	UT_return_if_fail( this->_error_if_fail(elem.get() != NULL) );
	UT_return_if_fail( this->_error_if_fail( UT_OK == elem->appendElement(char_data) ) );
}

