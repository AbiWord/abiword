/*! \page ptbl_overview PieceTable

\todo Add more class names / links to sources.

<h1>1. PieceTable</h1>

<h2>1.1. Introduction</h2>

<p>A pt_PieceTable is the data structure used to represent the
document.  It presents an interface to access the document content as
a sequence of (Unicode) characters.  It includes an interface to
access document structure and formatting information.  It provides
efficient editing operations, complete undo, and crash recovery.</p>

<h2>1.2. Class Overview</h2>

<p>The PieceTable consists of the following classs:

<ol>

<li>InitialBuffer -- This is a read-only character array consisting of
the entire character content of the document and initially read from
the disk.  <i>(All XML tags and other non-content items are omitted
from this buffer.)</i>

<li>ChangeBuffer -- This is an append-only character
array consisting of all character content inserted
into the document during the editing session.

<li>InitialAttrPropTable -- This is a read-only
table of Attribute/Property structures extracted from
the original document.

<li>ChangeAttrPropTable -- This is an append-only table
of Attribute/Property structures that are created
during the editing session.

<li>Piece -- This class represents a <i>piece of the
sequence</i> of the document; that is, a contiguous
sub-sequence having the same properties.  Such as a
span of text or an object (such as an in-line image).
It contains a links to the previous and next Pieces
in the document.  Pieces are created in response to
editing and formatting commands.

<ol>
<li>TextPiece -- This subclass represents a span of
contiguous text in one of the buffers.  All text within
the span has the same (CSS) properties.  A TextPiece
is not necessarily the longest contiguous span; it is
possible to have <i>adjacent</i> (both in order and in
buffer position) TextPieces with the same properties.
A TextPiece contains a buffer offset and length for
the location an size of the text and a flag to indicate
which buffer.  A TextPiece contains (or contains a
link to) the text formatting information.  Note that
the buffer offset only gives the location of the content
of the span in one of the buffers, it does not specify
the absolute position of the span in the document.

<li>ObjectPiece -- This subclass represents an in-line
object or image.  It has no references to the buffers,
but does provide a place-holder in the sequence.

<li>StructurePiece -- This subclass represents a
section or paragraph.  It has no references to the
buffers, but does provide (CSS) style information and
a place-holder in the sequence.  There are no links
between StructurePieces or between other Pieces and
their (containing) StructurePieces.
</ol>

<li>PieceList -- This is doubly-linked list of Pieces.
The are linked in document order.  A forward traversal
of this list will reveal the entire content of the
document; in doing so, it may wildly jump around both
of the buffers, but that is not an issue.

<li>PX_ChangeRecord -- Each editing and formatting change is
represented as a ChangeRecord.  A ChangeRecord
represents an atomic change that was made to one or
more pieces.  This includes offset/length changes to
a TextPiece and changes to the PieceList.

<li>ChangeVector -- This is a vector of ChangeRecords.
This is used like a stack.  ChangeRecords are appended
to the vector (pushed onto the stack) as they are
created in response to editing and formatting commands.
The <b>undo</b> operation takes the last ChangeRecord
in the vector and un-does its effect.  A <b>redo</b>
operation re-applies the ChangeRecord.  The ChangeVector
holds the complete information to undo all editing back
to the initial document. The index of the current
position in the ChangeVector is maintained.
ChangeRecords are not removed from the vector until the
<b>redo</b> is invalidated.  When a ChangeRecord is
removed from the vector, it is deleted.
</ol>

<h2>1.3. Operations</h2>

<ol>
<li>Insert(<i>position</i>,<i>bAfter</i>,<i>c</i>) -- To
<b>insert</b> one or more characters <i>c</i> into the
document (either <i>before</i> or <i>after</i>) the
absolute document position <i>position</i>, we do the
following:
<ol>
<li>Append the character(s) to the ChangeBuffer.
<li>Find the TextPiece that spans the document position.
<ul>
<li>If the document position is in the middle of a
TextPiece
(<i>p1</i>), we split it into two TextPieces (<i>p1a</i>,
<i>p1c</i>) and create a third TextPiece (<i>p1b</i>).
<i>p1a</i> and <i>p1c</i> contain the left and right
portions referenced in <i>p1</i>.  <i>p1b</i> spans
the newly-inserted character(s).  The PieceList is
updated so that the sequence <i>p1a,p1b,p1c</i> replace
<i>p1</i> in the list.
<li>If the document position is at the end of a TextPiece
and the buffer position in either buffer is contiguous
with the buffer and position referenced in the TextPiece
and the formatting is the same, we may avoid the three
part split and simply update the offset/length in the
TextPiece.  This case is very likely when the user is
composing text or is undoing a delete.
<li>If the document position is between Pieces, a new
TextPiece is created and inserted into the PieceList.
</ul>
<li>Create a ChangeRecord and append it to the
ChangeVector.  For an <b>insert</b>, we construct a
ChangeRecord of type <tt>InsertSpan</tt>.
<ul>
<li><tt>cr.span.m_documentOffset</tt> contains the document
position of the insertion.
<li><tt>cr.span.m_span</tt> marks the buffer position of the
text that was inserted.
<li><tt>cr.span.m_bAfter</tt> remembers whether the insertion
was before or after the document position.
</ul>
</ol>

<li>Delete(<i>position</i>,<i>bAfter</i>,<i>length</i>) -- To
<b>delete</b> one or more characters from the document (either
<i>before</i> or <i>after</i>) the absolute document position
<i>position</i>, we do the following:
<ol>
<li>Find the TextPiece that spans the document position.
<ul>
<li>If the length of characters is contained within the
TextPiece (<i>p1</i>), we split it into two TextPieces
(<i>p1a</i> and <i>pl1b</i>).  The offsets and lengths
are set in the new TextPieces such that the deleted sequence
is not in either piece.  (<i>The deleted text is not actually
deleted from the buffer; there are just no references to it
from the PieceList.</i>)
<li>If the document position is at the beginning or end of a
TextPiece, we can just adjust the offset/length, rather than
doing the split.
<li>If the deletion extends over multiple Pieces, we iterate
over each piece in the range and perform a delete on the
sub-sequence.  This will result in a multi-step ChangeRecord.
<li>TODO what about non-TextPieces??
</ul>
<li>Create a ChangeRecord and append it to the ChangeVector.
For a <b>delete</b>, we construct a ChangeRecord of type
<tt>DeleteSpan</tt>.
<ul>
<li><tt>cr.span.m_documentOffset</tt> contains the document
position of the deletion.
<li><tt>cr.span.m_span</tt> marks the buffer position of the
text that was deleted.
<li><tt>cr.span.m_bAfter</tt> remembers whether the insertion
was before or after the document position.
</ul>
</ol>

<li>InsertFormatting()
<li>ChangeFormatting()

<li>Undo -- This can be implemented using the information
in the ChangeVector.  If the CurrentPosition in the
ChangeVector is greater than zero, we have <b>undo</b>
information.  The information in the ChangeRecord prior
to the CurrentPosition is used to undo the editing
operation. After an <b>undo</b> the CurrentPosition is
decremented.

<ul>
<li>If the ChangeRecord is of type <tt>InsertSpan</tt>:
we perform a <b>delete</b> operation using
<tt>cr.span.m_documentOffset</tt>,
<tt>cr.span.m_span.m_length</tt> and
<tt>cr.span.m_bAfter</tt>.

<li>If the ChangeRecord is of type <tt>DeleteSpan</tt>:
we perform an <b>insert</b> operation using
<tt>cr.span.m_documentOffset</tt>,
<tt>cr.span.m_span</tt>, and
<tt>cr.span.m_bAfter</tt>.

<li>If the ChangeRecord is of type <tt>ChangeFormatting</tt>:
<li>If the ChangeRecord is of type <tt>InsertFormatting</tt>:
</ul>

<li>Redo -- This can be implemented using the information
in the ChangeVector.  If the CurrentPosition in the 
ChangeVector is less than the length of the ChangeVector,
the <b>redo</b> has not been invalidated and may be
applied.  The information in the ChangeRecord at the
CurrentPosition provides complete information to
describe the editing operation to be redone.  After a
<b>redo</b> the CurrentPosition is advanced.

<li>Autosave -- This can be implemented by periodically
writing the ChangeBuffer, ChangeVector, and the
ChangeAttrPropTable to temporary files.  After a 
crash, the original document and the temporary files
could be used to replay the editing operations and 
reconstruct the modified document.
</ol>

<h2>1.4. Observations</h2>

<ol>
<li>The content of the original file are never modified.
Pieces in the PieceList describe the current document;
the original content is referenced in a random access
fashion.  <i>For systems with small memory or for very
large documents, it may be worth demand loading blocks
of the original content rather than loading it completly
into the InitialBuffer</i>.

<li>Document content data (in the two buffers) are never
moved once written.  <b>insert</b> and <b>delete</b>
operations change the Pieces in the PieceList, but do
not move or change the contents of the two buffers.

<li>The result of an <b>undo</b> operation must
produce the identical document structure and content.
Since consecutive Pieces in the PieceList may have
the same formatting properties and may refer to
congituous buffer locations (there is no requirement to
coalesce them), an <b>undo</b> operation may produce a
different PieceList than we originally had prior to
doing the operation that was undone.
<ul><li>TODO Check this.  Whether the PieceList
should be identical or equivalent.</ul>
</ol>

<h2>1.5. Problems or Issues</h2>

<ol>
<li>TextPieces represent spans of text that are
convenient for the structure of the document and a
result of the sequence of editing operations.  They 
are not optimized for layout or display.
<ul>
<li>We can provide access methods to return a
<tt>const char *</tt> into the buffers along with
a length, which the caller could use in text drawing
or measuring calls, but not c-style, zero-terminated
strings.
</ul>

<li>Mapping an absolute document position to a Piece
involves a linear search of the PieceList to compute
the absolute document position and find the correct
Piece.  The number of Pieces in a document is a function
of the number of editing operations that have been
performed in the session and of the complexity of the
structure and formatting of the original document.  A
linear search might be painfully slow.
<ul>
<li><b>TODO</b> We will find a tree-structure to
use instead of the doubly-linked list that will give
us O(<i>log(n)</i>) searching.
<li><b>TODO</b> Consider caching the last few lookup
results so that we can avoid doing a search if possible.
This should have a high hit-rate when the user is
composing text.
</ul>

<li>We provide a complete, but first-order <b>undo</b>
with <b>redo</b>.
That is, we do not put the undo-operation in the undo
(like emacs).

<li>TODO The <i>before</i> and <i>after</i> stuff on
<b>insert</b> and <b>delete</b> is a bit of a hand-wave.

<li>TODO Need to add multi-step-undo so that delete operations
which span multiple pieces can be represented operation to
the user.
</ol>

<h2>1.6. Code</h2>

<pre>
class PT_PieceTable
{
	const UT_UCSChar * m_InitialBuffer;
	const UT_UCSChar * m_ChangeBuffer;
	pt_PieceList * m_pieceList;
	pt_AttrPropTable m_InitialAttrPropTable;
	pt_AttrPropTable m_ChangeAttrPropTable;
	...
};

class pt_Piece
{
	enum PieceType	{ TextPiece,
			  ObjectPiece,
			  StructurePiece };
	PieceType	m_pieceType;
	<linked-list or tree pointers>
	...
};

class pt_Span
{
	UT_Bool		m_bInInitialBuffer;
	UT_uint32	m_offset;
	UT_uint32	m_length;
};

class pt_TextPiece : public pt_Piece
{
	pt_Span			m_span;
	pt_AttrPropReference	m_apr;
	...
};

class pt_ObjectPiece : public pt_Piece
{
	...
};

class pt_StructurePiece : public pt_Piece
{
	pt_AttrPropReference	m_apr;
	...
};

class pt_PieceList
{
	<container for linked-list or tree structure>
	...
};

class pt_AttrPropReference
{
	UT_Bool		m_bInInitialTable;
	UT_uint32	m_index;
	...
};

class pt_AttrProp
{
	UT_HashTable * m_pAttributes;
	UT_HashTable * m_pProperties;
	...
};

class pt_AttrPropTable
{
	UT_vector<pt_AttrProp *>	m_Table;
	...
};

class pt_ChangeRecord
{
	UT_Bool m_bMultiStepStart;
	UT_Bool m_bMultiStepEnd;

	enum ChangeType	{ InsertSpan,
			  DeleteSpan,
			  ChangeFormatting,
			  InsertFormatting,
			  ...
			};
	struct {
		UT_uint32	m_documentOffset;
		UT_Bool		m_bAfter;
		pt_Span		m_span;
	} span;
	struct {
		UT_uint32	m_documentOffset1;
		UT_uint32	m_documentOffset2;
		pt_AttrPropReference	m_apr;
	} fmt;
	...
};

class pt_ChangeVector
{
	UT_vector	m_vecChangeRecords;
	UT_uint32	m_undoPosition;
	...
};
</pre>

*/
