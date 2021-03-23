#pragma once

#include <sigc++/sigc++.h>
#include <string>
#include <vector>

class AlexaClient
{
public:
    explicit AlexaClient();
    ~AlexaClient();

    struct Connection {
        /* The reference counted connection itself */
        SoupWebsocketConnection *connection;

        /* The reference counted proxied connection */
        GSocketConnection *proxy_connection;
        GDataInputStream *distream;

        /* Has this connection authenticated yet? */
        gboolean is_authenticated;

        /* Remote IP Address. Must be g_free'd on destroy */
        gchar *ip_address;

        SoupSession *session;
        SoupWebsocketConnection *cconnection;
    };

    enum AlexaState {
        UNKNOWN,
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        IDLE,
        LISTENING,
        EXPECTING,
        THINKING,
        SPEAKING
    };

    void connectSocketToSmartScreen();
    void disconnectSocketToSmartScreen();

    AlexaState alexaState() const;
    std::string alexaStateStr() const;

    bool alexaRendering() const;

    int webPageState() const;

    void set_webrtc_connection(struct Connection *conn);
    struct Connection* get_webrtc_connection() { return &connection; };

private:
    void alexaStateChanged();
    void alexaRenderingChanged();
    void webPageStateChanged();

    void processAppTextMessage(const std::string &message);
    void processAppBinaryMessage(const std::vector<unsigned char> &message);

    void processWebTextMessage(const std::string &message);
    void processWebBinaryMessage(const std::vector<unsigned char> &message);
    void processWebDisconnect();

    AlexaState m_alexaState{AlexaState::UNKNOWN};
    void setAlexaState(const AlexaState state);

    bool m_alexaRenderingTemplate{false};
    bool m_alexaRenderingDocument{false};
    bool m_alexaRenderingPlayer{false};

    /* webrtc */
    SoupServer *server;
    Connection connection;

    sigc::signal<void, bool> onNewWebConnectionChanged();
private:
    sigc::signal<void, bool> onNewWebConnection;
};
