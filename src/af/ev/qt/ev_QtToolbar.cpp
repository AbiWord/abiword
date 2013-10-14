/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-2006 Tomas Frydrych <dr.tomas@yahoo.co.uk>
 * Copyright (C) 2009 Hubert Figuiere
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

#include <QPushButton>
#include <QToolButton>
#include <QString>
#include <QComboBox>
#include <QFontComboBox>
#include <QMainWindow>
#include <QPixmap>

#include <string.h>
#include <stdlib.h>
#include <string>

#include "ap_Features.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ev_QtToolbar.h"
#include "xap_Types.h"
#include "xap_QtApp.h"
#include "xap_Frame.h"
#include "xap_QtFrameImpl.h"
#include "ev_Toolbar_Actions.h"
#include "ev_Toolbar_Layouts.h"
#include "ev_Toolbar_Labels.h"
#include "ev_Toolbar_Control.h"
#include "ev_EditEventMapper.h"
#include "xav_View.h"
#include "xap_Prefs.h"
#include "fv_View.h"
#include "xap_EncodingManager.h"
#include "xap_FontPreview.h"
#include "ut_string_class.h"
#include "pt_PieceTable.h"
#include "ap_Toolbar_Id.h"
#include "ap_QtStockIcons.h"

EV_QtToolbar::EV_QtToolbar(XAP_QtApp   *pQtApp, 
						XAP_Frame 	*pFrame, 
						const char 	*szToolbarLayoutName,
						const char 	*szToolbarLabelSetName)
  : EV_Toolbar(pQtApp->getEditMethodContainer(),
			   szToolbarLayoutName,
			   szToolbarLabelSetName),
	m_pQtApp(pQtApp),
	m_pFrame(pFrame)
{
}

EV_QtToolbar::~EV_QtToolbar(void)
{
}

Qt::ToolButtonStyle EV_QtToolbar::getStyle(void)
{
	const gchar * szValue = NULL;
	m_pQtApp->getPrefsValue(XAP_PREF_KEY_ToolbarAppearance,&szValue);
	UT_ASSERT((szValue) && (*szValue));

	Qt::ToolButtonStyle style = Qt::ToolButtonIconOnly;
	if (g_ascii_strcasecmp(szValue,"text")==0)
		style = Qt::ToolButtonTextOnly;
	else if (g_ascii_strcasecmp(szValue,"both")==0)
		style = Qt::ToolButtonTextBesideIcon;

	return style;
}

QBoxLayout* EV_QtToolbar::_getContainer()
{
#pragma warning TODO
	return NULL;
}

bool EV_QtToolbar::toolbarEvent(_wd 				* wd,
								  const UT_UCSChar 	* pData,
								  UT_uint32 		  dataLength)

{
	// TODO
	return true;
}


/*!
 * This method destroys the container widget here and returns the position in
 * the overall vbox container.
 */
UT_sint32 EV_QtToolbar::destroy(void)
{
	// TODO
	return 0;
}

/*!
 * This method rebuilds the toolbar and places it in the position it previously
 * occupied.
 */
void EV_QtToolbar::rebuildToolbar(UT_sint32 oldpos)
{
	// TODO
}

bool EV_QtToolbar::synthesize(void)
{
	// create a Qt toolbar from the info provided.
	const EV_Toolbar_ActionSet * pToolbarActionSet = m_pQtApp->getToolbarActionSet();
	UT_ASSERT(pToolbarActionSet);

	XAP_Toolbar_ControlFactory * pFactory = m_pQtApp->getControlFactory();
	UT_ASSERT(pFactory);

	UT_uint32 nrLabelItemsInLayout = m_pToolbarLayout->getLayoutItemCount();
	UT_ASSERT(nrLabelItemsInLayout > 0);

	m_wToolbar = new QToolBar();
	UT_ASSERT(m_wToolbar);

	Qt::ToolButtonStyle style = getStyle();
	m_wToolbar->setToolButtonStyle(style);

	for (UT_uint32 k=0; (k < nrLabelItemsInLayout); k++)
	{
		EV_Toolbar_LayoutItem * pLayoutItem = m_pToolbarLayout->getLayoutItem(k);
		UT_continue_if_fail(pLayoutItem);

		XAP_Toolbar_Id id = pLayoutItem->getToolbarId();
		EV_Toolbar_Action * pAction = pToolbarActionSet->getAction(id);
		UT_ASSERT(pAction);
		EV_Toolbar_Label * pLabel = m_pToolbarLabelSet->getLabel(id);
		UT_ASSERT(pLabel);

		const char * szToolTip = pLabel->getToolTip();
		if (!szToolTip || !*szToolTip)
			szToolTip = pLabel->getStatusMsg();		

		switch (pLayoutItem->getToolbarLayoutFlags())
		{
			case EV_TLF_Normal:
			{
				switch (pAction->getItemType())
				{
					case EV_TBIT_PushButton:
					{
						UT_ASSERT(g_ascii_strcasecmp(pLabel->getIconName(),"NoIcon")!=0);
						if(pAction->getToolbarId() != AP_TOOLBAR_ID_INSERT_TABLE)
						{
							const char *icon_name = pLabel->getIconName();
							QPixmap		pixmap;
							pixmap = abi_pixmap_from_toolbar_id (icon_name);
							if(!pixmap.isNull())
							{
								QIcon icon(pixmap);
								QString str = "";
								QPushButton *item = new QPushButton(icon, str);
								m_wToolbar->addWidget(item);
							}
						}
						else
						{
							const char *icon_name = pLabel->getIconName();
							QPixmap		pixmap;
							pixmap = abi_pixmap_from_toolbar_id (icon_name);
							if(!pixmap.isNull())
							{
								QIcon icon(pixmap);
								QString str = "";
								QPushButton *item = new QPushButton(icon, str);
								m_wToolbar->addWidget(item);
							}
						}

						break;
					}
					case EV_TBIT_ToggleButton:
					case EV_TBIT_GroupButton:
					{
						UT_ASSERT(g_ascii_strcasecmp(pLabel->getIconName(),"NoIcon")!=0);
						const char *icon_name = pLabel->getIconName();
						QPixmap		pixmap;
						pixmap = abi_pixmap_from_toolbar_id (icon_name);
						if(!pixmap.isNull())
						{
							QIcon icon(pixmap);					
							QString str = "";
							QPushButton *item = new QPushButton(icon, str);
							item->setCheckable(true);
							m_wToolbar->addWidget(item);
						}
						break;
					}
					case EV_TBIT_EditText:
					{
						break;
					}
					case EV_TBIT_DropDown:
					{
						break;
					}
					case EV_TBIT_ComboBox:
					{
						EV_Toolbar_Control * pControl = pFactory->getControl(this, id);
						UT_ASSERT(pControl);

						QComboBox *combo;
						bool isFontCombo = false;
						if(id == AP_TOOLBAR_ID_FMT_SIZE) 
						{
							combo = new QComboBox();
						}
						else if(id == AP_TOOLBAR_ID_FMT_FONT)
						{
							isFontCombo = true;
							combo = new QFontComboBox();
							QString str = "AbiFontCombo";
							combo->setAccessibleName(str); 
						}
						else if(id == AP_TOOLBAR_ID_ZOOM) 
						{
							combo = new QComboBox();
							QString str = "AbiZoomCombo";
							combo->setAccessibleName(str); 
						}
						else if(id == AP_TOOLBAR_ID_FMT_STYLE) 
						{
							combo = new QComboBox();
							QString str = "AbiStyleCombo";
							combo->setAccessibleName(str); 
						}
						else
						{
							UT_ASSERT(0);
						}

						// populate it
						if (pControl) 
						{
							pControl->populate();
							const UT_GenericVector<const char*> * v = pControl->getContents();
							UT_ASSERT(v);
							gint items = v->getItemCount();	
							if (isFontCombo) 
							{
								for (gint m=0; m < items; m++) 
								{
									QString str = v->getNthItem(m);
									combo->addItem(str);
								}	
							}
							else
							{
								for (gint m=0; m < items; m++) 
								{
									const char * sz = v->getNthItem(m);
									std::string sLoc;
									if (id == AP_TOOLBAR_ID_FMT_STYLE)
									{
										pt_PieceTable::s_getLocalisedStyleName(sz, sLoc);
										sz = sLoc.c_str();
									}
									QString str = sz;
									combo->addItem(str);
								}	
							}
						}

						m_wToolbar->addWidget(combo);

						// for now, we never repopulate, so can just toss it
						DELETEP(pControl);
						break;
					}
					case EV_TBIT_ColorFore:
					case EV_TBIT_ColorBack:
					{

						UT_ASSERT (g_ascii_strcasecmp(pLabel->getIconName(),"NoIcon") != 0);
						QComboBox *combo;

						if (pAction->getItemType() == EV_TBIT_ColorFore) 
						{
							// TODO Some icon implementation
							const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
							std::string sClear;
							pSS->getValueUTF8(XAP_STRING_ID_TB_ClearForeground,sClear);	
							combo = new QComboBox();
							QString str = sClear.c_str();		
							combo->setAccessibleName(str); 				
						}
						else
						{
							// TODO Some icon implementation
							const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
							std::string sClear;
							pSS->getValueUTF8(XAP_STRING_ID_TB_ClearForeground,sClear);	
							combo = new QComboBox();
							QString str = sClear.c_str();		
							combo->setAccessibleName(str); 								
						}
						m_wToolbar->addWidget(combo);
						break;
					}
					case EV_TBIT_StaticLabel:
					{
						// TODO do these...
						break;
					}
					case EV_TBIT_Spacer:
					{
						break;
					}
#ifdef ENABLE_MENUBUTTON
					case EV_TBIT_MenuButton:
					{
						break;
					}
#endif
					case EV_TBIT_BOGUS:
					default:
					{
						break;
					}
				}
				break;
			}
			case EV_TLF_Spacer:
			{
				m_wToolbar->addSeparator();
				break;
			}
			default:
			{
				UT_ASSERT(0);
			}
		}
	}

	QMainWindow * wTopLevel = static_cast<XAP_QtFrameImpl *>(m_pFrame->getFrameImpl())->getTopLevel();
	wTopLevel->addToolBar(m_wToolbar);
	wTopLevel->show();
	return true;
}

void EV_QtToolbar::_releaseListener(void)
{
	// TODO
}
	
bool EV_QtToolbar::bindListenerToView(AV_View * pView)
{
	// TODO
	return true;
}

bool EV_QtToolbar::refreshToolbar(AV_View * pView, AV_ChangeMask mask)
{
	// TODO
	return true;
}

XAP_QtApp * EV_QtToolbar::getApp(void)
{
	return m_pQtApp;
}

XAP_Frame * EV_QtToolbar::getFrame(void)
{
	return m_pFrame;
}

void EV_QtToolbar::show(void)
{
	// TODO
}

void EV_QtToolbar::hide(void)
{
	// TODO
}

/*!
 * This method examines the current document and repopulates the Styles
 * Combo box with what is in the document. It returns false if no styles 
 * combo box was found. True if it all worked.
 */
bool EV_QtToolbar::repopulateStyles(void)
{
	return true;
}
