#include "IrcChannel2.hpp"

#include "debug/AssertInGuiThread.hpp"
#include "messages/MessageBuilder.hpp"
#include "providers/irc/IrcCommands.hpp"
#include "providers/irc/IrcServer.hpp"

namespace chatterino {

IrcChannel::IrcChannel(const QString &name, IrcServer *server)
    : Channel(name, Channel::Type::Irc)
    , ChannelChatters(*static_cast<Channel *>(this))
    , server_(server)
{
}

  /////////////////////////////////////////////// mmos
  boost::optional<std::unique_ptr<EmoteElement>> IrcChannel::bttvEmote(const QString &word) {
    if (this->server() == nullptr) {
      return boost::none;
    } else {
      const BttvEmotes& bttv = this->server()->bttv();
      auto emote = bttv.emote({word});
      if (emote) {
        auto flags = MessageElementFlag::BttvEmote;
        return std::make_unique<EmoteElement>(emote.get(), flags);
      } else {
        return boost::none;
      }
    }
  }

  boost::optional<std::unique_ptr<EmoteElement>> IrcChannel::ffzEmote(const QString &word) {
    if (this->server() == nullptr) {
      return boost::none;
    } else {
      const FfzEmotes& bttv = this->server()->ffz();
      auto emote = bttv.emote({word});
      if (emote) {
        auto flags = MessageElementFlag::FfzEmote;
        return std::make_unique<EmoteElement>(emote.get(), flags);
      } else {
        return boost::none;
      }
    }
  }

  // skopiowane z TwitchChannel::bttvEmote
  boost::optional<EmotePtr> IrcChannel::bttvEmotex(const EmoteName &name)
  {
    auto emotes = this->server()->bttvChannel();
    auto it = emotes->find(name);

    if (it == emotes->end())
      return boost::none;
    return it->second;
  }

  boost::optional<EmotePtr> IrcChannel::ffzChannelEmote(const EmoteName &name)
  {
    auto emotes = this->server()->ffzChannel();
    auto it = emotes->find(name);

    if (it == emotes->end())
      return boost::none;
    return it->second;
  }


  void IrcChannel::addMessageContent(MessageBuilder& builder, const QString& message) {
    QStringList words = message.split(' ');
    for (auto word : words) {
      auto emote = bttvEmote(word);
      auto ffzGlobalEmote = ffzEmote(word);
      auto bttvChannelEmote = bttvEmotex(EmoteName{word});
      auto ffzChanEmote = ffzChannelEmote(EmoteName{word});
      if (bttvChannelEmote) {
        builder.append(std::move(std::make_unique<EmoteElement>(bttvChannelEmote.get(), MessageElementFlag::BttvEmote)));
      } else if (ffzChanEmote) {
        builder.append(std::move(std::make_unique<EmoteElement>(ffzChanEmote.get(), MessageElementFlag::FfzEmote)));
      } else if (emote) {
        builder.append(std::move(emote.get()));
      } else if (ffzGlobalEmote) {
        builder.append(std::move(ffzGlobalEmote.get()));
      } else {
        builder.emplace<TextElement>(word, MessageElementFlag::Text);
      }
    }
  }

  MessagePtr IrcChannel::buildMessage(const QString &nick, const QString &message) {
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
