/* Copyright (C) 2007 One Laptop Per Child
 * Copyright (C) 2009 Marc Maurer <uwog@uwog.net>
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

#include <account/xp/Event.h>
#include <account/xp/AccountEvent.h>
#include <session/xp/AbiCollabSessionManager.h>
#include <session/xp/AbiCollab.h>
#include <ev_EditMethod.h>
#include <xap_App.h>
#include <fv_View.h>
#include <xap_Frame.h>
#include <xap_UnixApp.h>

extern "C"
{
#include <telepathy-glib/dbus.h>
#include <telepathy-glib/connection.h>
#include <telepathy-glib/util.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/contact.h>

#include <libmissioncontrol/mission-control.h>
	
#include <libempathy/empathy-utils.h>
#include <libempathy-gtk/empathy-account-chooser.h>
#include <libempathy-gtk/empathy-ui-utils.h>
}
	
#include "DTubeUnixAccountHandler.h"
#include "DTubeBuddy.h"

static DBusHandlerResult s_dbus_handle_message(DBusConnection *connection, DBusMessage *message, void *user_data);

#define INTERFACE "com.abisource.abiword.abicollab.olpc"
#define SEND_ALL_METHOD "SendAll"
#define SEND_ONE_METHOD "SendOne"

DTubeAccountHandler* DTubeAccountHandler::m_pHandler = NULL;
DTubeAccountHandler* DTubeAccountHandler::getHandler() { return m_pHandler; }

static bool
is_usable_connection(TpConnection* conn)
{
	GValue *value;
	
	// check if muc tubes are implemented for this connection
	// TODO: do we really need muc tubes? all our communication is 1-1 basically...
	if (tp_cli_dbus_properties_run_get (conn, -1,
				TP_IFACE_CONNECTION_INTERFACE_REQUESTS,
				"RequestableChannelClasses", &value, NULL, NULL))
	{
		GPtrArray *classes = (GPtrArray *) g_value_get_boxed (value);
		for (guint i = 0; i < classes->len; i++)
		{
			GValue class_ = {0,};
			//GValue * chan_type;
			GValue * handle_type;			
			GHashTable *fixed_prop;

			g_value_init (&class_, TP_STRUCT_TYPE_REQUESTABLE_CHANNEL_CLASS);
			g_value_set_static_boxed (&class_, g_ptr_array_index (classes, i));

			dbus_g_type_struct_get (&class_, 0, &fixed_prop, G_MAXUINT);

			/* we don't use stream tubes, so do not check for those
			chan_type = (GValue *) g_hash_table_lookup (fixed_prop, TP_IFACE_CHANNEL ".ChannelType");
			if (chan_type == NULL || tp_strdiff (g_value_get_string (chan_type),
						"org.freedesktop.Telepathy.Channel.Type.StreamTube.DRAFT"))
			{
				continue;
			}
			 */

			handle_type = (GValue *) g_hash_table_lookup (fixed_prop, TP_IFACE_CHANNEL ".TargetHandleType");
			if (handle_type == NULL || g_value_get_uint (handle_type) != TP_HANDLE_TYPE_ROOM)
				 continue;

			return true;
		}
	}

	return false;
}

static void
tp_connection_contacts_by_handle_cb(TpConnection * /*connection*/,
			guint n_contacts,
			TpContact * const *contacts,
			guint /*n_failed*/,
			const TpHandle * /*failed*/,
			const GError * /*error*/,
			gpointer user_data,
			GObject * /*weak_object*/)
{
	UT_DEBUGMSG(("tp_connection_contacts_by_handle_cb()\n"));
	DTubeAccountHandler* pHandler = reinterpret_cast<DTubeAccountHandler*>(user_data);
	UT_return_if_fail(pHandler);
	pHandler->getBuddiesAsync_cb(n_contacts, contacts);
}

static void
tp_connection_get_contacts(TpConnection* conn, DTubeAccountHandler* pHandler)
{
	UT_DEBUGMSG(("tp_connection_get_contacts()\n"));
	UT_return_if_fail(conn);

	GPtrArray *channels;
	// FIXME: tp_cli_connection_run_list_channels is deprecated
	// FIXME: only use those channels where TargetHandleType is LIST and TargetID is "stored"
	tp_cli_connection_run_list_channels (conn, -1, &channels, NULL, NULL);
	printf("Number of connection channels: %d\n", channels->len);
	for (guint i = 0; i < channels->len; i++)
	{
		GValueArray  *chan_struct;
		const gchar  *object_path;
		const gchar  *channel_type;
		TpHandleType  handle_type;
		guint         handle;
		TpChannel *chan;

		chan_struct = (GValueArray *) g_ptr_array_index (channels, i);
		object_path = (const gchar *) g_value_get_boxed (g_value_array_get_nth (chan_struct, 0));
		channel_type = g_value_get_string (g_value_array_get_nth (chan_struct, 1));
		handle_type = (TpHandleType) g_value_get_uint (g_value_array_get_nth (chan_struct, 2));
		handle = g_value_get_uint (g_value_array_get_nth (chan_struct, 3));

		UT_DEBUGMSG(("Object path: %s\n", object_path));
		UT_DEBUGMSG(("Channel type: %s\n", channel_type));

		// FIXME: we're only interested in the 'stored' contact list, not in the publis or subscribe
		// one... surely we can filter on it, but I don't know how - MARCM
		
		if (handle_type != TP_HANDLE_TYPE_LIST || tp_strdiff (channel_type, TP_IFACE_CHANNEL_TYPE_CONTACT_LIST))
		{
			UT_DEBUGMSG(("This channel is not a contact list, skipping...\n"));
			continue;
		}

		// get the active member 'IDs' from the channel
		chan = tp_channel_new (conn, object_path, channel_type, handle_type, handle, NULL);
		tp_channel_run_until_ready (chan, NULL, NULL); // bad bad bad, should be _call_when_read (according to the TP people)
		const TpIntSet * set = tp_channel_group_get_members (chan);

		// convert the member ID set to a list of TpHandles
		TpIntSetIter iter;
		tp_intset_iter_init(&iter, set);
		const int n_elem = tp_intset_size(set);
		TpHandle handles[n_elem];
		int k = 0;
		while (tp_intset_iter_next (&iter))
			handles[k++] = iter.element;
		
		// fetch the TpContacts belonging to the TpHandles
		tp_connection_get_contacts_by_handle(conn, n_elem, handles, 0, NULL, tp_connection_contacts_by_handle_cb, pHandler, NULL, NULL);
	}
}

static void
list_connection_names_cb (const gchar * const *bus_names,
                          gsize n,
                          const gchar * const * /*cms*/,
                          const gchar * const * /*protocols*/,
                          const GError *error,
                          gpointer user_data,
                          GObject * /*unused*/)
{
	UT_DEBUGMSG(("list_connection_names_cb() n: %d\n", n));
	DTubeAccountHandler* pHandler = reinterpret_cast<DTubeAccountHandler*>(user_data);
	UT_return_if_fail(pHandler);
	
	if (error != NULL)
		return;

	TpDBusDaemon* bus = tp_dbus_daemon_new (tp_get_bus ());
	MissionControl* mc = empathy_mission_control_dup_singleton ();

	// connection <-> account display name mapping
	for (guint i = 0; i < n; i++)
	{
		UT_DEBUGMSG(("Constructing connection for bus name %s\n", bus_names[i]));
		TpConnection* conn = tp_connection_new (bus, bus_names[i], NULL, NULL);
		if (conn == NULL || !is_usable_connection(conn))
			continue;
		McAccount* account = mission_control_get_account_for_tpconnection (mc, conn, NULL);
		UT_UTF8String sAccountName = mc_account_get_display_name (account);
		UT_DEBUGMSG(("Found account that supports MUC: conn: %p, name: %s\n", conn, sAccountName.utf8_str()));

		tp_connection_get_contacts(conn, pHandler);
	}

	g_object_unref (mc);
	g_object_unref (bus);
}

static void
tube_dbus_names_changed_cb (TpChannel * /*proxy*/,
                            guint /*id*/,
                            const GPtrArray *added,
                            const GArray *removed,
                            gpointer user_data,
                            GObject * /*weak_object*/)
{
	DTubeAccountHandler *obj = (DTubeAccountHandler *) user_data;
	guint i;

	UT_DEBUGMSG(("Tube D-Bus names changed\n"));

	for (i = 0; i < added->len; i++)
	{
		GValueArray *v;
		TpHandle handle;
		const gchar *name;

		v = (GValueArray *) g_ptr_array_index (added, i);

		handle = g_value_get_uint (g_value_array_get_nth (v, 0));
		name = g_value_get_string (g_value_array_get_nth (v, 1));

		UT_DEBUGMSG(("... added %s (%d)\n", name, handle));

		UT_UTF8String buddyPath(name, strlen(name));
		PD_Document *pDoc = static_cast<PD_Document*>(XAP_App::getApp()->getLastFocussedFrame()->getCurrentDoc());

		obj->joinBuddy(pDoc, handle, buddyPath);
	}

	for (i = 0; i < removed->len; i++)
	{
		TpHandle handle;

		handle = g_array_index (removed, TpHandle, i);

		UT_DEBUGMSG(("... removed %d\n", handle));

		/* TODO: call buddyLeft */
	}
}

static void
initiator_ready_cb   (TpConnection * /*connection*/,
                      guint /*n_contacts*/,
                      TpContact * const *contacts,
                      guint /*n_failed*/,
                      const TpHandle * /*failed*/,
                      const GError * /*error*/,
                      gpointer user_data,
                      GObject *weak_object)
{
	DTubeAccountHandler *obj = (DTubeAccountHandler *) user_data;
	TpChannel *tube = TP_CHANNEL (weak_object);
	GtkWidget *dialog;
	gint response;
	guint id;
	TpChannel *chan;
	TpContact *initiator;
	GHashTable *parameters;
	GValue *title;

	initiator = contacts[0];

	g_object_get (tube,
				"channel", &chan,
				"id", &id,
				"parameters", &parameters,
				NULL);

	dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
	GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
	"%s offered you to join a collaboration session",
	tp_contact_get_alias (initiator));

	// get document title
	title = (GValue *) g_hash_table_lookup (parameters, "title");
	if (title != NULL)
	{
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
		"Document: %s", g_value_get_string (title));
	}

	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OK, GTK_RESPONSE_OK,
				NULL);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_OK)
	{
		UT_DEBUGMSG(("Tube accepted.\n"));
		obj->acceptTube(chan, id, tp_contact_get_handle (initiator));
	}
	else
	{
		UT_DEBUGMSG(("Tube declined.\n"));
		g_object_unref (tube);
	}

	gtk_widget_destroy (dialog);
	g_hash_table_destroy (parameters);
}

static void
new_tube_cb (EmpathyTubeHandler * /*thandler*/,
             TpChannel * tube,
             gpointer data)
{
	UT_DEBUGMSG(("new_tube_cb()\n"));
	
	TpChannel *chan;
	DTubeAccountHandler *obj = (DTubeAccountHandler *) data;
	TpHandle initiator;
	TpContactFeature features[1] = {TP_CONTACT_FEATURE_ALIAS};
	TpConnection *conn;

	g_object_get (tube,
				"channel", &chan,
				"initiator", &initiator,
				NULL);

	conn = tp_channel_borrow_connection (chan);
	tp_connection_run_until_ready (conn, FALSE, NULL, NULL);

	/* get the TpContact of the initiator */
	tp_connection_get_contacts_by_handle (conn, 1, &initiator, 1, features,
				initiator_ready_cb, obj, NULL, G_OBJECT (tube));

	g_object_ref (tube);
}

DTubeAccountHandler::DTubeAccountHandler()
	: AccountHandler(),
	m_bLocallyControlled(false)
{
	UT_DEBUGMSG(("DTubeAccountHandler::DTubeAccountHandler()\n"));
	m_pHandler = this;

	empathy_gtk_init ();

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	pManager->registerEventListener(this);
	tube_handler = empathy_tube_handler_new (TP_TUBE_TYPE_DBUS, "com.abisource.abiword.abicollab");
	g_signal_connect (tube_handler, "new-tube", G_CALLBACK (new_tube_cb), this);
	handle_to_bus_name = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, NULL);
}

DTubeAccountHandler::~DTubeAccountHandler()
{
	m_pHandler = NULL;
}

UT_UTF8String DTubeAccountHandler::getDescription()
{
	return "Telepathy";
}

UT_UTF8String DTubeAccountHandler::getDisplayType()
{
	return "Telepathy";
}

UT_UTF8String DTubeAccountHandler::getStorageType()
{
	return "com.abisource.abiword.abicollab.backend.dtube";
}

void DTubeAccountHandler::storeProperties()
{
	// no need to implement this as we will be getting
	// all our info always directly from sugar
}

ConnectResult DTubeAccountHandler::connect()
{
	UT_ASSERT_HARMLESS(UT_NOT_REACHED);
	return CONNECT_SUCCESS;
}

bool DTubeAccountHandler::disconnect()
{
	UT_ASSERT_HARMLESS(UT_NOT_REACHED);
	return true;
}

bool DTubeAccountHandler::isOnline()
{
	return true;
}

void DTubeAccountHandler::getBuddiesAsync()
{
	UT_DEBUGMSG(("DTubeAccountHandler::getBuddiesAsync()\n"));
		
	// ask telepathy for the connection names
	TpDBusDaemon* bus = tp_dbus_daemon_new (tp_get_bus ());
	tp_list_connection_names (bus, list_connection_names_cb, this, NULL, NULL);
	g_object_unref (bus);
}

void DTubeAccountHandler::getBuddiesAsync_cb(guint n_contacts, TpContact * const *contacts)
{
	UT_DEBUGMSG(("DTubeAccountHandler::getBuddiesAsync_cb()\n"));

	UT_DEBUGMSG(("Got %d contacts\n", n_contacts));
	for (UT_uint32 i = 0; i < n_contacts; i++)
	{
		TpContact* pContact = contacts[i];
		UT_continue_if_fail(pContact);

		TelepathyBuddyPtr pBuddy = _getBuddy(pContact);
		if (!pBuddy)
		{
			UT_DEBUGMSG(("Detected new telepathy buddy with account name: %s\n", tp_contact_get_identifier (pContact)));
			pBuddy = boost::shared_ptr<TelepathyBuddy>(new TelepathyBuddy(this, pContact));
			addBuddy(pBuddy);
		}
	}
}

BuddyPtr DTubeAccountHandler::constructBuddy(const PropertyMap& /*props*/)
{
	UT_DEBUGMSG(("DTubeAccountHandler::constructBuddy()\n"));
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);	
	return BuddyPtr();
}

BuddyPtr DTubeAccountHandler::constructBuddy(const std::string& /*descriptor*/, BuddyPtr /*pBuddy*/)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	return BuddyPtr();
}

bool DTubeAccountHandler::recognizeBuddyIdentifier(const std::string& /*identifier*/)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	return false;
}

void DTubeAccountHandler::forceDisconnectBuddy(BuddyPtr pBuddy)
{
	UT_return_if_fail(pBuddy);

	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED)
}

bool DTubeAccountHandler::startSession(PD_Document* pDoc, const std::vector<std::string>& vAcl, AbiCollab** /*pSession*/)
{
	UT_DEBUGMSG(("DTubeAccountHandler::startSession()\n"));
	UT_return_val_if_fail(pDoc, false);

	std::vector<TelepathyBuddyPtr> acl_;
	// this n^2 behavior shouldn't be too bad in practice: the ACL will never contain hundreds of elements
	for (std::vector<std::string>::const_iterator cit = vAcl.begin(); cit != vAcl.end(); cit++)
	{
		for (std::vector<BuddyPtr>::iterator it = getBuddies().begin(); it != getBuddies().end(); it++)
		{
			TelepathyBuddyPtr pBuddy = boost::static_pointer_cast<TelepathyBuddy>(*it);
			UT_continue_if_fail(pBuddy);
			if  (pBuddy->getDescriptor(false).utf8_str() == (*cit))
			{
				acl_.push_back(pBuddy);
				break;
			}
		}
	}

	UT_UTF8String sTubeAddress; 
	if (!_createAndOfferTube(pDoc, acl_, sTubeAddress))
		return false;

	// TODO: store the tube address
	
	return true;
}

void DTubeAccountHandler::handleEvent(Session& /*pSession*/)
{
	// TODO: implement me
}

/*
static void
announce_olpc_activity (TpChannel *text_chan,
                        const gchar *doc_title)
{

        GPtrArray *activities;
        GValue activity = {0,};
        GHashTable *olpc_activity_properties = g_hash_table_new (g_str_hash, g_str_equal);
        GValue act_id = {0,}, type = {0,}, color = {0,}, priv = {0,}, name = {0,};
        GError *err = NULL;
        TpHandle handle = tp_channel_get_handle (text_chan, NULL);
        TpConnection *conn = tp_channel_borrow_connection (text_chan);

        // FIXME: this is horrible
        #define ACTIVITY_ID "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

        // announce the actitiy
        activities = g_ptr_array_new ();
        g_value_init (&activity, ABI_STRUCT_TYPE_ACTIVITY);
        g_value_take_boxed (&activity, dbus_g_type_specialized_construct (ABI_STRUCT_TYPE_ACTIVITY));
        dbus_g_type_struct_set (&activity, 0, ACTIVITY_ID, 1, handle, G_MAXUINT);
        g_ptr_array_add (activities, (void *) g_value_get_boxed (&activity));

        abi_cli_olpc_buddy_info_call_set_activities (TP_PROXY (conn), -1, activities,
            NULL, NULL, NULL, NULL);
        g_ptr_array_free (activities, TRUE);

        // set activity's properties
        // id
        g_value_init (&act_id, G_TYPE_STRING);
        g_value_set_string (&act_id, ACTIVITY_ID);
        g_hash_table_insert (olpc_activity_properties, (void *) "id", (void *) &act_id);
        // type
        g_value_init (&type, G_TYPE_STRING);
        g_value_set_string (&type, "org.laptop.AbiWordActivity");
        g_hash_table_insert (olpc_activity_properties, (void *) "type", (void *) &type);
        // name (use the title)
        g_value_init (&name, G_TYPE_STRING);
        g_value_set_string (&name, doc_title);
        g_hash_table_insert (olpc_activity_properties, (void *) "name", (void *) &name);
        // color
        g_value_init (&color, G_TYPE_STRING);
        g_value_set_string (&color, "#FF8F00,#00588C");
        g_hash_table_insert (olpc_activity_properties, (void *) "color", (void *) &color);
        // private
        g_value_init (&priv, G_TYPE_BOOLEAN);
        g_value_set_boolean (&priv, FALSE);
        g_hash_table_insert (olpc_activity_properties, (void *) "private", (void *) &priv);

        abi_cli_olpc_activity_properties_call_set_properties (TP_PROXY (conn),
            -1, handle, olpc_activity_properties, NULL, NULL, NULL, NULL);

        g_hash_table_destroy (olpc_activity_properties);
}
*/

static void
set_muc_prop_cb (TpProxy * /*proxy*/,
                 const GError * /*error*/,
                 gpointer /*user_data*/,
                 GObject * /*weak_object*/)
{
        //TpChannel *text_chan = TP_CHANNEL (proxy);
        //const gchar *doc_title = (const gchar *) user_data;

        //announce_olpc_activity (text_chan, doc_title);
}

static void
list_properties_cb (TpProxy *proxy,
                    const GPtrArray *available_props,
                    const GError *error,
                    gpointer user_data,
                    GObject * /*weak_object*/)
{
	TpChannel *text_chan = TP_CHANNEL (proxy);
	guint i;
	//const gchar *doc_title = (const gchar *) user_data;

	if (error != NULL)
	{
		//announce_olpc_activity (text_chan, doc_title);
		return;
	}

	for (i = 0; i < available_props->len; i++)
	{
		GValue v = {0,};
		guint prop_id;
		gchar *name;

		g_value_init (&v, TP_STRUCT_TYPE_PROPERTY_SPEC);
		g_value_set_static_boxed (&v, g_ptr_array_index (available_props, i));
		dbus_g_type_struct_get (&v, 0, &prop_id, 1, &name, G_MAXUINT);
		if (!tp_strdiff (name, "anonymous"))
		{
			/* don't set the room anonyme */
			GPtrArray *muc_props;
			GValue anonyme = {0,}, prop = {0,};

			muc_props = g_ptr_array_new ();

			g_value_init (&anonyme, G_TYPE_BOOLEAN);
			g_value_set_boolean (&anonyme, FALSE);

			g_value_init (&prop, TP_STRUCT_TYPE_PROPERTY_VALUE);
			g_value_take_boxed (&prop, dbus_g_type_specialized_construct (TP_STRUCT_TYPE_PROPERTY_VALUE));
			dbus_g_type_struct_set (&prop, 0, prop_id, 1, &anonyme, G_MAXUINT);
			g_ptr_array_add (muc_props, (void *) g_value_get_boxed (&prop));

			tp_cli_properties_interface_call_set_properties (TP_PROXY (text_chan), -1,
						muc_props, set_muc_prop_cb, user_data, NULL, NULL);
			g_ptr_array_free (muc_props, TRUE);
			return;
		}
	}
}

static GHashTable* 
s_generate_hash(const std::map<std::string, std::string>& props)
{
	GHashTable* hash = g_hash_table_new (g_str_hash, g_str_equal);
	for (std::map<std::string, std::string>::const_iterator cit = props.begin(); cit != props.end(); cit++)
	{
		GValue* value = g_new0 (GValue, 1);
		g_value_init (value, G_TYPE_STRING);
		g_value_set_string (value, (*cit).second.c_str());
		g_hash_table_insert (hash, g_strdup((*cit).first.c_str()), value);
	}
	return hash;
}

void DTubeAccountHandler::signal(const Event& /*event*/, BuddyPtr /*pSource*/)
{
	UT_DEBUGMSG(("DTubeAccountHandler::signal()\n"));

	// NOTE: do NOT let AccountHandler::signal() send broadcast packets!
	// It will send them to all buddies, including the ones we created just
	// to be able to list the available Telepathy contacts: TelepathyBuddies.
	// They are just fake buddies however, and can't receive real packets. 
	// Only DTubeBuddy's can be sent packets

	/*
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

	// we just want to listen for when we get a document handle from the other side
	// (this obviously only makes sense for a joining party, not an offering one;
	// the offering party should never even receive such an event
	if (event.getClassType() == PCT_AccountBuddyAddDocumentEvent)
	{
		UT_DEBUGMSG(("We received a document handle from an offering party; let's join it immediately!\n"));
		AccountBuddyAddDocumentEvent& abade = (AccountBuddyAddDocumentEvent&)event;

		if (!m_bLocallyControlled)
		{
			DocHandle* pDocHandle = abade.getDocHandle();
			if (pDocHandle)
			{
				UT_DEBUGMSG(("Got dochandle, going to initiate a join on it!\n"));
				// FIXME: remove const cast
				pManager->joinSessionInitiate(pSource, pDocHandle);
			}
			else
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_NOT_REACHED);
		}
	}
	else if (event.getClassType() == PCT_StartSessionEvent)
	{
		UT_DEBUGMSG(("Sharing document. Offer tube\n"));
		DisplayShareDialog();
	}*/
}

bool DTubeAccountHandler::send(const Packet* pPacket)
{
	UT_DEBUGMSG(("DTubeAccountHandler::send(const Packet* pPacket)\n"));
	UT_return_val_if_fail(pPacket, false);
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	return true;
}

bool DTubeAccountHandler::send(const Packet* pPacket, BuddyPtr pBuddy)
{
	UT_DEBUGMSG(("DTubeAccountHandler::send(const Packet* pPacket, BuddyPtr pBuddy\n"));
	UT_return_val_if_fail(pPacket, false);
	UT_return_val_if_fail(pBuddy, false);
	    
	DTubeBuddyPtr pDTubeBuddy = boost::static_pointer_cast<DTubeBuddy>(pBuddy);
	UT_DEBUGMSG(("Sending packet to d-tube buddy on dbus addess: %s\n", pDTubeBuddy->getDBusName().utf8_str()));

	DBusMessage* pMessage = dbus_message_new_method_call (pDTubeBuddy->getDBusName().utf8_str(), "/org/laptop/DTube/Presence/Buddies", INTERFACE, SEND_ONE_METHOD);
	// TODO: check dst
	dbus_message_set_destination(pMessage, pDTubeBuddy->getDBusName().utf8_str());
	UT_DEBUGMSG(("Destination (%s) set on message\n", pDTubeBuddy->getDBusName().utf8_str()));

	// we don't want replies, because they easily run into dbus timeout problems 
	// when sending large packets
	// TODO: this means we should probably use signals though
	dbus_message_set_no_reply(pMessage, TRUE);
	
	// make to-be-send-stream once
	std::string data;
	_createPacketStream( data, pPacket );

	const char* packet_contents = &data[0];
	dbus_message_append_args(pMessage,
					DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &packet_contents, data.size(),
					DBUS_TYPE_INVALID);
	UT_DEBUGMSG(("Appended packet contents\n"));

	bool sent = dbus_connection_send(pDTubeBuddy->getTube(), pMessage, NULL);
	UT_ASSERT_HARMLESS(sent);
	if (sent)
		dbus_connection_flush(pDTubeBuddy->getTube());
	dbus_message_unref(pMessage);
	return sent;
}

bool DTubeAccountHandler::_startSession(PD_Document* pDoc, const UT_UTF8String& tubeDBusAddress)
{
	UT_DEBUGMSG(("DTubeAccountHandler::_startSession() - tubeDBusAddress: %s", tubeDBusAddress.utf8_str()));
	
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	// TODO: check that we aren't already in a session; this backend can only host one session at a time (for now)

	UT_return_val_if_fail(pDoc, false);

	UT_DEBUGMSG(("Got tube address: %s\n", tubeDBusAddress.utf8_str()));

	//DBusError error;
	DBusConnection* pTube = dbus_connection_open(tubeDBusAddress.utf8_str(), NULL);
	if (pTube)
	{
		UT_DEBUGMSG(("Opened a dbus connection for tube: %s\n", tubeDBusAddress.utf8_str()));

		UT_DEBUGMSG(("Adding dbus handlers to the main loop\n"));
		dbus_connection_setup_with_g_main(pTube, NULL);

		UT_DEBUGMSG(("Adding message filter\n"));
		dbus_connection_add_filter(pTube, s_dbus_handle_message, this, NULL);

		m_bLocallyControlled = true;

		// we are "connected" now, time to start sending out, and listening to messages (such as events)
		//pManager->registerEventListener(this);
		// start hosting a session on the current document
		//UT_UTF8String sID;
		//pManager->startSession(pDoc, sID, NULL);
		return true;
	}
	else
		UT_DEBUGMSG(("Failed to open a dbus connection for tube: %s\n", tubeDBusAddress.utf8_str()));

	return false;
}

bool DTubeAccountHandler::joinTube(const UT_UTF8String& tubeDBusAddress)
{
	UT_DEBUGMSG(("DTubeAccountHandler::joinTube()\n"));

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	// TODO: check that we aren't already in a session; this backend can only join one session at a time (for now)

	//DBusError error;
	DBusConnection* pTube = dbus_connection_open(tubeDBusAddress.utf8_str(), NULL);
	if (pTube)
	{
		UT_DEBUGMSG(("Opened a dbus connection for tube: %s\n", tubeDBusAddress.utf8_str()));

		UT_DEBUGMSG(("Adding dbus handlers to the main loop\n"));
		dbus_connection_setup_with_g_main(pTube, NULL);

		UT_DEBUGMSG(("Adding message filter\n"));
		dbus_connection_add_filter(pTube, s_dbus_handle_message, this, NULL);

		m_bLocallyControlled = false;

		// we are "connected" now, time to start sending out, and listening to messages (such as events)
		//pManager->registerEventListener(this);
	}
	else
		UT_DEBUGMSG(("Failed to open a dbus connection for tube: %s\n", tubeDBusAddress.utf8_str()));

	return false;
}

bool DTubeAccountHandler::joinBuddy(PD_Document* /*pDoc*/, TpHandle handle, const UT_UTF8String& buddyDBusAddress)
{
	UT_DEBUGMSG(("DTubeAccountHandler::joinBuddy()\n"));

	if (g_hash_table_lookup (handle_to_bus_name, GUINT_TO_POINTER (handle)) != NULL)
		return false;
	g_hash_table_insert (handle_to_bus_name, GUINT_TO_POINTER (handle), (void *) &buddyDBusAddress);

	// TODO: implement me properly
	return false;
	
/*	DTubeBuddyPtr pBuddy = boost::shared_ptr<DTubeBuddy>(new DTubeBuddy(this, buddyDBusAddress));
	addBuddy(pBuddy);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	if (m_bLocallyControlled)
	{
		AbiCollab* pSession = pManager->getSession(pDoc);
		UT_return_val_if_fail(pSession, false);
		pSession->addCollaborator(pBuddy);
		return true;
	}
	else
	{
		UT_DEBUGMSG(("Buddy joined, while we are NOT hosting a session; requesting sessions from buddy: %s\n", pBuddy->getDescriptor(false).utf8_str()));
		getSessionsAsync(pBuddy);
		return true;
	}

	return false;*/
}

bool DTubeAccountHandler::disjoinBuddy(FV_View* pView, const UT_UTF8String& buddyDBusAddress)
{
	UT_DEBUGMSG(("DTubeAccountHandler::disjoinBuddy()\n"));
	UT_return_val_if_fail(pView, false);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc, false);

 	if (m_bLocallyControlled)
	{
		UT_DEBUGMSG(("Dropping buddy %s from the session!", buddyDBusAddress.utf8_str()));
		AbiCollab* pSession = pManager->getSessionFromDocumentId(pDoc->getDocUUIDString());
		if (pSession)
		{	
			UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
			DTubeBuddyPtr pTmpBuddy;// = _getBuddy(buddyDBusAddress);
			pSession->removeCollaborator(pTmpBuddy);
			return true;
		}
	}
	else
	{
		UT_DEBUGMSG(("The session owner (%s) left!", buddyDBusAddress.utf8_str()));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return true;
	}

	return false;
}

void DTubeAccountHandler::acceptTube(TpChannel *chan,
                                     guint id,
                                     TpHandle initiator)
{
	gboolean result;
	gchar *address;
	GPtrArray *names;
	guint i;

	result = tp_cli_channel_type_tubes_run_accept_d_bus_tube (chan, -1, id,
				&address, NULL, NULL);
	g_assert (result);

	UT_UTF8String tubeDBusAddress(address, strlen (address));
	joinTube(tubeDBusAddress);

	tp_cli_channel_type_tubes_connect_to_d_bus_names_changed (chan,
				tube_dbus_names_changed_cb, this, NULL, NULL, NULL);

	/* add initiator */
	result = tp_cli_channel_type_tubes_run_get_d_bus_names (chan, -1,
	id, &names, NULL, NULL);

	for (i = 0; i < names->len; i++)
	{
		GValueArray *v;
		TpHandle handle;
		const gchar *name;

		v = (GValueArray *) g_ptr_array_index (names, i);

		handle = g_value_get_uint (g_value_array_get_nth (v, 0));
		name = g_value_get_string (g_value_array_get_nth (v, 1));

		if (handle != initiator)
			continue;

		UT_DEBUGMSG(("Found initiator %s (%d)\n", name, handle));
		UT_UTF8String buddyPath(name, strlen(name));
		PD_Document *pDoc = static_cast<PD_Document*>(XAP_App::getApp()->getLastFocussedFrame()->getCurrentDoc());

		joinBuddy(pDoc, handle, buddyPath);
		break;
	}
}

void DTubeAccountHandler::handleMessage(const char* senderDBusAddress, const char* packet_data, int packet_size)
{
	UT_DEBUGMSG(("DTubeAccountHandler::handleMessage()\n"));

	// get the buddy for this session
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	DTubeBuddyPtr pBuddy;// = _getBuddy(senderDBusAddress);
	UT_return_if_fail(pBuddy); // TODO: shouldn't we just disconnect here?

	// construct the packet
	// FIXME: inefficient copying of data
	std::string packet_str(packet_size, ' ');
	memcpy(&packet_str[0], packet_data, packet_size);
	FREEP(packet_data);
	Packet* pPacket = _createPacket(packet_str, pBuddy);
	UT_return_if_fail(pPacket); // TODO: shouldn't we just disconnect here?

	// handle!
	AccountHandler::handleMessage(pPacket, pBuddy);
}

DBusHandlerResult s_dbus_handle_message(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	UT_DEBUGMSG(("s_dbus_handle_message()\n"));
	UT_return_val_if_fail(connection, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	UT_return_val_if_fail(message, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	UT_return_val_if_fail(user_data, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	DTubeAccountHandler* pHandler = reinterpret_cast<DTubeAccountHandler*>(user_data);

	if (dbus_message_is_method_call(message, INTERFACE, SEND_ONE_METHOD))
	{
		UT_DEBUGMSG(("%s message accepted!\n", SEND_ONE_METHOD));

		const char* senderDBusAddress = dbus_message_get_sender(message);

		DBusError error;
		dbus_error_init (&error);
		const char* packet_data = 0;
		int packet_size = 0;
	    if (dbus_message_get_args(message, &error, 
					DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &packet_data, &packet_size,
					DBUS_TYPE_INVALID))
		{
			UT_DEBUGMSG(("Received packet from %s\n", senderDBusAddress));
			pHandler->handleMessage(senderDBusAddress, packet_data, packet_size);
			//dbus_free(packet);
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		else
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}	

	UT_DEBUGMSG(("Unhandled message\n"));
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

// TODO: cleanup after errors
bool DTubeAccountHandler::_createAndOfferTube(PD_Document* pDoc, const std::vector<TelepathyBuddyPtr>& vBuddies, UT_UTF8String& sTubeAddress)
{
	GError* error = NULL;
	GHashTable* params;
	gchar* object_path;
	GHashTable* channel_properties;
	gboolean result;
	gchar *address;
	GValue title = {0,};
	const gchar *doc_title;

	UT_return_val_if_fail(pDoc, false);
	UT_return_val_if_fail(vBuddies.size() > 0, false);
	
	// get some connection belonging to this contact
	// TODO: we probably want to change this to some user selectable thingy
	TpContact* pContact = vBuddies[0]->getContact();
	TpConnection * conn = tp_contact_get_connection (pContact);

	//
	// create a room, so we can invite some members in it
	//

	// first setup some properties for this room...
	
	std::map<std::string, std::string> h_params;
	h_params["org.freedesktop.Telepathy.Channel.ChannelType"] = TP_IFACE_CHANNEL_TYPE_DBUS_TUBE;
	//h_params["org.freedesktop.Telepathy.Channel.TargetHandleType"] = TP_HANDLE_TYPE_ROOM;
	h_params["org.freedesktop.Telepathy.Channel.TargetID"] = "abicollabtest@conference.jabber.org";
	h_params["org.freedesktop.Telepathy.Channel.Type.DBusTube.ServiceName"] = "com.abisource.abiword.abicollab";
	params = s_generate_hash(h_params);

	// org.freedesktop.Telepathy.Channel.TargetHandleType
	GValue target_handle_type = {0,};
	g_value_init (&target_handle_type, G_TYPE_INT);
	g_value_set_int (&target_handle_type, TP_HANDLE_TYPE_ROOM);
	g_hash_table_insert (params, (void *) "org.freedesktop.Telepathy.Channel.TargetHandleType", (void *) &target_handle_type);

	// ... then actually create the room
	if (!tp_cli_connection_interface_requests_run_create_channel (conn, -1, params, &object_path, &channel_properties, &error, NULL))
	{
		UT_DEBUGMSG(("Error creating room: %s\n", error ? error->message : "(null)"));
		return false;
	}
	UT_DEBUGMSG(("Got a room, path: %s\n", object_path));
	
	// get a channel to the new room
	TpChannel* chan = tp_channel_new_from_properties (conn, object_path, channel_properties, NULL);
	UT_return_val_if_fail(chan, "");
	tp_channel_run_until_ready (chan, NULL, NULL);
	// TODO: check for errors
	UT_DEBUGMSG(("Channel created to the room\n"));
	
	// add members to the room
	GArray* members = g_array_new (FALSE, FALSE, sizeof(TpHandle));

	for (UT_uint32 i = 0; i < vBuddies.size(); i++)
	{
		TelepathyBuddyPtr pBuddy = vBuddies[i];
		UT_continue_if_fail(pBuddy && pBuddy->getContact());
		TpHandle handle = tp_contact_get_handle(pBuddy->getContact());
		UT_DEBUGMSG(("Adding %s to the invite list\n", tp_contact_get_identifier(pBuddy->getContact())));
		g_array_append_val(members, handle);
	}

	UT_DEBUGMSG(("Inviting members to the room...\n"));
	if (!tp_cli_channel_interface_group_run_add_members (chan, -1,  members, "Hi there!", &error, NULL))
	{
		UT_DEBUGMSG(("Error inviting room members: %s\n", error ? error->message : "(null)"));
		return false;
	}
	UT_DEBUGMSG(("Members invited\n"));

	params = g_hash_table_new (g_str_hash, g_str_equal);
	/* Document title */
	/* HACK
	* <uwogBB> cassidy: but NOTE that if the current focussed document is other than the doc you shared, you'll get a fsckup
	* <uwogBB> cassidy: so mark it with "HACK", so we will add the document to the event signal it belongs to
	*/
	g_value_init (&title, G_TYPE_STRING);
	doc_title = XAP_App::getApp()->getLastFocussedFrame()->getTitle().utf8_str();
	g_value_set_string (&title, doc_title);
	g_hash_table_insert (params, (void *) "title", (void *) &title);

	// offer this tube to every participant in the room
	result = tp_cli_channel_type_dbus_tube_run_offer (chan, -1, params, TP_SOCKET_ACCESS_CONTROL_LOCALHOST, &address, &error, NULL);
	if (!result)
	{
		UT_DEBUGMSG(("Error offering tube to room participants: %s\n", error ? error->message : "(null)"));
		return false;
	}
	g_hash_table_destroy (params);

	UT_DEBUGMSG(("Tube offered, address: %s\n", address));
	sTubeAddress = address;
	
	// start listening on the tube for people entering and leaving it
	tp_cli_channel_type_tubes_connect_to_d_bus_names_changed (chan, tube_dbus_names_changed_cb, this, NULL, NULL, NULL);

	return true;
}

// FIXME: this can't be right
TelepathyBuddyPtr DTubeAccountHandler::_getBuddy(TpContact* pContact)
{
	for (std::vector<BuddyPtr>::iterator it = getBuddies().begin(); it != getBuddies().end(); it++)
	{
		TelepathyBuddyPtr pB = boost::static_pointer_cast<TelepathyBuddy>(*it);
		UT_continue_if_fail(pB);
		if (pB->equals(pContact))
			return pB;
	}
	return TelepathyBuddyPtr();
}


