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

#include <stdlib.h>
#include <string.h>

#include "ut_spi.h"

#include "xap_Module.h"
#include "xap_Spider.h"

XAP_Spider::XAP_Spider () :
  m_spies_max(0),
  m_spies_count(0),
  m_spies(0)
{
  // 
}

XAP_Spider::~XAP_Spider ()
{
  if (m_spies)
    {
      while (*m_spies) unregister_spi (*m_spies);
      free (m_spies);
    }
}

UT_uint32 XAP_Spider::register_spies ()
{
  if (m_spies_count == 0) return 0;

  UT_uint32 total = 0;
  UT_uint32 count;
  do
    {
      count = 0;
      for (UT_uint32 i = 0; i < m_spies_count; i++)
	if (!spi_registered (m_spies[i]))
	  {
	    UT_uint32 ret = register_spi (m_spies[i]);

	    if (ret > 0) count++;
	    else if (ret < 0) break; // registration failed & 'pending' list altered
	  }
      total += count;
    }
  while (count);

  /* remove any unregistered plugins (cyclic dependencies?) from 'pending' list
   */
  struct ABI_Foreign_SPI * bad_spi = 0;
  do
    {
      bad_spi = 0;
      for (UT_uint32 i = 0; i < m_spies_count; i++)
	if (!spi_registered (m_spies[i]))
	  {
	    bad_spi = m_spies[i];
	    break;
	  }
      unregister_spi (bad_spi);
    }
  while (bad_spi);

  return total;
}

UT_sint32 XAP_Spider::register_spi (const char * name)
{
  if ( name == 0) return 0;
  if (*name == 0) return 0;

  struct ABI_Foreign_SPI * spi = find_spi (name);

  return register_spi (spi);
}

UT_sint32 XAP_Spider::register_spi (struct ABI_Foreign_SPI * spi)
{
  if (spi == 0) return 0;

  bool satisfied = true;
  if (char ** dependence = spi->dependencies)
    while (*dependence)
      {
	if (!spi_registered (*dependence))
	  {
	    satisfied = false;
	    break;
	  }
	dependence++;
      }
  if (!satisfied) return 0;

  UT_SPI * spi_glass = UT_SPI::load (spi->plugin_register,
				     spi->plugin_unregister, m_spies, spi->version);
  if (spi_glass == 0)
    {
      unregister_spi (spi); // remove plugin from 'pending' list
      return -1;
    }

  spi->spi = spi_glass->SPI ();

  spi->spi_data = reinterpret_cast<void *>(spi_glass);

  return 1;
}

bool XAP_Spider::spi_registered (const char * name)
{
  if ( name == 0) return false;
  if (*name == 0) return false;

  struct ABI_Foreign_SPI * spi = find_spi (name);

  return spi_registered (spi);
}

bool XAP_Spider::spi_registered (struct ABI_Foreign_SPI * spi)
{
  if (spi == 0) return false;

  return (spi->spi_data ? true : false);
}

void XAP_Spider::unregister_spi (const char * name)
{
  if ( name == 0) return;
  if (*name == 0) return;

  struct ABI_Foreign_SPI * spi = find_spi (name);

  unregister_spi (spi);
}

void XAP_Spider::unregister_spi (struct ABI_Foreign_SPI * spi)
{
  if (spi == 0) return;

  if (spi_registered (spi))
    {
      struct ABI_Foreign_SPI * dependant = 0;
      do
	{
	  dependant = 0;
	  for (UT_uint32 i = 0; i < m_spies_count; i++)
	    {
	      if (char ** dependence = m_spies[i]->dependencies)
		while (*dependence)
		  {
		    if (strcmp (*dependence, spi->name) == 0)
		      {
			dependant = m_spies[i];
			break;
		      }
		    dependence++;
		  }
	      if (dependant) break;
	    }
	  if (dependant) unregister_spi (dependant);
	} while (dependant);

      UT_SPI * spi_glass = reinterpret_cast<UT_SPI *>(spi->spi_data);
      delete spi_glass;

      spi->spi = 0;
      spi->spi_data = 0;
    }

  for (UT_uint32 i = 0; i < m_spies_count; i++)
    if (m_spies[i] == spi)
      {
	m_spies[i] = m_spies[--m_spies_count];
	m_spies[m_spies_count] = 0;
	free (spi);
	break;
      }
}

struct ABI_Foreign_SPI * XAP_Spider::find_spi (const char * name)
{
  struct ABI_Foreign_SPI * spi = 0;

  for (UT_uint32 i = 0; i < m_spies_count; i++)
    if (strcmp (m_spies[i]->name, name) == 0)
      {
	spi = m_spies[i];
	break;
      }
  return spi;
}

const char * XAP_Spider::add_spi (XAP_Module * module)
{
  ABI_SPI_Version fn_version = 0;
  if (!module->resolveSymbol ("abi_spi_version", &(void *)fn_version)) return 0;
  if (fn_version == 0) return 0;

  ABI_SPI_Register fn_register = 0;
  if (!module->resolveSymbol ("abi_spi_register", &(void *)fn_register)) return 0;
  if (fn_register == 0) return 0;

  ABI_SPI_Unregister fn_unregister = 0;
  if (!module->resolveSymbol ("abi_spi_unregister", &(void *)fn_unregister)) return 0;
  if (fn_unregister == 0) return 0;

  char ** dependencies = 0;
  char *  name = 0;

  int version = fn_version (ABI_SPI_VERSION, &name, &dependencies);
  if (version == ABI_SPI_ERROR) return 0;

  if ( name == 0) return 0;
  if (*name == 0) return 0;

  if (find_spi (name)) return 0;

  if (!spies_grow ()) return 0;

  UT_uint32 bytes = sizeof (struct ABI_Foreign_SPI);
  if ((m_spies[m_spies_count] = (struct ABI_Foreign_SPI *) malloc (bytes)) == 0) return 0;

  m_spies[m_spies_count]->version = version;

  m_spies[m_spies_count]->dependencies = dependencies;
  m_spies[m_spies_count]->name = name;

  m_spies[m_spies_count]->plugin_version    = fn_version;
  m_spies[m_spies_count]->plugin_register   = fn_register;
  m_spies[m_spies_count]->plugin_unregister = fn_unregister;

  m_spies[m_spies_count]->spi = 0;

  m_spies[m_spies_count]->spi_data = 0;

  m_spies[++m_spies_count] = 0;

  return name;
}

bool XAP_Spider::spies_grow ()
{
  if (m_spies == 0)
    {
      m_spies = (struct ABI_Foreign_SPI **) malloc (8 * sizeof (struct ABI_Foreign_SPI *));
      if (m_spies == 0) return false;

      m_spies_max = 8;
      m_spies[0] = 0;
    }
  if (m_spies_count + 1 == m_spies_max)
    {
      UT_uint32 bytes = (m_spies_max + 8) * sizeof (struct ABI_Foreign_SPI *);
      struct ABI_Foreign_SPI ** more = (struct ABI_Foreign_SPI **) realloc (m_spies, bytes);
      if (more == 0) return false;

      m_spies = more;
      m_spies_max += 8;
    }
  return true;
}
