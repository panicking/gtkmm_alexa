#include <string>
#include <iostream>
#include <libsoup/soup.h>
#include <gmodule.h>
#include <glibmm.h>
#include <giomm.h>
#include <rapidjson/document.h>
#include "alexaclient.h"

static void
soup_server_error(SoupWebsocketConnection *self,
                  gpointer user_data)
{
    g_print("WS Error\n");
}

static void decode_alexa_json(AlexaClient* alexa, const gchar* ptr)
{
    rapidjson::StringStream parser(ptr);
    rapidjson::Document reader;

    reader.ParseStream(parser);

    if (!reader.HasMember("state"))
        return;
    std::string value = reader["state"].GetString();
    alexa->setAlexaState(value);
}

static void
soup_server_data(SoupWebsocketConnection *self, gint type, GBytes *message,
                 gpointer user_data)
{
    AlexaClient *alexa = static_cast<AlexaClient*>(user_data);
    AlexaClient::Connection *connection = alexa->get_webrtc_connection();

    if (connection->cconnection == NULL)
        return;

    if (type == SOUP_WEBSOCKET_DATA_TEXT) {
        gsize sz;
        const gchar* ptr;

        ptr = (const gchar *)g_bytes_get_data(message, &sz);

        soup_websocket_connection_send_text(connection->cconnection, ptr);
        g_print("Received server text data: %s\n", ptr);
    }
    else if (type == SOUP_WEBSOCKET_DATA_BINARY) {
        g_print("Received binary data (not shown)\n");
    }
    else {
        g_print("Invalid data type: %d\n", type);
    }
}

static void soup_client_data(SoupWebsocketConnection *conn, gint type,
                             GBytes *message, gpointer user_data)
{
    AlexaClient *alexa = static_cast<AlexaClient*>(user_data);
    AlexaClient::Connection *connection = alexa->get_webrtc_connection();

    if (type == SOUP_WEBSOCKET_DATA_TEXT) {
        gsize sz;
        const gchar* ptr;

        ptr = (const gchar *)g_bytes_get_data(message, &sz);
        g_print("Received client text data: %s\n", ptr);
        soup_websocket_connection_send_text(connection->connection, ptr);
        decode_alexa_json(alexa, ptr);
    }
    else if (type == SOUP_WEBSOCKET_DATA_BINARY) {
        g_print("Received binary data (not shown)\n");
    }
    else {
        g_print("Invalid data type: %d\n", type);
    }
}

static void soup_connection_state_changed(GObject *object, GParamSpec *param, gpointer user_data)
{
    AlexaClient *alexa = static_cast<AlexaClient*>(user_data);
    struct AlexaClient::Connection *connection = alexa->get_webrtc_connection();
    SoupConnectionState state;

    g_object_get(object, "state", &state, NULL);

    switch (state) {
    case SOUP_CONNECTION_IDLE:
        g_print("Alexa disconnect\n");
        g_clear_object(&connection->cconnection);
        break;
    }
    alexa->setAlexaState("UNKNOWN");
}

static void on_SoupClientWebsocketCallback(SoupSession *session, GAsyncResult *res, gpointer user_data)
{
    AlexaClient *alexa = static_cast<AlexaClient*>(user_data);
    struct AlexaClient::Connection *connection = alexa->get_webrtc_connection();
    GError *error = NULL;

    connection->cconnection = soup_session_websocket_connect_finish(session, res, &error);
    if (error) {
        g_print("Error: %s\n", error->message);
        g_error_free(error);
        return;
    }

    soup_websocket_connection_send_text(connection->cconnection, "alexa");
    g_signal_connect(connection->cconnection, "message", G_CALLBACK(soup_client_data), alexa);
    g_signal_connect(connection->cconnection, "notify::state", G_CALLBACK (soup_connection_state_changed), alexa);
    g_print("Alexa connected\n");
}

static void on_SoupServerWebsocketCallback(SoupServer *server, SoupWebsocketConnection *conn,
                                const char *path, SoupClientContext *client,
                                gpointer user_data)
{
    SoupMessage *msg = NULL;
    AlexaClient *alexa = static_cast<AlexaClient*>(user_data);
    struct AlexaClient::Connection *connection = alexa->get_webrtc_connection();

    if (connection->connection)
        return;

    g_print("Connection from WebPage\n");
    /* Figure out who we're talking to */
    GSocketAddress *socket_address = soup_client_context_get_remote_address(client);

    GSocketFamily family = g_socket_address_get_family(socket_address);

    if ((family != G_SOCKET_FAMILY_IPV4) && (family != G_SOCKET_FAMILY_IPV6)) {
        /* Should be unreachable */
        g_print("Non-IP socket?\n");
        return;
    }

    connection->connection = conn;
    g_object_ref(conn);
    /* Subscribe to the various signals */
    g_signal_connect(conn, "message", G_CALLBACK(soup_server_data), alexa);
    g_signal_connect(conn, "error", G_CALLBACK(soup_server_error), alexa);

    if (!connection->session) {
        connection->session = soup_session_new();
        /* websocket client */
        msg = soup_message_new(SOUP_METHOD_GET, "ws://localhost:8933");
        soup_session_websocket_connect_async(connection->session, msg, NULL, NULL, NULL,
                                             (GAsyncReadyCallback)on_SoupClientWebsocketCallback, alexa);
        g_print("Connect to Alexa\n");
    }
}

AlexaClient::AlexaClient()
{
    server = soup_server_new("tls-certificate", NULL, NULL);
    SoupMessage *msg = NULL;
    connection = { 0 };

    soup_server_add_websocket_handler(server, NULL, NULL, NULL, on_SoupServerWebsocketCallback, this, NULL);
    soup_server_listen_local(server, 8934, SOUP_SERVER_LISTEN_IPV4_ONLY, NULL);
}

AlexaClient::~AlexaClient()
{
    soup_server_remove_handler(server, NULL);
    g_clear_object(&server);
}

AlexaClient::AlexaState AlexaClient::alexaState() const
{
    return m_alexaState;
}

sigc::signal<void(std::string)> AlexaClient::onNewWebConnectionChanged()
{
    return onNewWebConnection;
}
