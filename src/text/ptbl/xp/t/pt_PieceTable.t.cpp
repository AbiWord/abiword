

#include "tf_test.h"
#include "pt_PieceTable.h"
#include "pd_Document.h"


// FIXME write real test
TFTEST_MAIN("pt_PieceTable")
{
//	PD_Document doc;
	pt_PieceTable pt(NULL);

	TFPASS(pt.getDocument() == NULL);
}
