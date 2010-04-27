/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "ie_imp.h"
#include "ie_exp.h"
#include "ie_impexp_Register.h"

#include "ie_exp_AbiWord_1.h"
#include "ie_exp_AWT.h"
#include "ie_exp_RTF.h"
#include "ie_exp_Text.h"
#include "ie_exp_HTML.h"

#include "ie_imp_AbiWord_1.h"
#include "ie_imp_MsWord_97.h"
#include "ie_imp_RTF.h"
#include "ie_imp_Text.h"
#include "ie_imp_XHTML.h"
#include "ie_imp_GraphicAsDocument.h"

/* graphics */
#include "ie_impGraphic.h"
#include "ie_impGraphic_PNG.h"
#include "ie_impGraphic_SVG.h"

#include "ie_mailmerge.h"

void IE_ImpExp_UnRegisterXP ()
{
  IE_ImpGraphic::unregisterAllImporters ();
  IE_Exp::unregisterAllExporters ();
  IE_Imp::unregisterAllImporters ();

  IE_MailMerge_UnRegisterXP ();
}

/*!
  Register all XP Importer and Exporter
  Should be called from AP_<FE>App
 */
void IE_ImpExp_RegisterXP ()
{
  /* graphical types first */

  IE_ImpGraphic::registerImporter(new IE_ImpGraphicPNG_Sniffer ());
  IE_ImpGraphic::registerImporter(new IE_ImpGraphicSVG_Sniffer ());

  /* now text-file types */

	IE_Imp::registerImporter(new IE_Imp_AbiWord_1_Sniffer ());
	IE_Imp::registerImporter(new IE_Imp_MsWord_97_Sniffer ());
	IE_Imp::registerImporter(new IE_Imp_RTF_Sniffer ());
	IE_Imp::registerImporter(new IE_Imp_Text_Sniffer ());
	IE_Imp::registerImporter(new IE_Imp_EncodedText_Sniffer ());
	IE_Imp::registerImporter(new IE_Imp_XHTML_Sniffer ());
	
	IE_Exp::registerExporter(new IE_Exp_AbiWord_1_Sniffer ());
	IE_Exp::registerExporter(new IE_Exp_AWT_Sniffer());

	// HACK - export RTF and claim it's DOC
	IE_Exp::registerExporter(new IE_Exp_MsWord_Hack_Sniffer ());

#ifdef HTML_ENABLE_HTML4
	IE_Exp::registerExporter(new IE_Exp_HTML4_Sniffer ());
#endif
	IE_Exp::registerExporter(new IE_Exp_HTML_Sniffer ());
#ifdef HTML_ENABLE_PHTML
	IE_Exp::registerExporter(new IE_Exp_PHTML_Sniffer ());
#endif
#ifdef HTML_ENABLE_MHTML
	IE_Exp::registerExporter(new IE_Exp_MHTML_Sniffer ());
#endif
	IE_Exp::registerExporter(new IE_Exp_RTF_Sniffer ());
	IE_Exp::registerExporter(new IE_Exp_RTF_attic_Sniffer ());
	IE_Exp::registerExporter(new IE_Exp_Text_Sniffer ());
	IE_Exp::registerExporter(new IE_Exp_EncodedText_Sniffer ());

	/* Register platform specific. */
	IE_ImpExp_RegisterPlatform ();

	IE_MailMerge_RegisterXP ();
}
    
