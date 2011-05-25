/* AbiSuite
 * Copyright (C) 2001 chenxiajian <chenxiajian1985@gmail.com> 
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

#ifndef HYPHENATE_MANAGER_H
#define HYPHENATE_MANAGER_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_string_class.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "barbarisms.h"

// forward declaration
class HyphenationManager;

//struct DictionaryMapping
//{
//	UT_String lang;	// the language tag
//	UT_String dict;	// the dictionary for the tag
//	UT_String enc;	// the encoding of the dictionary
//};



class ABI_EXPORT Hyphenation
{
	friend class HyphenationManager;
public:
	enum HyphenationResult
	{
		Hyphenation_SUCCEEDED = 0, // Hyphenation the word succeeded
		Hyphenation_FAILED = 1,    // could not Hyphenation the word
		Hyphenation_ERROR = 2      // internal error
	};	
	//hyphenate the word and get the hyphenation result vector(find a best result to match the line)
	UT_GenericVector<UT_UCSChar*>* hyphenateWord(const UT_UCSChar* word, size_t len);

	// vector of DictionaryMapping*
	virtual	UT_Vector & getMapping() {return m_vecEmpty;};
	virtual bool doesDictionaryExist (const char * /*szLang*/) {return false;};
	virtual bool addToCustomDict (const UT_UCSChar *word, size_t len);

	const UT_String& getLanguage () const
	{
		return m_sLanguage;
	}

	bool requestDictionary (const char * szLang);
	bool isDictionaryFound(void)
	{ return m_bFoundDictionary;}
	void setDictionaryFound(bool b)
	{ m_bFoundDictionary = b;}
protected:
	Hyphenation();

	virtual ~Hyphenation();

    void setLanguage (const char * lang)
    {
		m_sLanguage = lang;
    }

	static void couldNotLoadDictionary ( const char * szLang );


    UT_String       	m_sLanguage;
    UT_Vector			m_vecEmpty;

    bool				m_bIsBarbarism;
	bool				m_bIsDictionaryWord;
	bool                m_bFoundDictionary;

private:
    Hyphenation(const Hyphenation&);		// no impl
    void operator=(const Hyphenation&);	// no impl

	virtual bool				_requestDictionary (const char * szLang) = 0;
	virtual HyphenationResult	__hyphenate(const UT_UCSChar* word, size_t len) = 0;
	virtual UT_GenericVector<UT_UCSChar*>* __hyphenateWord(const UT_UCSChar* word, size_t len)=0;

};

class ABI_EXPORT HyphenationManager
{
public:
	static HyphenationManager & instance (void);

	virtual ~HyphenationManager ();


	virtual HyphenationManager * lastDictionary (void) const;
	virtual HyphenationManager * requestDictionary (const char * szLang);
	UT_uint32 numLoadedDicts () const { return m_nLoadedDicts; }

	Hyphenation *	getInstance() const;

private:
	HyphenationManager ();
	HyphenationManager ( const HyphenationManager & other );
	HyphenationManager & operator= ( const HyphenationManager & other );


	UT_StringPtrMap m_map;
	UT_String m_missingHashs;
	Hyphenation * m_lastDict;
	UT_uint32 m_nLoadedDicts;
};

#endif /* HYPHENATE_MANAGER_H */
