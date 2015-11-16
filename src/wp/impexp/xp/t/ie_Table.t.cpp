#include "tf_test.h"
#include "ie_Table.h"

#define TFSUITE "core.wp.impexp.table"


class TF_Document
  : public PD_Document
{
public:
  ~TF_Document();
};


TFTEST_MAIN("ie Table")
{
  {
    TF_Document doc;
    ie_Table table;
    table.setDoc(&doc);

    
  }
}
