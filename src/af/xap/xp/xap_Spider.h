/* AbiSource Application Framework
 * 
 * Copyright (C) 2002 Francis James Franklin <fjf@alinameridon.com>
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

#ifndef XAP_SPIDER_H
#define XAP_SPIDER_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

struct ABI_Foreign_SPI;

class UT_SPI;

class XAP_Module;

class XAP_Spider
{
 public:
  XAP_Spider ();

  ~XAP_Spider ();

  /* register_spi works only if all the specified spi's dependencies
   * (if any) are registered. returns 1 if registration is successful,
   * 0 if dependencies are not met, and -1 if registration fails (in
   * which case the plugin is removed from the 'pending' list)
   * 
   * register_spies cycles through the 'pending' list until no further
   * progress can be made & returns the number of plugins registered
   */
  UT_uint32 register_spies ();
  UT_sint32 register_spi (const char * name);
 private:
  UT_sint32 register_spi (struct ABI_Foreign_SPI * spi);

 public:
  bool spi_registered (const char * name);
 private:
  bool spi_registered (struct ABI_Foreign_SPI * spi);

 public:
  /* unregistering a spi will remove it from the 'pending' list
   */
  void unregister_spi (const char * name);
 private:
  void unregister_spi (struct ABI_Foreign_SPI * spi);

  struct ABI_Foreign_SPI * find_spi (const char * name);

 public:
  UT_SPI * lookup_spi (const char * name);

  /* returns a pointer to a string (the plugin's spi-name/id) inside the plugin
   * so it's only valid as long as the plugin is loaded
   * 
   * add_spi() adds the plugin to a 'pending' list; 
   */
  const char * add_spi (XAP_Module * module);

 private:
  bool spies_grow ();

  UT_uint32 m_spies_max;
  UT_uint32 m_spies_count;

  struct ABI_Foreign_SPI ** m_spies;
};

#endif /* XAP_SPIDER_H */
