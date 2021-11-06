/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:nil; -*- */
/* Abiword
 * Copyright (C) 2021 Hubert Figuière
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

#pragma once

#include <CoreGraphics/CoreGraphics.h>

/// Contruct GR_CGStateSave to save the GState
/// it will be restoreed when the object get out of scope
class GR_CGStateSave
{
public:
    GR_CGStateSave(const GR_CGStateSave&) = delete;
    GR_CGStateSave(CGContextRef context)
        : m_context(context)
    {
        ::CGContextSaveGState(m_context);
    }
    ~GR_CGStateSave()
    {
        ::CGContextRestoreGState(m_context);
    }

private:
    CGContextRef m_context;
};
