/* AbiSource Application Framework
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#pragma once

#include "ut_export.h"

class UT_Rect;
class GR_Graphics;

/// Interface for drawables.
class ABI_EXPORT XAP_Drawable
{
public:
    virtual GR_Graphics* getGraphics() const = 0;
    /// Queue a drawing
    virtual void queueDraw(const UT_Rect* pRect = nullptr) = 0;
    /// Immediately draw.
    virtual void drawImmediate(const UT_Rect* pRect = nullptr) = 0;
};
