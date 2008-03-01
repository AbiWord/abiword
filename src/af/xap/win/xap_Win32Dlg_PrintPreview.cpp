/* AbiSource Application Framework
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gstdio.h>
#include <io.h>

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Dialog_Id.h"
#include "xap_Win32Dlg_PrintPreview.h"
#include "xap_Frame.h"
#include "xap_App.h"
#include "xap_DialogFactory.h"
#include "gr_Win32Graphics.h"
#include "ut_Win32OS.h"

#include "fv_View.h"

XAP_Win32Dialog_PrintPreview::XAP_Win32Dialog_PrintPreview(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_PrintPreview(pDlgFactory,id), m_pPrintGraphics(0), m_emfFilename(0)
{
}

XAP_Win32Dialog_PrintPreview::~XAP_Win32Dialog_PrintPreview(void)
{
  DELETEP(m_pPrintGraphics);
  g_free(m_emfFilename);
}

XAP_Dialog * XAP_Win32Dialog_PrintPreview::static_constructor(XAP_DialogFactory * pFactory,
							      XAP_Dialog_Id id)
{
  return new XAP_Win32Dialog_PrintPreview (pFactory,id);
}

GR_Graphics * XAP_Win32Dialog_PrintPreview::getPrinterGraphicsContext(void)
{
  return m_pPrintGraphics;
}

void XAP_Win32Dialog_PrintPreview::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
  HDC dc;

  if (!m_pPrintGraphics) return;

  dc = ((GR_Win32Graphics *)m_pPrintGraphics)->getPrimaryDC();

  if (dc != NULL)
    {
      HENHMETAFILE metafile;
      
      metafile = CloseEnhMetaFile (dc);
      DeleteEnhMetaFile (metafile);
      
      ShellExecuteW (NULL, L"open", (WCHAR *)m_emfFilename, NULL, NULL, SW_SHOW);
    }

  DELETEP(m_pPrintGraphics);

  if (m_emfFilename)
    {
      // don't remove m_emfFilename, because the preview application is probably still using the file
      g_free(m_emfFilename);
      m_emfFilename = 0;
    }
}

void XAP_Win32Dialog_PrintPreview::runModal(XAP_Frame * pFrame)
{
  HDC metafile_dc;
  RECT rect;
  char *filename;
  int fd;

  FV_View * pView = static_cast<FV_View*>(pFrame->getCurrentView());

  filename = g_build_filename (g_get_tmp_dir (), "prXXXXXX.emf", NULL);
  fd = g_mkstemp (filename);
  close(fd);

  m_emfFilename = g_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);

  rect.left = 0;
  rect.right = (LONG)(pView->getPageSize().Width (DIM_MM) * 100);
  rect.top = 0;
  rect.bottom = (LONG)(pView->getPageSize().Height (DIM_MM) * 100);

  metafile_dc = CreateEnhMetaFileW (NULL, (WCHAR*)m_emfFilename,
				    &rect, L"AbiWord\0Print Preview\0\0");

  if (metafile_dc == NULL)
    {
      g_remove (filename);
      g_free (m_emfFilename);
      m_emfFilename = 0;
      UT_DEBUGMSG(("Can't create metafile"));
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }
  else
    {
      DOCINFO *di;

      di = (DOCINFO*) UT_calloc(1, sizeof(DOCINFO));
      memset( di, 0, sizeof(DOCINFO) );
      di->cbSize = sizeof(DOCINFO);
      di->lpszDocName = (LPCTSTR)TEXT("AbiWord Print Preview");
      di->lpszOutput = (LPTSTR) NULL;
      di->lpszDatatype = (LPTSTR) NULL;
      di->fwType = 0;

      GR_Win32AllocInfo ai(metafile_dc, di, NULL);
      m_pPrintGraphics = XAP_App::getApp()->newGraphics(ai);
      UT_ASSERT(m_pPrintGraphics);
    }

  g_free(filename);
}
