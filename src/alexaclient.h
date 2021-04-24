#pragma once

#include <sigc++/sigc++.h>
#include <string>
#include <vector>
#include <map>

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

    std::string alexaStateStr() const {
        for (auto &index: s_mapStringToAlexaState) {
            if (index.second == m_alexaState)
                return index.first;
        }
        return nullptr;
    };

    void setAlexaState(const std::string state) {
        m_alexaState = s_mapStringToAlexaState[state];
        onNewWebConnection.emit(state);
    }

    AlexaState alexaStateEnum(std::string state) {
        return s_mapStringToAlexaState[state];
    }

    bool alexaRendering() const;

    int webPageState() const;

    void set_webrtc_connection(struct Connection *conn);
    struct Connection* get_webrtc_connection() { return &connection; };

    sigc::signal<void(std::string)> onNewWebConnectionChanged();

private:

    std::map<std::string, AlexaState> s_mapStringToAlexaState =
    {
        { "UNKNOWN", AlexaState::UNKNOWN },
        { "DISCONNECTED", AlexaState::DISCONNECTED },
        { "CONNECTING", AlexaState::CONNECTING },
        { "CONNECTED", AlexaState::CONNECTED },
        { "IDLE", AlexaState::IDLE },
        { "LISTENING", AlexaState::LISTENING },
        { "EXPECTING", AlexaState::EXPECTING },
        { "THINKING", AlexaState::THINKING },
        { "SPEAKING", AlexaState::SPEAKING }
    };

    void alexaStateChanged();
    void alexaRenderingChanged();
    void webPageStateChanged();

    void processAppTextMessage(const std::string &message);
    void processAppBinaryMessage(const std::vector<unsigned char> &message);

    void processWebTextMessage(const std::string &message);
    void processWebBinaryMessage(const std::vector<unsigned char> &message);
    void processWebDisconnect();

    AlexaState m_alexaState{AlexaState::UNKNOWN};

    bool m_alexaRenderingTemplate{false};
    bool m_alexaRenderingDocument{false};
    bool m_alexaRenderingPlayer{false};

    /* webrtc */
    SoupServer *server;
    Connection connection;

private:
    sigc::signal<void(std::string)> onNewWebConnection;
};
