#pragma once

#include <boost/signals2.hpp>
#include <string>
#include <vector>
#include <map>

class AlexaClient
{
public:
    explicit AlexaClient();
    ~AlexaClient();

    struct Connection {
        SoupWebsocketConnection *connection;
        /* client session */
        SoupWebsocketConnection *cconnection;
        SoupSession *session;
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
        onNewWebConnectionChanged(state);
    }

    AlexaState alexaStateEnum(std::string state) {
        return s_mapStringToAlexaState[state];
    }

    void set_webrtc_connection(struct Connection *conn);
    struct Connection* get_webrtc_connection() { return &connection; };

    boost::signals2::signal<void (std::string)> onNewWebConnectionChanged;

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

    AlexaState m_alexaState{AlexaState::UNKNOWN};

    /* webrtc */
    SoupServer *server;
    Connection connection;
};
