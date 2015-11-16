/* AbiWord
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_PrintingDelegate.h"
#import "xap_CocoaCompat.h"
#import "xap_PrintingNSView.h"


@implementation XAP_PrintingNSView

- (void)dealloc
{
	DELETEP(_printingDelegate);
	[super dealloc];
}

- (void)setPrintingDelegate:(XAP_PrintingDelegate*)delegate
{
	DELETEP(_printingDelegate);
	_printingDelegate = delegate;
}


- (BOOL)knowsPageRange:(NSRangePointer)range 
{
	UT_ASSERT(_printingDelegate);
	
	range->length = _printingDelegate->getPageCount();
	range->location = 1;

	return YES;
}

- (NSRect)rectForPage:(NSInteger)page
{
	UT_UNUSED(page);
	return [self bounds];
}

- (void)drawRect:(NSRect)rect 
{
	UT_UNUSED(rect);
    // Drawing code here.
	if ([NSGraphicsContext currentContextDrawingToScreen]) {
		UT_ASSERT_NOT_REACHED();		
	}
	else {
		_printingDelegate->printPage([[NSPrintOperation currentOperation] currentPage]);
	}
}

@end


