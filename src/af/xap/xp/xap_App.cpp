#include "ut_types.h"
#include "ut_assert.h"
#include "ev_EditMethod.h"
#include "ev_Menu_Actions.h"
#include "ap_Ap.h"
#include "ap_EditMethods.h"
#include "ap_Menu_ActionSet.h"


#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

AP_Ap::AP_Ap(void)
{
	m_pEMC = NULL;
	m_pMenuActionSet = NULL;
}

AP_Ap::~AP_Ap(void)
{
	DELETEP(m_pEMC);
	DELETEP(m_pMenuActionSet);
}

UT_Bool AP_Ap::initialize(int argc, char ** argv)
{
	// create application-wide resources that
	// are shared by everything.

	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	// TODO use argc,argv to process any command-line
	// TODO options that we need.

	return UT_TRUE;
}

EV_EditMethodContainer * AP_Ap::getEditMethodContainer(void) const
{
	return m_pEMC;
}

const EV_Menu_ActionSet * AP_Ap::getMenuActionSet(void) const
{
	return m_pMenuActionSet;
}


