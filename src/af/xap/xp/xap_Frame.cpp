
#include "ut_types.h"
#include "ut_assert.h"
#include "ap_Ap.h"
#include "ap_Frame.h"
#include "ev_EditBinding.h"
#include "ev_EditEventMapper.h"
#include "ev_EditMethod.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "ap_LoadBindings.h"
#include "ap_Menu_Layouts.h"
#include "ap_Menu_LabelSet.h"


#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

AP_Frame::AP_Frame(AP_Ap * ap)
{
	m_ap = ap;

	m_pDoc = NULL;
	m_pView = NULL;
	m_pEBM = NULL;
	m_pEEM = NULL;
	m_pMenuLayout = NULL;
	m_pMenuLabelSet = NULL;
}

AP_Frame::~AP_Frame(void)
{
	// only delete the things that we created...

	DELETEP(m_pEBM);
	DELETEP(m_pEEM);
	DELETEP(m_pMenuLayout);
	DELETEP(m_pMenuLabelSet);
}

UT_Bool AP_Frame::initialize(int argc, char ** argv)
{
	UT_Bool bResult;

	// choose which set of key- and mouse-bindings to load
	
	char * szBindings = "default";
	// TODO override szBindings from argc,argv.
	bResult = AP_LoadBindings(szBindings,m_ap->getEditMethodContainer(),&m_pEBM);
	UT_ASSERT(bResult && (m_pEBM != NULL));

	// create a EventMapper state-machine to process our events

	m_pEEM = new EV_EditEventMapper(m_pEBM);
	UT_ASSERT(m_pEEM);

	// select which menu bar we should use

	char * szMenuLayout = "Main";
	// TODO override szMenuLayout using argc,argv
	m_pMenuLayout = AP_CreateMenuLayout(szMenuLayout);
	UT_ASSERT(m_pMenuLayout);

	// select language for menu labels
	char * szLanguage = "EnUS";
	// TODO override szLanguage using argc,argv
	m_pMenuLabelSet = AP_CreateMenuLabelSet(szLanguage);
	UT_ASSERT(m_pMenuLabelSet);

	// ... add other stuff here ...

	return UT_TRUE;
}
