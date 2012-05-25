#ifndef IE_TOC_H
#define IE_TOC_H

#include "ut_string_class.h"
#include "ut_vector.h"
#include "pt_Types.h"

class PD_Document;

class ABI_EXPORT IE_TOCHelper
{
  friend class TOC_Listener;

 public:

  IE_TOCHelper(PD_Document * doc);
  ~IE_TOCHelper();

  bool hasTOC() const; // true if there are any "headings" in the document
  bool docHasTOC() const; // true if the doc has 1 or more tables of contents

  bool isTOCStyle(const UT_UTF8String & styleName, int * out_level = NULL) const;
  bool isTOCStyle(const char * styleName, int * out_level = NULL) const;

  int getNumTOCEntries() const;

  UT_UTF8String getNthTOCEntry(int nth, int * out_level = NULL) const;
  bool getNthTOCEntryPos(int nth, PT_DocPosition &pos) const;

 private:

  bool _tocNameLevelHelper(const UT_UTF8String & styleName,
			   const char * sLStyle) const;

  void _defineTOC(const UT_UTF8String & toc_text, int level, PT_DocPosition pos);

  // undefined
  IE_TOCHelper();
  IE_TOCHelper(const IE_TOCHelper & rhs);
  IE_TOCHelper& operator=(const IE_TOCHelper & rhs);

  // storage
  UT_GenericVector<UT_UTF8String *> mTOCStrings;
  UT_GenericVector<int> mTOCLevels;
  // By adding this vector we making it possible to get ranges of every
  // section of a document by using TOCHelper
  UT_GenericVector<PT_DocPosition> mTOCPositions;

  bool mHasTOC;
  bool mDocHasTOC;
  PD_Document *mDoc;
};

#endif
