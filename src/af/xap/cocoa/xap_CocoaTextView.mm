/* AbiSource Application Framework
 * Copyright (C) 2003 Hubert Figuiere
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
/*
	Implements the text view that supports the NSTextInput protocol
 */
 
#include "ut_assert.h"
#include "ut_debugmsg.h" 
#include "ev_CocoaKeyboard.h"
#include "ev_NamedVirtualKey.h"
#include "xap_Frame.h"
#include "xav_View.h"

#import "xap_CocoaTextView.h"


@implementation XAP_CocoaTextView


/* */
- (void)setView:(AV_View*)view
{
	m_pView = view;
}


- (AV_View*)view
{
	return m_pView;
}

/* standard methods */

- (BOOL)resignFirstResponder
{
	return NO;
}

/* NSResponder methods */
- (void)NVKEvent:(EV_EditBits)code;
{
	AV_View * pView = m_pFrame->getCurrentView();
	ev_CocoaKeyboard * pCocoaKeyboard = static_cast<ev_CocoaKeyboard *>
		(m_pFrame->getKeyboard());

	if (pView) {
		pCocoaKeyboard->NVKEvent(pView, code);
	}
}

- (void)insertNewline:(id)sender
{
	[self  NVKEvent:EV_NVK_RETURN];
}

- (void)insertTab:(id)sender
{
	[self  NVKEvent:EV_NVK_TAB];
}

- (void)deleteBackward:(id)sender
{
	[self  NVKEvent:EV_NVK_BACKSPACE];
}

- (void)deleteForward:(id)sender
{
	[self  NVKEvent:EV_NVK_DELETE];
}

- (void)moveForward:(id)sender;
{
	[self  NVKEvent:EV_NVK_RIGHT];
}

- (void)moveRight:(id)sender;
{
	[self  NVKEvent:EV_NVK_RIGHT];
}

- (void)moveBackward:(id)sender;
{
	[self  NVKEvent:EV_NVK_LEFT];
}

- (void)moveLeft:(id)sender;
{
	[self  NVKEvent:EV_NVK_LEFT];
}

- (void)moveUp:(id)sender;
{
	[self  NVKEvent:EV_NVK_UP];
}

- (void)moveDown:(id)sender;
{
	[self  NVKEvent:EV_NVK_DOWN];
}

- (void)moveToBeginningOfLine:(id)sender
{
	[self  NVKEvent:EV_NVK_HOME];
}

- (void)moveToEndOfLine:(id)sender
{
	[self  NVKEvent:EV_NVK_END];
}

- (void)pageDown:(id)sender;
{
	[self  NVKEvent:EV_NVK_PAGEDOWN];
}
- (void)pageUp:(id)sender;
{
	[self  NVKEvent:EV_NVK_PAGEUP];
}

/* NSTextInput protocol */
- (NSAttributedString *)attributedSubstringFromRange:(NSRange)theRange
{
	UT_ASSERT_NOT_REACHED();
}


- (unsigned int)characterIndexForPoint:(NSPoint)thePoint
{
	UT_ASSERT_NOT_REACHED();
}


- (long)conversationIdentifier
{
	return (long)self;
}

- (void)doCommandBySelector:(SEL)aSelector
{
	if ([self respondsToSelector:aSelector]) {
		[self performSelector:aSelector withObject:self];
		return;
	}
	UT_ASSERT_NOT_REACHED();
}


- (NSRect)firstRectForCharacterRange:(NSRange)theRange
{
	UT_ASSERT_NOT_REACHED();
}


- (BOOL)hasMarkedText
{
	return NO;
}


- (void)insertText:(id)aString
{
//  	pFrame->setTimeOfLastEvent([theEvent timestamp]);
	AV_View * pView = m_pFrame->getCurrentView();
	ev_CocoaKeyboard * pCocoaKeyboard = static_cast<ev_CocoaKeyboard *>
		(m_pFrame->getKeyboard());

	if (pView)
		pCocoaKeyboard->insertTextEvent(pView, aString);	
}


- (NSRange)markedRange
{
	UT_ASSERT_NOT_REACHED();
	return NSMakeRange (NSNotFound, 0);
}

- (NSRange)selectedRange
{
	UT_ASSERT_NOT_REACHED();
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)selRange
{
	UT_ASSERT_NOT_REACHED();
}

- (void)unmarkText
{
	UT_ASSERT_NOT_REACHED();
}

- (NSArray*)validAttributesForMarkedText
{
	return [NSArray array];
}


@end
