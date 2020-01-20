#pragma once

#include "common/Channel.hpp"
#include "common/ChannelChatters.hpp"

#include "messages/MessageBuilder.hpp"
#include "messages/MessageElement.hpp"
#include "messages/Emote.hpp"

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
    boost::optional<std::unique_ptr<EmoteElement>> bttvEmote(const QString &word);
    boost::optional<std::unique_ptr<EmoteElement>> ffzEmote(const QString &word);

  MessagePtr buildMessage(const QString &nick, const QString &message);
  boost::optional<EmotePtr> bttvEmotex(const EmoteName &name);

    IrcServer *server_;

    friend class Irc;
};

}  // namespace chatterino
