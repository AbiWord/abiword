#include "xap_EncodingManager.h"

XAP_EncodingManager *XAP_EncodingManager::get_instance()
{
	if (_instance == 0)
		_instance = new XAP_EncodingManager();

	return _instance;
}
