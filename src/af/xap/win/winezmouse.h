 /*
  * Scroll wheel mouse definitions
  *
  * Copyright (C) 2000 CodeWeavers
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 2.1 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
  */

 #ifndef __WINE_ZMOUSE_H
 #define __WINE_ZMOUSE_H

 #include <wchar.h>

 #define MSH_MOUSEWHEEL L"MSWHEEL_ROLLMSG"
 #define MOUSEZ_CLASSNAME  L"MouseZ"
 #define MOUSEZ_TITLE      L"Magellan MSWHEEL"

 #define MSH_WHEELMODULE_CLASS (MOUSEZ_CLASSNAME)
 #define MSH_WHEELMODULE_TITLE (MOUSEZ_TITLE)

 #define MSH_WHEELSUPPORT L"MSH_WHEELSUPPORT_MSG"

 #define MSH_SCROLL_LINES L"MSH_SCROLL_LINES_MSG"

 #ifndef WHEEL_DELTA
 #define WHEEL_DELTA      120
 #endif

 #ifndef WHEEL_PAGESCROLL
 #define WHEEL_PAGESCROLL  (UINT_MAX)
 #endif

 #ifndef SPI_SETWHEELSCROLLLINES
 #define SPI_SETWHEELSCROLLLINES   105
 #endif

 #endif  /* __WINE_ZMOUSE_H */
