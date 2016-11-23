/* AbiSource
 *
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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

#ifndef _ODI_FRAME_LISTENERSTATE_H_
#define _ODI_FRAME_LISTENERSTATE_H_

#include <ut_std_string.h>

// Internal includes
#include "ODi_ListenerState.h"

// Internal classes
class ODi_Office_Styles;
class ODi_Style_List;
class ODi_Abi_Data;

// AbiWord classes
class PD_Document;


/**
 * To parse a <draw:frame> element.
 */
class ODi_Frame_ListenerState : public ODi_ListenerState {

public:

	ODi_Frame_ListenerState(PD_Document* pDocument,
        ODi_Office_Styles* pStyles,
        ODi_Abi_Data& rAbiData,
        ODi_ElementStack& rElementStack);

    virtual ~ODi_Frame_ListenerState() {}

    void startElement (const gchar* pName, const gchar** ppAtts,
                       ODi_ListenerStateAction& rAction);

    void endElement (const gchar* pName, ODi_ListenerStateAction& rAction);

    void charData (const gchar* pBuffer, int length);

private:

    /**
     * @param ppAtts The attributes of a <draw:image> element.
     */
    void _drawImage (const gchar** ppAtts, ODi_ListenerStateAction& rAction);

    void _drawInlineImage (const gchar** ppAtts);

    /**
     * @param ppAtts The attributes of a <draw:object> element.
     */
    void _drawObject (const gchar** ppAtts, ODi_ListenerStateAction& rAction);

    /**
     * @param ppAtts The attributes of a <draw:text-box> element.
     * @param rAction Any action to be taken, regarding state change.
     */
    void _drawTextBox (const gchar** ppAtts, ODi_ListenerStateAction& rAction);

    bool _getFrameProperties(std::string& rProps, const gchar** ppAtts);

	PD_Document* m_pAbiDocument;
    ODi_Abi_Data& m_rAbiData;
    ODi_Office_Styles* m_pStyles;

    bool m_parsedFrameStartTag;
    bool m_bOnContentStream;

    // "true" if this frame has degenerated into an inlined <image>
    // on the AbiWord document.
    bool m_inlinedImage;
    UT_sint32 m_iFrameDepth;

    UT_ByteBufPtr m_pMathBB;
    bool m_bInMath;

	bool m_bInlineImagePending;
	bool m_bPositionedImagePending;
	std::string m_sAltTitle;
	bool m_bInAltTitle;
	std::string m_sAltDesc;
	bool m_bInAltDesc;
	std::map<std::string, std::string> m_mPendingImgProps;
};

#endif //_ODI_FRAME_LISTENERSTATE_H_
