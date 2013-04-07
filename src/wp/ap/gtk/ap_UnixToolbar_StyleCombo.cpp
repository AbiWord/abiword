/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_vector.h"
#include "ut_locale.h"
#include "ap_UnixToolbar_StyleCombo.h"
#include "ap_Toolbar_Id.h"
#include "xap_Frame.h"
#include "pd_Style.h"
#include "xad_Document.h"
#include "xap_App.h"
#include "ev_UnixToolbar.h"
#include "ut_debugmsg.h"
#include "gr_CairoGraphics.h"

/*****************************************************************/

int sort_cb(gconstpointer a, gconstpointer b)
{
	return strcmp((const gchar*)a, (const gchar*)b);
}

EV_Toolbar_Control * AP_UnixToolbar_StyleCombo::static_constructor(EV_Toolbar * pToolbar,
														  XAP_Toolbar_Id id)
{
	AP_UnixToolbar_StyleCombo * p = new AP_UnixToolbar_StyleCombo(pToolbar,id);
	return p;
}

AP_UnixToolbar_StyleCombo::AP_UnixToolbar_StyleCombo(EV_Toolbar * pToolbar,
													 XAP_Toolbar_Id id)
	: EV_Toolbar_Control(pToolbar/*,id*/), 
	  m_pDefaultDesc(NULL)
{
	UT_DEBUG_ONLY_ARG(id);
	UT_ASSERT(id==AP_TOOLBAR_ID_FMT_STYLE);

	m_nPixels = 120;		// TODO: do a better calculation
	m_nLimit = 15;         // 15 styles before the scroll bar??.
	m_pFrame = static_cast<AP_UnixFrame *>(static_cast<EV_UnixToolbar *>(pToolbar)->getFrame());
}

AP_UnixToolbar_StyleCombo::~AP_UnixToolbar_StyleCombo(void)
{
	freeStyles();
	pango_font_description_free(m_pDefaultDesc);
}

/*****************************************************************/

bool AP_UnixToolbar_StyleCombo::populate(void)
{
	// clear anything that's already there
	m_vecContents.clear();

	// populate the vector

#if 1
	// HACK: for now, just hardwire it
	// NB if you change the case of the labels, it will stop working
	// unless you also change all the places where the style appears!
	m_vecContents.addItem("Normal");
	m_vecContents.addItem("Heading 1");
	m_vecContents.addItem("Heading 2");
	m_vecContents.addItem("Heading 3");
	m_vecContents.addItem("Plain Text");
	m_vecContents.addItem("Block Text");
#else

	AD_Document * pAD_Doc = m_pFrame->getCurrentDoc();
	if(!pAD_Doc)
	{
		return false;
	}
	PD_Document *pDocument = static_cast<PD_Document *>(pAD_Doc);

	// TODO: need a view/doc pointer to get this right
	// ALSO: will need to repopulate as new styles added
	// HYP:  only call this method from shared code? 
	const PD_Style * pStyle;

	UT_GenericVector<PD_Style*> * pStyles = NULL;
	pDocument->enumStyles(pStyles);
	for (UT_uint32 k=0; k < pStyles->getItemCount(); k++)
	{
		pStyle = pStyles->getNthItem(k);
		if (pStyle && pStyle->isDisplayed()) {
			m_vecContents.addItem(pStyle->getName());
		}
	}
	DELETEP(pStyles);
#endif 

	return true;
}


bool AP_UnixToolbar_StyleCombo::repopulate(void)
{
	// repopulate the vector from the current document
    // If ithere is one present

	AD_Document * pAD_Doc = m_pFrame->getCurrentDoc();
	if(!pAD_Doc)
	{
		return false;
	}

	PD_Document *pDocument = static_cast<PD_Document *>(pAD_Doc);

	GR_GraphicsFactory * pGF = XAP_App::getApp()->getGraphicsFactory();
	if(!pGF)
	{
		return false;
	}

	// clear anything that's already there
	m_vecContents.clear();
	freeStyles();

	// defaults for style combo
	if (m_pDefaultDesc == NULL)
	{
		// for now this is hardcoded
		m_pDefaultDesc = pango_font_description_new ();
		pango_font_description_set_family (m_pDefaultDesc, "Times");
		pango_font_description_set_size (m_pDefaultDesc, 12 * PANGO_SCALE);
	}

	const PD_Style * pStyle;
	GSList *list = NULL;

	UT_GenericVector<PD_Style*> *pStyles = NULL;
	pDocument->enumStyles(pStyles);
	for (UT_sint32 k=0; k < pStyles->getItemCount(); k++)
	{
		pStyle = pStyles->getNthItem(k);

		if(!pStyle) {
			continue;
		}
		if (!pStyle->isDisplayed() && 
		    !(dynamic_cast<const PD_BuiltinStyle *>(pStyle) && pStyle->isList() && pStyle->isUsed())) {
			continue;
		}

		list = g_slist_prepend (list, (gpointer)pStyle->getName());

		/* wysiwyg styles are disabled for now 
		   // also test before enabling
		PangoFontDescription *desc = pango_font_description_copy (m_pDefaultDesc);
		getPangoAttrs(pStyle, desc);
		m_mapStyles.insert(std::make_pair(szName, desc));
		*/
	}
	DELETEP(pStyles);

	// Ok, it's a bit hackish to put them in a list for sorting first
	// but somehow the vector's qsort totally failed for me
	if (list) 
	{
		list = g_slist_sort(list, (GCompareFunc)sort_cb);		
		GSList * real_list = list;
		do 
		{
			m_vecContents.addItem((const char *)list->data);

		} while (NULL != (list = g_slist_next(list)));
		g_slist_free(real_list);
	}		

	return true;
}

const PangoFontDescription* 
AP_UnixToolbar_StyleCombo::getStyle (const gchar *name)
{
	std::map<std::string, PangoFontDescription*>::const_iterator iter = m_mapStyles.find(name);
	if(iter == m_mapStyles.end()) {
		repopulate();
		iter = m_mapStyles.find(name);
	}
	if(iter != m_mapStyles.end()) {
		return iter->second;
	}
	return NULL;
}

/*!
* \todo ROB parse more attributes like font-color, background-color
*/
void 		 
AP_UnixToolbar_StyleCombo::getPangoAttrs (PD_Style *pStyle, 
										  PangoFontDescription *desc) {

	UT_return_if_fail (pStyle);
	UT_LocaleTransactor t (LC_NUMERIC, "C");

	const gchar *value = NULL;

	if (pStyle->getPropertyExpand ("font-family", value)) {
		pango_font_description_set_family (desc, value);
	}

	if (pStyle->getPropertyExpand ("font-size", value)) {
		pango_font_description_set_size (desc, (gint)(UT_convertToDimension (value, DIM_PT) * PANGO_SCALE));
	}

	if (pStyle->getPropertyExpand ("font-style", value)) {
		PangoStyle style = PANGO_STYLE_NORMAL;
		if (!strcmp (value, "italic"))
			style = PANGO_STYLE_ITALIC;
		pango_font_description_set_style (desc, style);
	}

	if (pStyle->getPropertyExpand ("font-weight", value)) {
		PangoWeight weight = PANGO_WEIGHT_NORMAL;
		if (!strcmp (value, "bold"))
			weight = PANGO_WEIGHT_BOLD;
		pango_font_description_set_weight (desc, weight);
	}
}

void 
AP_UnixToolbar_StyleCombo::freeStyles()
{
	std::map<std::string, PangoFontDescription*>::iterator iter = m_mapStyles.begin();
	for( ; iter != m_mapStyles.end(); ++iter) {
		pango_font_description_free(iter->second);
	}
}
