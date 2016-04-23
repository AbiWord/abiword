#include "tf_test.h"
#include "px_CR_Strux.h"
#include "ie_Table.h"

#define TFSUITE "core.wp.impexp.table"


namespace {

struct TF_TableDesc
{
  int numCols;
  int numRows;
};

class TF_TableListener
  : public PL_Listener
{
public:
  TF_TableListener(PD_Document* pDoc, ie_Table & tableHelper)
    :m_tableHelper(tableHelper)
    {
      tableHelper.setDoc(pDoc);
    }

  virtual bool populate(fl_ContainerLayout* /*sfh*/,
                        const PX_ChangeRecord * /*pcr*/)
    {
      return true;
    }

  virtual bool populateStrux(pf_Frag_Strux* sdh,
                             const PX_ChangeRecord* pcr,
                             fl_ContainerLayout** /*psfh*/)
    {
      if(pcr->getType() != PX_ChangeRecord::PXT_InsertStrux) {
        return false;
      }

      const PX_ChangeRecord_Strux * pcrx =
        static_cast<const PX_ChangeRecord_Strux *> (pcr);
      PT_AttrPropIndex api = pcr->getIndexAP();

      switch (pcrx->getStruxType())
      {
      case PTX_SectionTable:
        m_tableHelper.openTable(sdh, api);
        break;
      case PTX_SectionCell:
        m_tableHelper.openCell(api);
        break;
      case PTX_EndTable:
        TF_TableDesc t;
        memset(&t, 0, sizeof(t));
        t.numCols = m_tableHelper.getNumCols();
        t.numRows = m_tableHelper.getNumRows();
        m_tables.push_back(t);
        m_tableHelper.closeTable();
        break;
      case PTX_EndCell:
        m_tableHelper.closeCell();
        break;
      default:
        break;
      }

      return true;
    }

  virtual bool change(fl_ContainerLayout* /*sfh*/,
                      const PX_ChangeRecord * /*pcr*/)
    {
      return true;
    }

  virtual bool insertStrux(fl_ContainerLayout* /*sfh*/,
                           const PX_ChangeRecord * /*pcr*/,
                           pf_Frag_Strux* /*sdh*/,
                           PL_ListenerId /*lid*/,
                           void (* /*pfnBindHandles*/) (pf_Frag_Strux* sdhNew,
                                                   PL_ListenerId lid,
                                                   fl_ContainerLayout* sfhNew))
    {
      return true;
    }

  virtual bool signal(UT_uint32 /*iSignal*/)
    {
      return true;
    }

  const std::vector<TF_TableDesc> & tables() const
    {
      return m_tables;
    }
private:
  ie_Table & m_tableHelper;
  std::vector<TF_TableDesc> m_tables;
};

static const char *DATA_FILE =
  "/test/wp/table.abw";

}

TFTEST_MAIN("ie Table")
{
  {
    std::string data_file;
    TFPASS(TF_Test::ensure_test_data(DATA_FILE, data_file));

    PD_Document* doc = new PD_Document;

    UT_Error err = doc->readFromFile(data_file.c_str(), IEFT_Unknown, NULL);
    TFPASSEQ(err, UT_OK);

    {
      ie_Table table;
      TF_TableListener listener(doc, table);

      bool status = doc->tellListener(&listener);

      TFPASS(status);

      const std::vector<TF_TableDesc> & tables = listener.tables();

      TFPASSEQ(tables.size(), 2);

      if (tables.size() >= 2) {
        // if the test fails we don't want to crash here.
        // XXX figure out a better way to deal with this.
        TFPASSEQ(tables[0].numCols, 4);
        TFPASSEQ(tables[0].numRows, 3);

        TFPASSEQ(tables[1].numCols, 5);
        TFPASSEQ(tables[1].numRows, 2);
      }

      TFPASSEQ(table.getNumRows(), 0);
      TFPASSEQ(table.getNumCols(), 0);
    }

    doc->unref();
  }
}
