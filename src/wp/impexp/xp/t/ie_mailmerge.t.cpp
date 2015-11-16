

#include "tf_test.h"
#include "ie_mailmerge.h"

#define TFSUITE "core.wp.impexp.mailmerge"

class IE_MergeSniffer_TH
{
public:
};

class IE_MailMerge_TH
{
public:
  static std::vector<IE_MergeSniffer *> & getSniffers()
    {
      return IE_MailMerge::getSniffers();
    }

};

TFTEST_MAIN("IE Mail merge sniffers")
{

  std::vector<IE_MergeSniffer *> & sniffers = IE_MailMerge_TH::getSniffers();

  TFPASS(sniffers.size() == 3);

  IE_MergeSniffer *sniffer = sniffers[0];

  TFPASS(sniffer);
  TFPASS(sniffer->getFileType() == 1);

  TFPASS(sniffer->supportsFileType(1));
}


static const char *DATA_FILE =
  "/test/wp/mailmerge/data.csv";

TFTEST_MAIN("IE Mail merger")
{
  std::string data_file;
  TFPASS(TF_Test::ensure_test_data(DATA_FILE, data_file));

  IE_MailMerge *pie = 0;
  UT_Error err = IE_MailMerge::constructMerger(data_file.c_str(),
                                               IEMT_Unknown, &pie);

  TFPASSEQ(err, UT_OK);
  TFPASS(pie);

  std::vector<std::string> headers;
  err = pie->getHeaders(data_file.c_str(), headers);
  TFPASSEQ(err, UT_OK);
  TFPASSEQ(headers.size(), 6);
  if (!headers.empty()) {
    TFPASS(headers[0] == "FirstName");
  } else {
    TFFAIL(true);
  }
}
