/* Copyright (C) 2006,2007 Marc Maurer <uwog@uwog.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "ServiceAccountHandler.h"

/****************************************
 DocTreeItem
*****************************************/
/*DocTreeItem::DocTreeItem()
	: m_id(0),
	m_name(""),
	m_type(DOCTREEITEM_TYPE_NILL), 
	m_acl(DOCTREEITEM_ACL_PRIVATE), 
	m_data(0)
{
}

DocTreeItem::DocTreeItem(UT_sint64 id, const char* name, DocTreeItemType type, DocTreeItemAcl acl)
	: m_id(id),
	m_name(name),
	m_type(type), 
	m_acl(acl), 
	m_data(0)
{
}*/

ServiceAccountHandler::ServiceAccountHandler()
	: AccountHandler()
{
}

ServiceAccountHandler::~ServiceAccountHandler()
{
	disconnect();
}

UT_UTF8String ServiceAccountHandler::getDescription()
{
	return UT_UTF8String_sprintf("AbiWord Collaboration Service");
}

UT_UTF8String ServiceAccountHandler::getDisplayType()
{
	return "AbiWord Collaboration Service";
}

UT_UTF8String ServiceAccountHandler::getStorageType()
{
	return "com.abisource.abiword.abicollab.backend.service";
}

void ServiceAccountHandler::storeProperties()
{
	UT_DEBUGMSG(("ServiceAccountHandler::storeProperties() - TODO: implement me\n"));
}

ConnectResult ServiceAccountHandler::connect()
{
	UT_DEBUGMSG(("ServiceAccountHandler::connect()\n"));
	
	return CONNECT_INTERNAL_ERROR;
}

bool ServiceAccountHandler::disconnect()
{
	UT_DEBUGMSG(("ServiceAccountHandler::disconnect() - TODO: implement me\n"));
	return true;
}

bool ServiceAccountHandler::isOnline()
{
	// TODO: implement me
	return false;
}

Buddy* ServiceAccountHandler::constructBuddy(const PropertyMap& props)
{
	UT_DEBUGMSG(("ServiceAccountHandler::constructBuddy() - TODO: implement me\n"));

	return NULL; // TODO: implement me
}

bool ServiceAccountHandler::send(const Packet* packet)
{
	UT_DEBUGMSG(("ServiceAccountHandler::send(const Packet*)\n"));
	return false;
}

bool ServiceAccountHandler::send(const Packet*, const Buddy& buddy)
{
	UT_DEBUGMSG(("ServiceAccountHandler::send(const Packet*, const Buddy& buddy)\n"));
	return false;
}

/*
void s_parse_document(xmlNode* document, DocTreeItem* parent)
{
	if (!document || !parent)
		return;

	if (document->type == XML_ELEMENT_NODE)
	{
		DocTreeItem* item = new DocTreeItem();
		// parse the document properties
		for (xmlNode* prop = document->children; prop; prop = prop->next)
		{
			if (prop->type == XML_TEXT_NODE && prop->next)
			{
				prop = prop->next;
				if (prop->type == XML_ELEMENT_NODE)
				{
					if (strcmp(reinterpret_cast<const char*>(prop->name), "id") == 0)
					{
						UT_sint32 id = atoi(reinterpret_cast<const char*>(xmlNodeGetContent(prop)));
						if (id > 0)
							item->m_id = id;
					}
					else if (strcmp(reinterpret_cast<const char*>(prop->name), "name") == 0)
					{
						item->m_name = reinterpret_cast<const char*>(xmlNodeGetContent(prop));
					}
					else if (strcmp(reinterpret_cast<const char*>(prop->name), "readonly") == 0)
					{
						// TODO: ...
					}
					else if (strcmp(reinterpret_cast<const char*>(prop->name), "acl") == 0)
					{
						// TODO: ...
					}
					else
					{
						printf("unknown prop name: %s\n", prop->name);
						UT_ASSERT_HARMLESS(false);
						continue;
					}
				}
				else
				{
					UT_DEBUGMSG(("Unexpected element in document - type: %d, name: %s", prop->type, prop->name));
					UT_ASSERT_HARMLESS(false);
				}
			}
		}
		parent->m_vecChildren.push_back(item);
	}
}

void s_parse_folder(xmlNode* folder, DocTreeItem* parent)
{
	if (!folder || !parent)
		return;

	if (folder->type == XML_ELEMENT_NODE)
	{
		const char* name = reinterpret_cast<const char*>(xmlGetProp(folder, reinterpret_cast<const xmlChar*>("name")));
		if (!name)
		{
			name = "Unknown folder";
		}
		// TODO: get a folderId here (but it is not in the XML atm)
		DocTreeItem* item = new DocTreeItem(0, name, DOCTREEITEM_TYPE_FOLDER, DOCTREEITEM_ALC_NILL);
		// folders can contain documents or folders
		xmlNode *folder_child = NULL;
		for (folder_child = folder->children; folder_child; folder_child = folder_child->next)
		{
			if (folder_child->type == XML_ELEMENT_NODE)
			{
				if (strcmp(reinterpret_cast<const char*>(folder_child->name), "document") == 0)
				{
					s_parse_document(folder_child, item);
				}
				else if (strcmp(reinterpret_cast<const char*>(folder_child->name), "folder") == 0)
				{
					s_parse_folder(folder_child, item);
				}
			}
		}
		parent->m_vecChildren.push_back(item);
	}
}

void s_parse_group_elements(xmlNode* group, DocTreeItem* parent)
{
	xmlNode *group_element = NULL;
	for (group_element = group->children; group_element; group_element = group_element->next)
	{
		if (group_element->type == XML_ELEMENT_NODE)
		{
			if (strcmp(reinterpret_cast<const char*>(group_element->name), "document") == 0)
			{
				s_parse_document(group_element, parent);
			}
			else if (strcmp(reinterpret_cast<const char*>(group_element->name), "folder") == 0)
			{
				s_parse_folder(group_element, parent);
			}
		}
	}

}

void s_parse_groups(xmlNode* root, std::vector<DocTreeItem*>& vecDocTreeItems)
{
	xmlNode *group = NULL;
	for (group = root->children; group; group = group->next)
	{
		if (group->type == XML_ELEMENT_NODE)
		{
			UT_UTF8String prettyName;
			if (strcmp(reinterpret_cast<const char*>(group->name), "owner") == 0)
			{
				prettyName = "My Documents";
			}
			else if (strcmp(reinterpret_cast<const char*>(group->name), "friends") == 0)
			{
				prettyName = "My Friends' Documents";
			}
			else if (strcmp(reinterpret_cast<const char*>(group->name), "groups") == 0)
			{
				prettyName = "My Groups' Documents";
			}
			else
			{
				UT_ASSERT_HARMLESS(false);
				continue;
			}

			// First param on the next line is unused for groups
			DocTreeItem* item = new DocTreeItem(0, prettyName.utf8_str(), DOCTREEITEM_TYPE_ROOT, DOCTREEITEM_ALC_NILL);
			vecDocTreeItems.push_back(item);
			s_parse_group_elements(group, item);
		}
	}
}*/

void ServiceAccountHandler::populateDocuments(const UT_UTF8String& packet)
{
/*	m_vecDocTreeItems.clear(); // TODO: we have to free all items

	UT_DEBUGMSG(("Incoming packet:\n%s\n", packet.utf8_str()));

    xmlNode *root_element = NULL;
	xmlDocPtr doc = xmlReadMemory (packet.utf8_str(), packet.length(), 0, "UTF-8", 0);
    if (doc == NULL)
	{
		UT_DEBUGMSG(("Error parsing document XML tree!\n"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}
	
	root_element = xmlDocGetRootElement(doc);
	if (root_element)
	{
		// we only expect 1 root element (GetDocumentResponse)
		UT_ASSERT(root_element->next == 0);
		s_parse_groups(root_element, m_vecDocTreeItems);
	}
	xmlCleanupParser();
	xmlFreeDoc(doc);
	
	// signal the listeners we have (atm, that is only the Documents dialog)
	if (m_pDocDialog)
	{
		m_pDocDialog->repopulate();
	}*/
}

void ServiceAccountHandler::updateDocumentsAsync()
{
/*	UT_return_if_fail(m_pHandler);

	m_pHandler->send(
			UT_UTF8String("<GetDocumentsRequest/>"), 
			getAbiterString()
	);
	*/
}

void ServiceAccountHandler::openDocumentAsync(UT_sint64 docId)
{
/*	UT_return_if_fail(m_pHandler);
	char buf[24]; // ick
	snprintf(buf, sizeof(buf), "%d", docId);

	UT_UTF8String odr;
	odr += "<OpenDocumentRequest>";
	odr += "<docId>";
	odr += &buf[0];
	odr += "</docId>";
	odr += "</OpenDocumentRequest>";
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	pFrame->setCursor(GR_Graphics::GR_CURSOR_WAIT);
	m_pHandler->send(odr, getAbiterString());*/
}


void ServiceAccountHandler::saveDocumentAsync(UT_sint64 docId, PD_Document * pDoc)
{
/*	UT_return_if_fail(m_pHandler); // TODO: notify the user

	UT_UTF8String odr;
	odr += "<SaveDocumentRequest>";

	UT_UTF8String document;
	bool b = s_CollabFactoryContainer.fillDocumentString(document, pDoc);
	UT_return_if_fail(b); // TODO: notify the user

	odr += "<document id=";
	UT_UTF8String sNum =  UT_UTF8String_sprintf("\"%d\"",docId);
	odr += sNum;
	odr += ">";
    odr += document;
    odr += "</document>";

    odr += "</SaveDocumentRequest>";
    UT_UTF8String sCAC = getAbiterString();
    m_pHandler->send(odr, sCAC.utf8_str());*/
}

void ServiceAccountHandler::closeDocument(UT_sint64 docId, PD_Document * pDoc)
{
/*	UT_return_if_fail(m_pHandler);

	UT_UTF8String odr;
	odr += "<CloseDocumentRequest>\n";
	odr += "<docId>";
	odr += UT_UTF8String_sprintf("%d",docId);
	odr += "</docId>\n";
	odr += "</CloseDocumentRequest>";
	UT_UTF8String sCAC = getAbiterString();
	UT_DEBUGMSG(("sending |%s| \n to |%s| \n",odr.utf8_str(),sCAC.utf8_str()));
	m_pHandler->send(odr, sCAC.utf8_str());*/
}

/*

void  s_handle_service_import_event(UT_Worker * pWorker)
{
	AbiCollab* pCollab = reinterpret_cast<AbiCollab*>(pWorker->getInstanceData());
	pCollab->handleImportEvent();
}

static bool	s_handle_service_message(const AbiCollab_Packet_Data *pAPD)
{
	AbiCollabService* pService = AbiCollabService::getService();
	UT_return_val_if_fail(pService, false);
    
	xmlDocPtr doc = xmlReadMemory (pAPD->packet.utf8_str(), pAPD->packet.length(), 0, "UTF-8", 0);
	UT_return_val_if_fail(doc, false);
	UT_DEBUGMSG(("Received service packet\n"));
	xmlNode *response_element = xmlDocGetRootElement(doc);
	if (response_element)
	{
		// we only expect 1 root element (GetDocumentResponse)
		UT_ASSERT(response_element->next == 0);
		
		UT_DEBUGMSG(("got resp: %s\n", reinterpret_cast<const char*>(response_element->name)));
		
		if (strcmp(reinterpret_cast<const char*>(response_element->name), "GetDocumentsResponse") == 0)
		{
			// FIXME FIXME: don't re-parse!!!
			pService->populateDocuments(pAPD->packet);
		}
		else if(strcmp(reinterpret_cast<const char*>(response_element->name), "SaveDocumentResponse") == 0)
		{
		        UT_DEBUGMSG(("SaveResponsePacked |%s| \n",pAPD->packet.utf8_str()));
			bool bSavedOK = false;
			for (xmlNode* document_node = response_element->children; document_node; document_node = document_node->next)
			{
				UT_DEBUGMSG(("Document_node type %d \n",document_node->type));
				if (document_node->type == XML_ELEMENT_NODE)
				{
					UT_DEBUGMSG(("Inside Document_node name |%s| \n",document_node->name));
					if (strcmp(reinterpret_cast<const char*>(document_node->name),"status") == 0)
					{
					    UT_DEBUGMSG(("Found Status \n"));
					    const char* statusVal = reinterpret_cast<const char*>(xmlNodeGetContent(document_node));
					    if(statusVal)
					    {
						UT_UTF8String sStatusVal =statusVal;
						FREEP(statusVal);
						if(strcmp(sStatusVal.utf8_str(),"success")==0)
						{
						    bSavedOK = true;
						}
					    }
					    else
					    {
						break;
					    }
					}
				}
			}
			if(!bSavedOK)
			{
			    XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
			    pFrame->showMessageBox(szFailedToSave,
						   XAP_Dialog_MessageBox::b_O,
						   XAP_Dialog_MessageBox::a_OK);

			}
			else
			{
			    XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
			    PD_Document* pDoc = static_cast<PD_Document *>(pFrame->getCurrentDoc());
			    FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
			    pDoc->setClean();
			    pView->notifyListeners(AV_CHG_SAVE);
			}
		}
		else if (strcmp(reinterpret_cast<const char*>(response_element->name), "OpenDocumentResponse") == 0)
		{
			for (xmlNode* document_node = response_element->children; document_node; document_node = document_node->next)
			{
				UT_DEBUGMSG(("Outside Document_node name |%s| \n",document_node->name));
				UT_DEBUGMSG(("Document_node type %d \n",document_node->type));
				if (document_node->type == XML_ELEMENT_NODE)
				{
					UT_DEBUGMSG(("Inside Document_node name |%s| \n",document_node->name));
					if (strcmp(reinterpret_cast<const char*>(document_node->name),"document") == 0)
				  	{
						UT_UTF8String sID = reinterpret_cast<char *>(xmlGetProp(document_node, reinterpret_cast<const xmlChar*>("id")));
						UT_DEBUGMSG(("got document!!!!! %s id = %s \n", document_node->name,sID.utf8_str()));
						UT_sint64 iID = atoi(sID.utf8_str());
						//printf("doc content:\n%s\n", reinterpret_cast<const char*>(xmlNodeGetContent(document_node)));
					
						// TODO: move this out
						//
						// Get Document name
						//
						const char *szDocname = reinterpret_cast<char *>(xmlGetProp(document_node, reinterpret_cast<const xmlChar*>("name")));					
						const char* base64gzBuf = reinterpret_cast<const char*>(xmlNodeGetContent(document_node));
						//printf("from xml: %s\n", base64gzBuf);
						size_t gzbufLen = gsf_base64_decode_simple((guint8*)base64gzBuf, strlen(base64gzBuf));
						char* gzBuf = (char*) base64gzBuf;
						// TODO: do we need to pass false here?
						GsfInput *source = gsf_input_memory_new((guint8*)gzBuf, gzbufLen, false);
						GsfInput *gzabwBuf = gsf_input_gzip_new (source, NULL); // todo: don't pass null here, but check for errors
                                                // unzip the document
						gsf_off_t abwLen = gsf_input_size(gzabwBuf);
						char* abwBuf = (char*)malloc(abwLen+1);
						gsf_input_seek(gzabwBuf, 0, G_SEEK_SET);
						gsf_input_read(gzabwBuf, abwLen, (guint8*)abwBuf);
						abwBuf[abwLen] = '\0';

						XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
						if(!pFrame || pFrame->isDirty() || (pFrame->getFilename() != NULL))
						{
							pFrame = XAP_App::getApp()->newFrame();
							if (!pFrame)
							{
								return false;
							}

							// Open a complete but blank frame, then load the document into it

							UT_Error errorCode = pFrame->loadDocument(NULL, IEFT_Unknown);
							if (!errorCode)
							{
					    			pFrame->show();
							}
							else
							{
								return false;
							}
						}
						PD_Document* pDoc = new PD_Document(XAP_App::getApp());
						pDoc->createRawDocument();
						IE_Imp_XML* imp = (IE_Imp_XML*)new IE_Imp_AbiWord_1(pDoc);
						imp->importFile(abwBuf, abwLen); // todo: check for errors
						pDoc->finishRawCreation();
						pFrame->loadDocument(pDoc); 
						FREEP(abwBuf);
						DELETEP(imp);

						g_object_unref(G_OBJECT(source));
						AbiCollabService_Export * pExport = new AbiCollabService_Export(pDoc,iID);
						UT_uint32 lid = 0;
						pDoc->addListener(static_cast<PL_Listener *>(pExport), &lid);
						UT_UTF8String sUTF8("");
						AbiCollab* pCollab = s_CollabFactoryContainer.create(0, sUTF8, false, NULL);
						const UT_UTF8String sServer(CAC_SERVER);
						bool b = pCollab->createHandler(sServer,5222,s_UserName,s_Password,NULL);
						if(!b)
						  return false;
						pCollab->setDocument(pDoc);
						pCollab->setServiceID(iID);
						pCollab->setOffering(true);
						pCollab->startRemoteListener();
						if(szDocname)
						{
						    UT_DEBUGMSG(("Docname %s \n",szDocname));
						    pDoc->setFilename(const_cast<char *>(szDocname));
						}
						pFrame->updateTitle();
						UT_DEBUGMSG(("We are offering a collaboration from a c.a.c doc!\n"));
					}
					else if(strcmp(reinterpret_cast<const char*>(document_node->name),"host")==0)
					{
						UT_DEBUGMSG(("Host_node name |%s| \n",document_node->name));
						UT_UTF8String sID = reinterpret_cast<char *>(xmlGetProp(document_node, reinterpret_cast<const xmlChar*>("docId")));
						UT_sint64 iID = atoi(sID.utf8_str());
						const char* szName =  reinterpret_cast<const char*>(xmlNodeGetContent(document_node));
						//
						// strip off the resource
						//
						UT_String sName = szName;
						UT_sint32 i = 0;
						for(i=0; i<sName.size();i++)
						{
						     if(sName[i] == '/')
						       break;
						}
						UT_UTF8String sHostUsername = szName;
						if(i<sName.size())
							sHostUsername=sName.substr(0,i).c_str();
						FREEP(szName);
						UT_DEBUGMSG(("got name !!!!! %s id = %d \n", sHostUsername.utf8_str(),iID));
						UT_UTF8String sUTF8("");
						AbiCollab* pCollab = s_CollabFactoryContainer.create(0, sUTF8, false, NULL);
						const UT_UTF8String sServer(CAC_SERVER);
						bool b = pCollab->createHandler(sServer,5222,s_UserName,s_Password,sHostUsername.utf8_str());
						if(!b)
							return false;
						pCollab->setServiceID(iID);
						//
						// Need to attach a CAC listener to the
						// document after it has been loaded.
						// We'll create one here and make
						// AbiCollab finish the attachments after
						// loading the remote doc
						//
						pCollab->setExportServiceListener(static_cast<PL_DocChangeListener *>(new AbiCollabService_Export(NULL,iID)));
						pCollab->startRemoteListener();
					}
				}
			}
		}
		else
		{
		  //
		  // Authorise a remote user to join the collaboration
		  //
		}
	}
	//
	// TODO connect to jabber server and offer this document for
	// collaboration
	//
	xmlFreeDoc(doc);
}


static bool s_handle_service_event(void* data)
{
	AbiCollabService* pService = AbiCollabService::getService();
	UT_return_val_if_fail(pService, false);
	AbiCollab_ConnectionHandler* pHandler = pService->getConnectionHandler();
	if(pHandler == NULL)
	{
	    return false;
	}
	UT_return_val_if_fail(pHandler, false);


	if (pHandler->receive().size() > 0)
	{
		for (std::list<const AbiCollab_Packet_Data *>::iterator lpos = pHandler->receive().begin(); lpos != pHandler->receive().end(); )
		{
			const AbiCollab_Packet_Data* pAPD = pHandler->receive().front();
			lpos = pHandler->receive().erase(lpos);
			if (!pAPD)
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				break;
			}
				
			// TODO: Check if this message really came from abiter@collaborate.abisource.com/AbiCollabService
			//       If not, we'll ignore it. We don't want people to flood us with fake content
			// TODO: Don't hardcode the abiter/server/resource names
			// TODO: there are more message types than this one :)
			if (strcmp("abiter@collaborate.abisource.com/AbiCollabService", pAPD->buddy.utf8_str()) == 0)
			{
				s_handle_service_message(pAPD);
			}
			else
			{
				// we've received a service message from someone other than the official
				// arbiter; ignore it;
				UT_DEBUGMSG(("Received message from |%s|, content |%s| \n",pAPD->buddy.utf8_str(), pAPD->packet.utf8_str()));
				UT_DEBUGMSG(("Can't handle this!!!! \n"));

				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		}
	}

	return true;
}
*/
