
#include "ut_Rehydrate.h"
#include <stdio.h>
#include <Be.h>

bool RehydrateWindow(char *name, BMessage *tempMsg) {
	BResources res; 
	app_info info;
	char *object;
	size_t size;

	be_app->GetAppInfo(&info);
	/*Make a BFile reference to the executable where resource lives*/
	BFile appfile(&info.ref, B_READ_ONLY);
	if(appfile.InitCheck() != B_NO_ERROR)    		//Check the appfile
 		return(false);
	
	/*Refer the BResource object to the BFile reference*/
 	if (res.SetTo(&appfile) != B_NO_ERROR) {		//Check the resource
 		printf("This is not a resource file \n");
		return(false);
 	}
 	
 	/*Actually locate the resource item which you have stored.
 	  This is done by using the index ('ARGC') and the name 
 	  (mainWindow) of the resource item.
 	  For windows it seems that you must keep the index to ARCV,
 	  but the name can be anything that you like */
 	object = (char*) res.FindResource('ARCV', name, &size);
	if (!object) {									//Check the pointer
		printf("Can't find window %s in resource\n", name);
 		return(false);
	}

	/*Now unflatten the object into a BMessage that we can pass
	  to our window class constructor. */  
	if (tempMsg->Unflatten(object) != B_OK) {		//Check the message
		printf("Can't unflatten message\n");
 		return(false);
	}

	/*Final stage in the re-hydration process, actuall call the
	  window constructor of our derived class with the BMessage
	  that has the unflattened resource in it.
	*/
	return(true);
} 

