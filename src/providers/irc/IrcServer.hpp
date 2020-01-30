#pragma once

#include "providers/irc/AbstractIrcServer.hpp"
#include "providers/irc/IrcAccount.hpp"
#include "providers/bttv/BttvEmotes.hpp"
#include "providers/ffz/FfzEmotes.hpp"
#include "providers/emote/EmoteProvider.hpp"
#include "common/Atomic.hpp"
#include "messages/Emote.hpp"

namespace chatterino {

struct IrcServerData;

class IrcServer : public AbstractIrcServer
{
public:
    explicit IrcServer(const IrcServerData &data);
    IrcServer(const IrcServerData &data,
              const std::vector<std::weak_ptr<Channel>> &restoreChannels);
    ~IrcServer() override;

    int id();
    const QString &user();
    const QString &nick();

    // const BttvEmotes &bttv() { return bttv_; }
    const std::shared_ptr<const EmoteMap> &bttvChannel() { return bttvChannel_; }
    const std::shared_ptr<const EmoteMap> &ffzChannel() { return ffzChannel_; }
    // const FfzEmotes &ffz() { return ffzEmotes_; }

    EmoteProvider emoteProvider_;
    // AbstractIrcServer interface
protected:
    void initializeConnectionSignals(IrcConnection *connection,
                                     ConnectionType type) override;
    void initializeConnection(IrcConnection *connection,
                              ConnectionType type) override;
    std::shared_ptr<Channel> createChannel(const QString &channelName) override;
    bool hasSeparateWriteConnection() const override;

    void onReadConnected(IrcConnection *connection) override;
    void privateMessageReceived(Communi::IrcPrivateMessage *message) override;
    void readConnectionMessageReceived(Communi::IrcMessage *message) override;

private:
    // pointer so we don't have to circle include Irc2.hpp
    IrcServerData *data_;
    // BttvEmotes bttv_;
    // FfzEmotes ffzEmotes_;
    // Atomic<std::shared_ptr<const EmoteMap>> bttvChannel_;
    std::shared_ptr<const EmoteMap> bttvChannel_;
    std::shared_ptr<const EmoteMap> ffzChannel_;
};

}  // namespace chatterino
