#include "IrcChannel2.hpp"

#include "debug/AssertInGuiThread.hpp"
#include "messages/Message.hpp"
#include "messages/MessageBuilder.hpp"
#include "providers/irc/IrcCommands.hpp"
#include "providers/irc/IrcServer.hpp"

namespace chatterino {

IrcChannel::IrcChannel(const QString &name, IrcServer *server)
    : Channel(name, Channel::Type::Irc)
    , ChannelChatters(*static_cast<Channel *>(this))
    , server_(server)
{
    std::vector<QString> channelIds = std::vector(
        {QString("11148817"), QString("22484632"), QString("31400525")});
    emoteProvider_.loadChannelEmotes(channelIds);
}

/////////////////////////////////////////////// mmos
void IrcChannel::addMessageContent(MessageBuilder &builder,
                                   const QString &message)
{
    QStringList words = message.split(' ');

    for (auto word : words)
    {
        if (word == server()->nick() || word == "@" + server()->nick()) {
                builder->flags.set(MessageFlag::Highlighted);
        }
        auto linkString = builder.matchLink(word);
        if (!linkString.isEmpty()) {
            auto link = Link();
            builder.addLink(word, linkString);
        }
        else if (auto emote = this->emoteProvider_.tryEmote(word))
        {
            builder.append(std::move(emote.get()));
        }
        else
        {
            builder.emplace<TextElement>(word, MessageElementFlag::Text);
        }
    }
}

MessagePtr IrcChannel::buildMessage(const QString &nick, const QString &message)
{
    MessageBuilder builder;
    builder.emplace<TimestampElement>();
    builder.emplace<TextElement>(nick + ":", MessageElementFlag::Username);
    addMessageContent(builder, message);
    return builder.release();
}
/////////////////////////////////////////////// mmos

void IrcChannel::sendMessage(const QString &message)
{
    assertInGuiThread();

    if (message.startsWith("/"))
    {
        int index = message.indexOf(' ', 1);
        QString command = message.mid(1, index - 1);
        QString params = index == -1 ? "" : message.mid(index + 1);

        invokeIrcCommand(command, params, *this);
    }
    else
    {
        if (this->server())
            this->server()->sendMessage(this->getName(), message);

        // MessageBuilder builder;
        // builder.emplace<TimestampElement>();
        // builder.emplace<TextElement>(this->server()->nick() + ":",
        //                              MessageElementFlag::Username);
        // builder.emplace<TextElement>(message, MessageElementFlag::Text);
        this->addMessage(buildMessage(this->server()->nick(), message));
    }
}

IrcServer *IrcChannel::server()
{
    assertInGuiThread();

    return this->server_;
}

void IrcChannel::setServer(IrcServer *server)
{
    assertInGuiThread();

    this->server_ = server;
}

bool IrcChannel::canReconnect() const
{
    return true;
}

void IrcChannel::reconnect()
{
    if (this->server())
        this->server()->connect();
}

}  // namespace chatterino
