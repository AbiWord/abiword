
#include "ap_UnixAp.h"
#include "ap_UnixFrame.h"

int main(int argc, char ** argv)
{
	// initialize our application.

	AP_UnixAp * pMyUnixAp = new AP_UnixAp();
	pMyUnixAp->initialize(&argc,&argv);

	// create the first window.

	AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(pMyUnixAp);
	pFirstUnixFrame->initialize(&argc,&argv);
	
	// TODO fish thru argv and see if there's a filename.
	// TODO for now, we just take argv[1] as a document name.

	pFirstUnixFrame->loadDocument(argv[1]);

	// turn over control to gtk

	gtk_main();

	// destroy the Ap.  It should take care of deleting all frames.

	delete pMyUnixAp;
	
	return 0;
}
