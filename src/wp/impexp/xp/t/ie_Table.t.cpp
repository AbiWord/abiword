#include "tf_test.h"
#include "ie_Table.h"

#define TFSUITE "core.wp.impexp.table"


class TF_Document
  : public PD_Document
{
public:
  ~TF_Document() {};
};

static const char *DATA_FILE =
  "/test/wp/mailmerge/data.csv";


TFTEST_MAIN("ie Table")
{
  {
    std::string data_file;
    TFPASS(TF_Test::ensure_test_data(DATA_FILE, data_file));

    TF_Document doc;

    UT_Error err = doc.readFromFile(data_file.c_str(), IEFT_Unknown, NULL);
    TFPASS(err == UT_OK);

    ie_Table table;
    table.setDoc(&doc);

  }
}
