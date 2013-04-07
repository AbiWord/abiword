/* AbiWord
 * Copyright (C) 2002 Hubert Figuiere
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
/*
	A bunch of convenience routines to convert Abi classes to Foundation
 */
 
#import "xap_CocoaAbiConversions.h"

#import "ut_bytebuf.h"

@implementation NSData (CocoaAbiConversions)

/*!
	init a NSData from an UT_ByteBuf
 */
- (NSData*)initWithAbiByteBuffer:(const UT_ByteBuf*)byteBuf
{
	return [self initWithBytes:byteBuf->getPointer(0) length:byteBuf->getLength()];
}

/*!
	Output the NSData content to an allocated UT_ByteBuf
 */
- (void)convertToAbiByteBuf:(UT_ByteBuf*)byteBuf
{
	byteBuf->truncate(0);
	byteBuf->append((const UT_Byte*)[self bytes], [self length]);
}

@end