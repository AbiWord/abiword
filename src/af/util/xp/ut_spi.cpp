/* AbiSource Program Utilities: Simple Plugin Interface
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
 
#include "ut_spi.h"
#include "ut_string.h"
#include "ut_exception.h"

UT_SPI_Field::UT_SPI_Field (void * plugin_data) :
  m_plugin_data(plugin_data)
{
  // 
}

UT_SPI_Field::~UT_SPI_Field ()
{
  // 
}


UT_SPI_Field_v1::UT_SPI_Field_v1 (void * plugin_data, struct ABI_Field_SPI_v1 * field) :
  UT_SPI_Field(plugin_data),
  m_version(1),
  m_field(field)
{
  if (m_field == 0) // bad!
    {
      m_version = ABI_SPI_ERROR;
      return;
    }
  if (m_field->version != 1)
    {
      m_version = ABI_SPI_ERROR;
      return;
    }
  if (!(m_field->doc_init &&
	m_field->doc_free &&
	m_field->doc_fins &&
	m_field->doc_fdel &&
	m_field->doc_fref))
    {
      m_version = ABI_SPI_ERROR;
      return;
    }
}

UT_SPI_Field_v1::~UT_SPI_Field_v1 ()
{
  // 
}

int UT_SPI_Field_v1::doc_init (void *& doc_data)
{
  if (version () != 1) return 0;

  return m_field->doc_init (plugin_data (), &doc_data);
}

int UT_SPI_Field_v1::doc_free (void * doc_data)
{
  if (version () != 1) return 0;

  return m_field->doc_free (plugin_data (), doc_data);
}

int UT_SPI_Field_v1::doc_fins (void * doc_data, char **& field_atts)
{
  if (version () != 1) return 0;

  return m_field->doc_fins (plugin_data (), doc_data, &field_atts);
}

int UT_SPI_Field_v1::doc_fdel (void * doc_data, char **& field_atts)
{
  if (version () != 1) return 0;

  return m_field->doc_fdel (plugin_data (), doc_data, &field_atts);
}

int UT_SPI_Field_v1::doc_fref (void * doc_data, char **& field_atts,
			       char *& content_type, char *& utf8_text)
{
  if (version () != 1) return 0;

  return m_field->doc_fref (plugin_data (), doc_data, &field_atts, &content_type, &utf8_text);
}

UT_SPI::UT_SPI ():
  m_null(""),
  m_name(0),
  m_desc(0),
  m_version(0),
  m_author(0),
  m_usage(0),
  m_field(0)
{
  m_name    = const_cast<char *>(m_null);
  m_desc    = const_cast<char *>(m_null);
  m_version = const_cast<char *>(m_null);
  m_author  = const_cast<char *>(m_null);
  m_usage   = const_cast<char *>(m_null);
}

UT_SPI::~UT_SPI ()
{
  if (m_name    != m_null) FREEP (m_name);
  if (m_desc    != m_null) FREEP (m_desc);
  if (m_version != m_null) FREEP (m_version);
  if (m_author  != m_null) FREEP (m_author);
  if (m_usage   != m_null) FREEP (m_usage);
}

const char * UT_SPI::plugin_name (const char * name)
{
  if (m_name != m_null) FREEP (m_name);
  if ((m_name = UT_strdup (name)) == 0) m_name = const_cast<char *>(m_null);
  return (m_name);
}

const char * UT_SPI::plugin_desc (const char * desc)
{
  if (m_desc != m_null) FREEP (m_desc);
  if ((m_desc = UT_strdup (desc)) == 0) m_desc = const_cast<char *>(m_null);
  return (m_desc);
}

const char * UT_SPI::plugin_version (const char * version)
{
  if (m_version != m_null) FREEP (m_version);
  if ((m_version = UT_strdup (version)) == 0) m_version = const_cast<char *>(m_null);
  return (m_version);
}

const char * UT_SPI::plugin_author (const char * author)
{
  if (m_author != m_null) FREEP (m_author);
  if ((m_author = UT_strdup (author)) == 0) m_author = const_cast<char *>(m_null);
  return (m_author);
}

const char * UT_SPI::plugin_usage (const char * usage)
{
  if (m_usage != m_null) FREEP (m_usage);
  if ((m_usage = UT_strdup (usage)) == 0) m_usage = const_cast<char *>(m_null);
  return (m_usage);
}

UT_SPI * UT_SPI::load (ABI_SPI_Register plugin_register, ABI_SPI_Unregister plugin_unregister,
		       struct ABI_Foreign_SPI * const * spies, int version)
{
  UT_SPI * spi = 0;
  switch (version)
    {
    case 1:
      UT_TRY
	{
	  spi = new UT_SPI_v1(plugin_register,plugin_unregister,spies);
	}
      UT_CATCH (...)
	{
	  spi = 0;
	}
      break;
    default:
      break;
    }
  if (spi == 0) return 0;

  if (spi->version () != version) return 0;

  return spi;
}

UT_SPI_v1::UT_SPI_v1 (ABI_SPI_Register plugin_register, ABI_SPI_Unregister plugin_unregister,
		      struct ABI_Foreign_SPI * const * spies) :
  m_version(1),
  m_plugin_unregister(plugin_unregister)
{
  if ((plugin_register == 0) || (plugin_unregister == 0))
    {
      m_version = ABI_SPI_ERROR;
      return;
    }
  if ((m_version = plugin_register (SPI (), spies)) != 1) return;

  if (m_spi.plugin_name)    plugin_name    (m_spi.plugin_name);
  if (m_spi.plugin_desc)    plugin_desc    (m_spi.plugin_desc);
  if (m_spi.plugin_version) plugin_version (m_spi.plugin_version);
  if (m_spi.plugin_author)  plugin_author  (m_spi.plugin_author);
  if (m_spi.plugin_usage)   plugin_usage   (m_spi.plugin_usage);

  if (m_spi.spi_field)
    {
      switch (m_spi.spi_field->version)
	{
	case 1:
	  m_field = new UT_SPI_Field_v1(m_spi.plugin_data,m_spi.spi_field);
	  break;
	default:
	  break;
	}
      if (m_field)
	if (m_field->version () != m_spi.spi_field->version)
	  {
	    delete m_field;
	    m_field = 0;
	  }
    }
}

UT_SPI_v1::~UT_SPI_v1 ()
{
  if (version () != 1) return;

  if (m_field) delete m_field;

  m_plugin_unregister (m_spi.plugin_data, SPI ());
}
