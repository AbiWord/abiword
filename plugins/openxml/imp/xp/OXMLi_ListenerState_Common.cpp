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

	if(nameMatches(rqst->pName, NS_W_KEY, "p")) {
		//New paragraph...
		OXML_SharedElement elem(new OXML_Element_Paragraph(""));
		rqst->stck->push(elem);

		rqst->handled = true;
	} else if (nameMatches(rqst->pName, NS_W_KEY, "r")) {
		//New text run...
		OXML_SharedElement elem(new OXML_Element_Run(""));
		rqst->stck->push(elem);

		rqst->handled = true;

	} else if (nameMatches(rqst->pName, NS_W_KEY, "t")) {
		//New text...
		OXML_SharedElement elem(new OXML_Element_Text("", 0));
		rqst->stck->push(elem);
		rqst->handled = true;

	} else if (nameMatches(rqst->pName, NS_W_KEY, "sectPr")) {
		//Verify the context...
		std::string contextTag = rqst->context->back();
		if (contextMatches(contextTag, NS_W_KEY, "pPr") || 
			contextMatches(contextTag, NS_W_KEY, "body")) {
			OXML_SharedElement dummy(new OXML_Element_Paragraph(""));
			rqst->stck->push(dummy);

			m_pendingSectBreak = true;
			rqst->handled = true;
		}

/********************************
 ****  PARAGRAPH FORMATTING  ****
 ********************************/
	} else if(nameMatches(rqst->pName, NS_W_KEY, "shd")) {
		std::string contextTag = rqst->context->back();

		if(!contextMatches(contextTag, NS_W_KEY, "pPr") && 
			!contextMatches(contextTag, NS_W_KEY, "rPr"))
			return;

		const gchar* fill = attrMatches(NS_W_KEY, "fill", rqst->ppAtts);

		OXML_SharedElement elem = rqst->stck->top();

		if(fill && strcmp(fill, "auto")) 
		{
			UT_Error err = UT_OK;
			err = elem->setProperty("bgcolor", fill);
			if(err != UT_OK)
				UT_DEBUGMSG(("FRT:OpenXML importer can't set background-color:%s\n", fill));	
		}
		rqst->handled = true;

	} else if ( nameMatches(rqst->pName, NS_W_KEY, "pageBreakBefore")){
		//verify the context
		std::string contextTag = rqst->context->back();

		if (contextMatches(contextTag, NS_W_KEY, "pPr")) {
			OXML_SharedElement elem = rqst->stck->top();
			if(elem->getTag() == P_TAG)
			{
				OXML_Element_Paragraph* para = static_cast<OXML_Element_Paragraph*>(get_pointer(elem)); 
				para->setPageBreak();
			}
			rqst->handled = true;
		}
	
	} else if ( nameMatches(rqst->pName, NS_W_KEY, "tab")){
		//verify the context
		std::string contextTag = rqst->context->back();

		if (contextMatches(contextTag, NS_W_KEY, "r")) {
			//This is an actual tab to be inserted
			OXML_SharedElement tab ( new OXML_Element_Text("\t", 2) );
			rqst->stck->push(tab);
			rqst->handled = true;
		}
		else if(contextMatches(contextTag, NS_W_KEY, "tabs")){
			OXML_SharedElement para = rqst->stck->top();
			const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
			const gchar* pos = attrMatches(NS_W_KEY, "pos", rqst->ppAtts);
			const gchar* leadCh = attrMatches(NS_W_KEY, "leader", rqst->ppAtts);
			if(!val || !*val || !pos || !*pos)
				return;

			std::string value(val);
			std::string position(_TwipsToInches(pos));
			position += "in";
			std::string leader("0"); //no leader by default
			
			std::string tabstops("");
			const gchar* tabProp = NULL;
			para->getProperty("tabstops", tabProp);	
			if(tabProp)
			{
				tabstops = tabProp;
				tabstops += ",";
			}

			tabstops += position;
			tabstops += "/";

			if(!value.compare("left"))
				tabstops += "L";
			else if(!value.compare("right"))
				tabstops += "R";
			else if(!value.compare("center"))
				tabstops += "C";
			else if(!value.compare("bar"))
				tabstops += "B";
			else if(!value.compare("decimal"))
				tabstops += "D";

			if(leadCh)
			{
				std::string leaderChar(leadCh);
				if(!leaderChar.compare("dot"))
					leader = "1";
				else if(!leaderChar.compare("heavy"))
					leader = "3";
				else if(!leaderChar.compare("hyphen"))
					leader = "2";
				else if(!leaderChar.compare("middleDot"))
					leader = "1";
				else if(!leaderChar.compare("underscore"))
					leader = "3";
			}
			tabstops += leader;
				
			para->setProperty("tabstops", tabstops);						
		}		
		rqst->handled = true;
	} else if ( nameMatches(rqst->pName, NS_W_KEY, "ilvl")){
		//verify the context
		std::string contextTag = rqst->context->back();
		if(contextMatches(contextTag, NS_W_KEY, "numPr")){
			OXML_SharedElement para = rqst->stck->top();
			const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
			if(!val || !*val)
				return;
			std::string level(val);
			para->setAttribute("level", level.c_str());	
		}		
		rqst->handled = true;
	} else if ( nameMatches(rqst->pName, NS_W_KEY, "numId")){
		//verify the context
		std::string contextTag = rqst->context->back();
		if(contextMatches(contextTag, NS_W_KEY, "numPr")){
			OXML_SharedElement para = rqst->stck->top();
			const gchar* val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
			if(!val || !*val)
				return;
			std::string numId(val);

			OXML_Document* doc = OXML_Document::getInstance();
			std::string absNumId = doc->getMappedNumberingId(numId);
			if(!absNumId.empty())
				para->setAttribute("listid", absNumId.c_str());					
		}		
		rqst->handled = true;
	} else if ( nameMatches(rqst->pName, NS_W_KEY, "jc") ||
				nameMatches(rqst->pName, NS_W_KEY, "ind") ||
				nameMatches(rqst->pName, NS_W_KEY, "spacing") ||
				nameMatches(rqst->pName, NS_W_KEY, "pStyle")) {
	//Verify the context...
	std::string contextTag = rqst->context->at(rqst->context->size() - 2);
	if (contextMatches(contextTag, NS_W_KEY, "p") ||
		contextMatches(contextTag, NS_W_KEY, "pPrDefault") ||
		contextMatches(contextTag, NS_W_KEY, "lvl") ||  
		contextMatches(contextTag, NS_W_KEY, "style")) { 

		OXML_SharedElement para = rqst->stck->top();

		if (nameMatches(rqst->pName, NS_W_KEY, "jc")) {
			const gchar * val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);

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

		} else if (nameMatches(rqst->pName, NS_W_KEY, "ind")) {
			const gchar * left = attrMatches(NS_W_KEY, "left", rqst->ppAtts);
			const gchar * right = attrMatches(NS_W_KEY, "right", rqst->ppAtts);
			const gchar * fLine = attrMatches(NS_W_KEY, "firstLine", rqst->ppAtts);
			const gchar * hanging = attrMatches(NS_W_KEY, "hanging", rqst->ppAtts);

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

		} else if (nameMatches(rqst->pName, NS_W_KEY, "spacing")) {
			const gchar * before = attrMatches(NS_W_KEY, "before", rqst->ppAtts);
			const gchar * after = attrMatches(NS_W_KEY, "after", rqst->ppAtts);
			const gchar * lineRule = attrMatches(NS_W_KEY, "lineRule", rqst->ppAtts);

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
				const gchar * line = attrMatches(NS_W_KEY, "line", rqst->ppAtts);
				UT_return_if_fail( _error_if_fail(line != NULL) );
				double ln_spc = UT_convertDimensionless(line) / 240;
				final = UT_convertToDimensionlessString(ln_spc);
				UT_return_if_fail( _error_if_fail( UT_OK == para->setProperty("line-height", final.c_str()) ));
			}
		} else if (nameMatches(rqst->pName, NS_W_KEY, "pStyle")) {
			const gchar * val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
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
	} else if (	nameMatches(rqst->pName, NS_W_KEY, "b") || 
				nameMatches(rqst->pName, NS_W_KEY, "i") || 
				nameMatches(rqst->pName, NS_W_KEY, "u") ||
				nameMatches(rqst->pName, NS_W_KEY, "color") ||
				nameMatches(rqst->pName, NS_W_KEY, "vertAlign") || // for subscript and superscript
				nameMatches(rqst->pName, NS_W_KEY, "highlight") ||
				nameMatches(rqst->pName, NS_W_KEY, "strike") ||
				nameMatches(rqst->pName, NS_W_KEY, "dstrike") ||
				nameMatches(rqst->pName, NS_W_KEY, "rFonts") ||
				nameMatches(rqst->pName, NS_W_KEY, "lang") ||
				nameMatches(rqst->pName, NS_W_KEY, "noProof") ||
				nameMatches(rqst->pName, NS_W_KEY, "vanish") ||
				nameMatches(rqst->pName, NS_W_KEY, "sz") ) {
		//Verify the context...
		std::string contextTag = rqst->context->at(rqst->context->size() - 2);
		if (contextMatches(contextTag, NS_W_KEY, "r") ||
			contextMatches(contextTag, NS_W_KEY, "rPrDefault") || 
			contextMatches(contextTag, NS_W_KEY, "style")) {
			OXML_SharedElement run = rqst->stck->top();

			if (nameMatches(rqst->pName, NS_W_KEY, "b")) {
				const gchar * isOn = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
				if (isOn == NULL || !strcmp(isOn, "on") || !strcmp(isOn, "1") || !strcmp(isOn, "true") ) {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("font-weight", "bold") ));
				} else {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("font-weight", "normal") ));
				}

			} else if (nameMatches(rqst->pName, NS_W_KEY, "i")) {
				const gchar * isOn = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
				if (isOn == NULL || !strcmp(isOn, "on") || !strcmp(isOn, "1") || !strcmp(isOn, "true") ) {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("font-style", "italic") ));
				} else {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("font-style", "normal") ));
				}

			} else if (nameMatches(rqst->pName, NS_W_KEY, "vertAlign")) {
				const gchar * val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
				if (val == NULL || !*val || !strcmp(val, "baseline")) {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("text-position", "normal") ));
				} else if (!strcmp(val, "superscript")) {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("text-position", "superscript") ));
				} else if (!strcmp(val, "subscript")) {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("text-position", "subscript") ));
				}
				
			} else if (nameMatches(rqst->pName, NS_W_KEY, "u")) {
				const gchar * newVal = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
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

			} else if (nameMatches(rqst->pName, NS_W_KEY, "color")) {
				const gchar * val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
				if (val != NULL) {
					if (!strcmp(val, "auto")) val = "#000000";
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("color", val)));
				} else {
					val = attrMatches(NS_W_KEY, "themeColor", rqst->ppAtts);
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

			} else if (nameMatches(rqst->pName, NS_W_KEY, "highlight")) {
				const gchar * val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
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

			} else if (nameMatches(rqst->pName, NS_W_KEY, "strike") || 
					   nameMatches(rqst->pName, NS_W_KEY, "dstrike")) {
				const gchar * isOn = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
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

			} else if (nameMatches(rqst->pName, NS_W_KEY, "rFonts")) {
				OXML_Document * doc = OXML_Document::getInstance();
				UT_return_if_fail( this->_error_if_fail(doc != NULL) );
				OXML_SharedFontManager fmgr = doc->getFontManager();
				UT_return_if_fail( this->_error_if_fail(fmgr.get() != NULL) );

				std::string fontName;
				OXML_FontLevel level = UNKNOWN_LEVEL;
				OXML_CharRange range = UNKNOWN_RANGE;

				const gchar * ascii = NULL; //TODO: add support for eastAsia, bidi, and hAnsi
				if (NULL != (ascii = attrMatches(NS_W_KEY, "asciiTheme", rqst->ppAtts))) {
					this->getFontLevelRange(ascii, level, range);
					fontName = fmgr->getValidFont(level, range); //Retrieve valid font name from Theme
				} else if (NULL != (ascii = attrMatches(NS_W_KEY, "ascii", rqst->ppAtts))) {
					fontName = ascii;
					fontName = fmgr->getValidFont(fontName); //Make sure the name is valid
				} else {
					fontName = fmgr->getDefaultFont();
				}
				UT_return_if_fail( _error_if_fail( UT_OK == run->setProperty("font-family", fontName.c_str()) ));

			} else if (nameMatches(rqst->pName, NS_W_KEY, "lang")) {
				const gchar * val = attrMatches(NS_W_KEY, "val", rqst->ppAtts); //TODO: add support for eastAsia and bidi attributes
				const gchar * previousVal = NULL;
				if (UT_OK == run->getProperty("lang", previousVal)) {
					if ( 0 != strcmp(previousVal, "-none-"))
						val = previousVal;
				}
				if ( val != NULL)
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("lang", val) ));

			} else if (nameMatches(rqst->pName, NS_W_KEY, "noProof")) {
				//noProof has priority over lang, so no need to check for previous values
				const gchar * isOn = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
				if (isOn == NULL || !strcmp(isOn, "on") || !strcmp(isOn, "1") || !strcmp(isOn, "true") )
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("lang", "-none-") ));

			} else if (nameMatches(rqst->pName, NS_W_KEY, "vanish")) {
				const gchar * isOn = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
				if (isOn == NULL || !strcmp(isOn, "on") || !strcmp(isOn, "1") || !strcmp(isOn, "true") ) {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("display", "none") ));
				} else {
					UT_return_if_fail( this->_error_if_fail( UT_OK == run->setProperty("display", "inline") ));
				}

			} else if (nameMatches(rqst->pName, NS_W_KEY, "sz")) {
				const gchar * szStr = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
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

	} else if (	nameMatches(rqst->pName, NS_W_KEY, "type") ||
				nameMatches(rqst->pName, NS_W_KEY, "footerReference") ||
				nameMatches(rqst->pName, NS_W_KEY, "headerReference") ||
				nameMatches(rqst->pName, NS_W_KEY, "cols")) {
		//Verify the context...
		std::string contextTag = rqst->context->back();
		if (contextMatches(contextTag, NS_W_KEY, "sectPr")) {
			if (nameMatches(rqst->pName, NS_W_KEY, "type")) {
				const gchar * val = attrMatches(NS_W_KEY, "val", rqst->ppAtts);
				UT_return_if_fail( this->_error_if_fail(val != NULL) );

				OXML_SharedSection last = rqst->sect_stck->top();
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

			} else if (nameMatches(rqst->pName, NS_W_KEY, "footerReference")) {
				const gchar * id = attrMatches(NS_R_KEY, "id", rqst->ppAtts);
				UT_return_if_fail( this->_error_if_fail(id != NULL) );
				OXML_SharedSection last = rqst->sect_stck->top();

				OXMLi_PackageManager * mgr = OXMLi_PackageManager::getInstance();
				UT_return_if_fail( _error_if_fail( UT_OK == mgr->parseDocumentHdrFtr(id) ) );

				OXML_Document * doc = OXML_Document::getInstance();
				UT_return_if_fail(_error_if_fail(doc != NULL));
				const gchar * type = attrMatches(NS_W_KEY, "type", rqst->ppAtts);
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

			} else if (nameMatches(rqst->pName, NS_W_KEY, "headerReference")) {
				const gchar * id = attrMatches(NS_R_KEY, "id", rqst->ppAtts);
				UT_return_if_fail( this->_error_if_fail(id != NULL) );
				OXML_SharedSection last = rqst->sect_stck->top();

				OXMLi_PackageManager * mgr = OXMLi_PackageManager::getInstance();
				UT_return_if_fail( _error_if_fail( UT_OK == mgr->parseDocumentHdrFtr(id) ) );

				OXML_Document * doc = OXML_Document::getInstance();
				UT_return_if_fail(_error_if_fail(doc != NULL));
				const gchar * type = attrMatches(NS_W_KEY, "type", rqst->ppAtts);
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
			else if (nameMatches(rqst->pName, NS_W_KEY, "cols")) {
				const gchar * num = attrMatches(NS_W_KEY, "num", rqst->ppAtts);
				const gchar * sep = attrMatches(NS_W_KEY, "sep", rqst->ppAtts);

				if(!num || atoi(num)<1)
					num = "1";

				if(!sep)
					sep = "off";
				
				OXML_SharedSection last = rqst->sect_stck->top();
				last->setProperty("columns", num);
				last->setProperty("column-line", sep);
			}
		}

	} else if (nameMatches(rqst->pName, NS_W_KEY, "footnoteReference")) {
		const gchar * id = attrMatches(NS_W_KEY, "id", rqst->ppAtts);
		if(id)
		{
			OXML_SharedElement footnote(new OXML_Element_Field(id, fd_Field::FD_Footnote_Ref, ""));
			rqst->stck->push(footnote);
		}
		rqst->handled = true;

	} else if (nameMatches(rqst->pName, NS_W_KEY, "endnoteReference")) {
		const gchar * id = attrMatches(NS_W_KEY, "id", rqst->ppAtts);
		if(id)
		{
			OXML_SharedElement endnote(new OXML_Element_Field(id, fd_Field::FD_Endnote_Ref, NULL));
			rqst->stck->push(endnote);
		}
		rqst->handled = true;		

	} else if (nameMatches(rqst->pName, NS_W_KEY, "hyperlink")) {
		const gchar * id = attrMatches(NS_R_KEY, "id", rqst->ppAtts);
		const gchar * anchor = attrMatches(NS_W_KEY, "anchor", rqst->ppAtts);
		if(id)
		{
			OXMLi_PackageManager * mgr = OXMLi_PackageManager::getInstance();
			std::string target = mgr->getPartName(id);
			OXML_Element_Hyperlink* hyperlink = new OXML_Element_Hyperlink("");
			hyperlink->setHyperlinkTarget(target);				
			OXML_SharedElement elem(hyperlink);
			rqst->stck->push(elem);
		}
		else if(anchor)
		{
			std::string bookmarkAnchor("#");
			bookmarkAnchor += anchor;
			OXML_Element_Hyperlink* hyperlink = new OXML_Element_Hyperlink("");
			hyperlink->setHyperlinkTarget(bookmarkAnchor);				
			OXML_SharedElement elem(hyperlink);
			rqst->stck->push(elem);
		}
		rqst->handled = true;

	} else if (nameMatches(rqst->pName, NS_W_KEY, "bookmarkStart")) {
		const gchar * id = attrMatches(NS_W_KEY, "id", rqst->ppAtts);
		const gchar * name = attrMatches(NS_W_KEY, "name", rqst->ppAtts);
		if(id && name)
		{
			std::string bookmarkId(id);
			std::string bookmarkName(name);
			OXML_Element_Bookmark* bookmark = new OXML_Element_Bookmark(bookmarkId);
			bookmark->setType("start");		
			bookmark->setName(bookmarkName);		
			OXML_SharedElement elem(bookmark);
			rqst->stck->push(elem);
			OXML_Document* pDoc = OXML_Document::getInstance();
			if(!pDoc->setBookmarkName(bookmarkId, bookmarkName))
				return;
		}
		rqst->handled = true;

	} else if (nameMatches(rqst->pName, NS_W_KEY, "bookmarkEnd")) {
		const gchar * id = attrMatches(NS_W_KEY, "id", rqst->ppAtts);
		if(id)
		{
			std::string bookmarkId(id);
			OXML_Element_Bookmark* bookmark = new OXML_Element_Bookmark(bookmarkId);
			bookmark->setType("end");				
			OXML_Document* pDoc = OXML_Document::getInstance();
			bookmark->setName(pDoc->getBookmarkName(bookmarkId));
			OXML_SharedElement elem(bookmark);
			rqst->stck->push(elem);
		}
		rqst->handled = true;

/******* END OF SECTION FORMATTING ********/
		
	} else if (nameMatches(rqst->pName, NS_W_KEY, "br")) {
		const gchar * type = attrMatches(NS_W_KEY, "type", rqst->ppAtts);
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

	if (nameMatches(rqst->pName, NS_W_KEY, "p")) {
		//Paragraph is done, appending it.
		if (rqst->stck->size() == 1) { //Only the paragraph is on the stack, append to section
			OXML_SharedElement elem = rqst->stck->top();
			UT_return_if_fail( this->_error_if_fail(elem.get() != NULL) );
			OXML_SharedSection sect = rqst->sect_stck->top();
			UT_return_if_fail( this->_error_if_fail(sect.get() != NULL) );
			UT_return_if_fail( this->_error_if_fail(UT_OK == sect->appendElement(elem) ) );
			rqst->stck->pop();
		} else { //Append to next element on the stack
			UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck, rqst->sect_stck) ) );
		}

		//Perform the section break if any
		if (m_pendingSectBreak) {
			OXML_Document * doc = OXML_Document::getInstance();
			UT_return_if_fail(_error_if_fail(doc != NULL));
			OXML_SharedSection sect(new OXML_Section());

			rqst->sect_stck->push(sect);
			m_pendingSectBreak = false;
		}

		rqst->handled = true;
	} else if (nameMatches(rqst->pName, NS_W_KEY, "r")) {
		//Run is done, appending it.
		UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck, rqst->sect_stck) ) );

		rqst->handled = true;
	} else if (nameMatches(rqst->pName, NS_W_KEY, "t")) {
		//Text is done, appending it.
		UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck, rqst->sect_stck) ) );
		rqst->handled = true;
	} else if (nameMatches(rqst->pName, NS_W_KEY, "sectPr")) {
		std::string contextTag = rqst->context->back();
		if (contextMatches(contextTag, NS_W_KEY, "pPr") ||
			contextMatches(contextTag, NS_W_KEY, "body")) {
			OXML_SharedSection sect = rqst->sect_stck->top();
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
	} else if (	nameMatches(rqst->pName, NS_W_KEY, "jc") || 
				nameMatches(rqst->pName, NS_W_KEY, "ind") ||
				nameMatches(rqst->pName, NS_W_KEY, "spacing") ) {
		rqst->handled = true;
	} else if (	nameMatches(rqst->pName, NS_W_KEY, "b") || 
				nameMatches(rqst->pName, NS_W_KEY, "i") || 
				nameMatches(rqst->pName, NS_W_KEY, "u") ||
				nameMatches(rqst->pName, NS_W_KEY, "color") ||
				nameMatches(rqst->pName, NS_W_KEY, "vertAlign") ||
				nameMatches(rqst->pName, NS_W_KEY, "highlight") ||
				nameMatches(rqst->pName, NS_W_KEY, "strike") ||
				nameMatches(rqst->pName, NS_W_KEY, "dstrike") ||
				nameMatches(rqst->pName, NS_W_KEY, "rFonts") ||
				nameMatches(rqst->pName, NS_W_KEY, "lang") ||
				nameMatches(rqst->pName, NS_W_KEY, "noProof") ||
				nameMatches(rqst->pName, NS_W_KEY, "vanish") ||
				nameMatches(rqst->pName, NS_W_KEY, "sz") ) {
		rqst->handled = true;
	} else if (	nameMatches(rqst->pName, NS_W_KEY, "type") ||
				nameMatches(rqst->pName, NS_W_KEY, "footerReference") ||
				nameMatches(rqst->pName, NS_W_KEY, "headerReference") ||
				nameMatches(rqst->pName, NS_W_KEY, "cols")) {
		std::string contextTag = rqst->context->back();
		if (contextMatches(contextTag, NS_W_KEY, "sectPr")) {
			rqst->handled = true;
		}
	} else if (nameMatches(rqst->pName, NS_W_KEY, "tab")) {
		std::string contextTag = rqst->context->back();
		if (contextMatches(contextTag, NS_W_KEY, "r")) {
			UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck, rqst->sect_stck) ) );
			rqst->handled = true;
		}
		else if(contextMatches(contextTag, NS_W_KEY, "tabs"))
			rqst->handled = true;
	} else if (nameMatches(rqst->pName, NS_W_KEY, "br")) {
		UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck, rqst->sect_stck) ) );
		rqst->handled = true;
	} else if (nameMatches(rqst->pName, NS_W_KEY, "footnoteReference") || 
			   nameMatches(rqst->pName, NS_W_KEY, "endnoteReference")) {
		UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck, rqst->sect_stck) ) );
		rqst->handled = true;
	} else if (nameMatches(rqst->pName, NS_W_KEY, "hyperlink")) {
		UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck, rqst->sect_stck) ) );
		rqst->handled = true;
	} else if (nameMatches(rqst->pName, NS_W_KEY, "bookmarkStart") ||
				nameMatches(rqst->pName, NS_W_KEY, "bookmarkEnd")) {
		UT_return_if_fail( this->_error_if_fail( UT_OK == _flushTopLevel(rqst->stck, rqst->sect_stck) ) );
		rqst->handled = true;		
	} else if (nameMatches(rqst->pName, NS_W_KEY, "pageBreakBefore")) {
		rqst->handled = contextMatches(rqst->context->back(), NS_W_KEY, "pPr");		
	} else if (nameMatches(rqst->pName, NS_W_KEY, "shd")) {
		std::string contextTag = rqst->context->back();
		rqst->handled = contextMatches(contextTag, NS_W_KEY, "pPr") || contextMatches(contextTag, NS_W_KEY, "rPr");
	}
}

void OXMLi_ListenerState_Common::charData (OXMLi_CharDataRequest * rqst)
{
	if(!rqst)
	{
		UT_DEBUGMSG(("FRT: OpenXML importer invalid NULL request in OXMLi_ListenerState_Common.charData\n"));
		return;
	}
	
	if(rqst->stck->empty())
		return;

	OXML_SharedElement sharedElem = rqst->stck->top();
	OXML_Element* elem = sharedElem.get();

	if(!elem || (elem->getTag() != T_TAG))
		return;

	OXML_Element_Text* textElement = static_cast<OXML_Element_Text*>(elem);
	textElement->setText(rqst->buffer, rqst->length);
}
