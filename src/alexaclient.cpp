#include <string>
#include <iostream>
#include <libsoup/soup.h>
#include <gmodule.h>
#include "alexaclient.h"


static void
soup_server_error(SoupWebsocketConnection *self,
               gpointer                 user_data)
{
    fprintf(stderr, "WS Error\n");
}

static void
soup_server_data(SoupWebsocketConnection *self,
               gint                     type,
               GBytes                  *message,
               gpointer                 user_data)
{
    AlexaClient *alexa = static_cast<AlexaClient*>(user_data);
    AlexaClient::Connection *connection = alexa->get_webrtc_connection();

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
    }
    else if (type == SOUP_WEBSOCKET_DATA_BINARY) {
        g_print("Received binary data (not shown)\n");
    }
    else {
        g_print("Invalid data type: %d\n", type);
    }
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
    fprintf(stderr, "Alexa connected\n");
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

    fprintf(stderr, "Connection from WebPage\n");
    /* Figure out who we're talking to */
    GSocketAddress *socket_address = soup_client_context_get_remote_address(client);

    GSocketFamily family = g_socket_address_get_family(socket_address);

    if ((family != G_SOCKET_FAMILY_IPV4) && (family != G_SOCKET_FAMILY_IPV6)) {
        /* Should be unreachable */
        fprintf(stderr, "Non-IP socket?\n");
        return;
    }

    GInetAddress *inet_address = g_inet_socket_address_get_address((GInetSocketAddress *) socket_address);
    connection->ip_address = g_inet_address_to_string(inet_address);
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
        fprintf(stderr, "Connect to Alexa\n");
    }
}

AlexaClient::AlexaClient()
{
    SoupServer *server = soup_server_new("tls-certificate", NULL, NULL);
    SoupMessage *msg = NULL;
    connection = { 0 };

    soup_server_add_websocket_handler(server, NULL, NULL, NULL, on_SoupServerWebsocketCallback, this, NULL);
    soup_server_listen_local(server, 8934, SOUP_SERVER_LISTEN_IPV4_ONLY, NULL);
}

AlexaClient::~AlexaClient()
{
}

AlexaClient::AlexaState AlexaClient::alexaState() const
{
    return m_alexaState;
}

bool AlexaClient::alexaRendering() const
{
    return m_alexaRenderingDocument || m_alexaRenderingPlayer || m_alexaRenderingTemplate;
}

sigc::signal<void, bool> AlexaClient::onNewWebConnectionChanged()
{
    return onNewWebConnection;
}

void AlexaClient::processAppTextMessage(const std::string &message)
{
}

void AlexaClient::processAppBinaryMessage(const std::vector<unsigned char> &message)
{
}

void AlexaClient::processWebTextMessage(const std::string &message)
{
}

void AlexaClient::processWebBinaryMessage(const std::vector<unsigned char> &message)
{
}

void AlexaClient::processWebDisconnect()
{
}
