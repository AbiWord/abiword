
#ifndef PX_CHANGERECORD_SPAN_H
#define PX_CHANGERECORD_SPAN_H

#include "ut_types.h"
#include "px_ChangeRecord.h"

// PX_ChangeRecord_Span describes an insertSpan or
// deleteSpan change made to the document.
// This description should be sufficient to allow undo to
// work and sufficient to allow the formatter to do a
// partial format and screen update (if appropriate).
// The change record must be free of pointers, since it
// represents what was done to the document -- and not
// how it was done (that is, not what was done to various
// intermediate data structures).  this also lets it be
// cached to disk (for autosave and maybe multi-session
// undo).
//
// m_position contains the absolute document position of
// the text span at the time the change was made.
// m_vsIndex,m_offset,m_length describe the actual contents
// of the text span.  Note that we do not include formatting
// information in this change record -- it is assumed that
// the text span will receive the same attributes/properties
// as the surrounding text.

class PX_ChangeRecord_Span : public PX_ChangeRecord
{
public:
	PX_ChangeRecord_Span();
	~PX_ChangeRecord_Span();
	
protected:
	UT_Bool					m_bLeftSide;
	pt_BufPosition			m_offset;	/* location of our text in the VS[].m_buffer */
	UT_uint32				m_length;	/* length of our text in that buffer */
};

#endif /* PX_CHANGERECORD_SPAN_H */
