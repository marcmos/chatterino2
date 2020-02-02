#pragma once

#include "common/Channel.hpp"
#include "common/ChannelChatters.hpp"

#include "messages/MessageBuilder.hpp"
#include "providers/emote/EmoteProvider.hpp"

namespace chatterino {

class Irc;
class IrcServer;


class IrcChannel : public Channel, public ChannelChatters
{
public:
    explicit IrcChannel(const QString &name, IrcServer *server);

    void sendMessage(const QString &message) override;

    // server may be nullptr
    IrcServer *server();

    // Channel methods
    virtual bool canReconnect() const override;
    virtual void reconnect() override;

  void addMessageContent(MessageBuilder& builder, const QString& message);
private:
    void setServer(IrcServer *server);

    MessagePtr buildMessage(const QString &nick, const QString &message);

    IrcServer *server_;
    EmoteProvider emoteProvider_;

    friend class Irc;
};

}  // namespace chatterino
