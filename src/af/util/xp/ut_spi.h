/* Simple Plugin Interface - Section 1: Interface Definitions
 * 
 * Copyright (C) 2002 Francis James Franklin <fjf@alinameridon.com>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#ifndef UTSPI_H
#define UTSPI_H

/* IMPORTANT - This file should be included only *once* by any given plugin,
 * =========   and ABI_PLUGIN_SOURCE should be #defined (especially if the
 *             plugin is written in C).
 */

#ifdef WIN32
#define ABI_SPI_EXPORT __declspec(dllexport)
#ifdef ABI_PLUGIN_SOURCE

#include <windows.h>

static HANDLE s_hModule = (HANDLE) NULL;

BOOL APIENTRY DllMain (HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  s_hModule = hModule;
  return TRUE;
} 

#endif /* ABI_PLUGIN_SOURCE */
#else
#define ABI_SPI_EXPORT 
#endif

#ifdef __cplusplus
extern "C" {
#endif

  /* - Start of "C"-style interface definitions ------------------------- */

  /* - SPI: Field ------------------------------------------------------- */

  /* Maintainers: if changing this struct, first copy it to the archive section
   *              below, then increment the version number here (hint: there
   *              are three instances)
   */
#define  ABI_FIELD_SPI_VERSION 1
  struct ABI_Field_SPI_v1
  {
    int version; /* *always* the first entry! plugin sets this (version > 0) */

    /* These are required entries:
     */
    int (*doc_init) (void * plugin_data, void ** doc_data);
    int (*doc_free) (void * plugin_data, void *  doc_data);

    int (*doc_fins) (void * plugin_data, void *  doc_data, char *** field_atts);
    int (*doc_fdel) (void * plugin_data, void *  doc_data, char *** field_atts);
    int (*doc_fref) (void * plugin_data, void *  doc_data, char *** field_atts,
		     char ** content_type, char ** utf8_text);
  };
  typedef struct ABI_Field_SPI_v1 ABI_Field_SPI;

  /* - SPI -------------------------------------------------------------- */

  /* Maintainers: if changing this struct, first copy it to the section
   *              at the bottom of this file, then increment the version
   *              number here (hint: there are three instances)
   */
#define  ABI_SPI_VERSION 1
  struct ABI_SPI_v1
  {
    int version; /* *always* the first entry! AbiWord sets this (version > 0) */

    /* hook for plugin to hang user data; this variable is passed to all
     * SPI functions (except abi_spi_register)
     */
    void * plugin_data;

    /* attach descriptive strings here:
     */
    char * plugin_name;    /* plugin name */
    char * plugin_desc;    /* plugin description */
    char * plugin_version; /* plugin version string */
    char * plugin_author;  /* plugin author's name */
    char * plugin_usage;   /* plugin usage/instructions */

    /* AbiWord initializes the following pointers to 0; attach the address of
     * the appropriate struct (defined above) if you implement the interface.
     * You *must* set the interface version correctly, e.g.:

     static ABI_Field_SPI field = { ABI_FIELD_SPI_VERSION };

     if (spi->version == ABI_SPI_VERSION) spi->spi_field = &field;

     */
    ABI_Field_SPI * spi_field;
  };
  typedef struct ABI_SPI_v1 ABI_SPI;

  /* - Required functions for SPI plugin loading & registration --------- */

  struct ABI_Foreign_SPI;

#ifdef ABI_PLUGIN_SOURCE
  /* all plugins which use the SPI must implement the following:
   */
  ABI_SPI_EXPORT int abi_spi_version (int version, char ** name, char *** dependencies);
  ABI_SPI_EXPORT int abi_spi_register (ABI_SPI * spi, struct ABI_Foreign_SPI * const * spies);
  ABI_SPI_EXPORT int abi_spi_unregister (void * plugin_data, ABI_SPI * spi);
#endif
  typedef int (*ABI_SPI_Version) (int version, char ** name, char *** dependencies);
  typedef int (*ABI_SPI_Register) (ABI_SPI * spi, struct ABI_Foreign_SPI * const * spies);
  typedef int (*ABI_SPI_Unregister) (void * plugin_data, ABI_SPI * spi);

  /* Note for plugins:
   * If the plugin is unable to implement any interfaces then

   return ABI_SPI_ERROR;

   * On success, however,

   return spi->version;

   */
#define ABI_SPI_ERROR 0

  /* - SPI: Foreign ----------------------------------------------------- */

  struct ABI_Foreign_SPI
  {
    int version;

    char ** dependencies;
    char *  name;

    ABI_SPI_Version    plugin_version;
    ABI_SPI_Register   plugin_register;
    ABI_SPI_Unregister plugin_unregister;

    ABI_SPI * spi;

    void * spi_data;
  };

  /* - Archive ---------------------------------------------------------- */

  /* Maintainers: include here any old versions of SPI structs that AbiWord
   *              still supports...
   */

  /* - End of "C"-style interface definitions --------------------------- */

#ifdef __cplusplus
}
#endif

#ifndef ABI_PLUGIN_SOURCE

/* Simple Plugin Interface - Section 2: AbiSource Program Utilities
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
 
class UT_SPI_Field
{
 protected:
  UT_SPI_Field (void * plugin_data);

 public:
  virtual ~UT_SPI_Field ();

  virtual int version () const = 0; // override this with Field_SPI version no.

  virtual int doc_init (void *& doc_data) = 0;
  virtual int doc_free (void *  doc_data) = 0;

  virtual int doc_fins (void *  doc_data, char **& field_atts) = 0;
  virtual int doc_fdel (void *  doc_data, char **& field_atts) = 0;
  virtual int doc_fref (void *  doc_data, char **& field_atts,
			char *& content_type, char *& utf8_text) = 0;

 protected:
  void * plugin_data () const { return m_plugin_data; }

 private:
  void * m_plugin_data;
};

class UT_SPI_Field_v1 : public UT_SPI_Field
{
 public:
  UT_SPI_Field_v1 (void * plugin_data, struct ABI_Field_SPI_v1 * field);

  ~UT_SPI_Field_v1 ();

  int version () const { return m_version; }

  int doc_init (void *& doc_data);
  int doc_free (void *  doc_data);

  int doc_fins (void *  doc_data, char **& field_atts);
  int doc_fdel (void *  doc_data, char **& field_atts);
  int doc_fref (void *  doc_data, char **& field_atts, char *& content_type, char *& utf8_text);

 private:
  int m_version;

  struct ABI_Field_SPI_v1 * m_field;
};

class UT_SPI
{
 protected:
  UT_SPI ();

 public:
  virtual ~UT_SPI ();

  virtual int version () const = 0; // override this with SPI version no.

  const char * plugin_name ()    const { return m_name; }
  const char * plugin_desc ()    const { return m_desc; }
  const char * plugin_version () const { return m_version; }
  const char * plugin_author ()  const { return m_author; }
  const char * plugin_usage ()   const { return m_usage; }

 protected:
  const char * plugin_name (const char * name);
  const char * plugin_desc (const char * desc);
  const char * plugin_version (const char * version);
  const char * plugin_author (const char * author);
  const char * plugin_usage (const char * usage);

 private:
  const char * m_null;

  char * m_name;
  char * m_desc;
  char * m_version;
  char * m_author;
  char * m_usage;

 public:
  UT_SPI_Field * field () const { return m_field; }

 protected:
  UT_SPI_Field * m_field;

 public:
  static UT_SPI * load (ABI_SPI_Register   plugin_register,
			ABI_SPI_Unregister plugin_unregister,
			struct ABI_Foreign_SPI * const * spies, int version);

  virtual ABI_SPI * SPI () = 0;
};

class UT_SPI_v1 : public UT_SPI
{
 public:
  UT_SPI_v1 (ABI_SPI_Register plugin_register, ABI_SPI_Unregister plugin_unregister,
	     struct ABI_Foreign_SPI * const * spies);

  ~UT_SPI_v1 ();

  int version () const { return m_version; }

  ABI_SPI * SPI () { return reinterpret_cast<ABI_SPI *>(&m_spi); }

 private:
  int m_version;

  ABI_SPI_Unregister m_plugin_unregister;

  struct ABI_SPI_v1 m_spi;
};

#endif /* ! ABI_PLUGIN_SOURCE */

#endif /* UTSPI_H */
