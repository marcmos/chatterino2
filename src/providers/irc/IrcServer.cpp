#include "IrcServer.hpp"

#include <cassert>
#include <cstdlib>

#include "messages/Message.hpp"
#include "messages/MessageBuilder.hpp"
#include "providers/irc/Irc2.hpp"
#include "providers/irc/IrcChannel2.hpp"
#include "singletons/Settings.hpp"
#include "util/QObjectRef.hpp"

namespace chatterino {

QColor getRandomColor(const QString seed)
{
    static const std::vector<QColor> twitchUsernameColors = {
        {255, 0, 0},      // Red
        {0, 0, 255},      // Blue
        {0, 255, 0},      // Green
        {178, 34, 34},    // FireBrick
        {255, 127, 80},   // Coral
        {154, 205, 50},   // YellowGreen
        {255, 69, 0},     // OrangeRed
        {46, 139, 87},    // SeaGreen
        {218, 165, 32},   // GoldenRod
        {210, 105, 30},   // Chocolate
        {95, 158, 160},   // CadetBlue
        {30, 144, 255},   // DodgerBlue
        {255, 105, 180},  // HotPink
        {138, 43, 226},   // BlueViolet
        {0, 255, 127},    // SpringGreen
    };

    int hash = 0;

    for (int i = 0; i < seed.length(); i++)
    {
        hash += seed.at(i).toLatin1();
    }

    const auto colorIndex = hash % twitchUsernameColors.size();
    return twitchUsernameColors[colorIndex];
}

IrcServer::IrcServer(const IrcServerData &data)
    : data_(new IrcServerData(data))
{
    this->connect();
}

IrcServer::IrcServer(const IrcServerData &data,
                     const std::vector<std::weak_ptr<Channel>> &restoreChannels)
    : IrcServer(data)
{
    for (auto &&weak : restoreChannels)
    {
        if (auto shared = weak.lock())
        {
            this->channels[shared->getName()] = weak;
        }
    }
}

IrcServer::~IrcServer()
{
    delete this->data_;
}

int IrcServer::id()
{
    return this->data_->id;
}

const QString &IrcServer::user()
{
    return this->data_->user;
}

const QString &IrcServer::nick()
{
    return this->data_->nick.isEmpty() ? this->data_->user : this->data_->nick;
}

void IrcServer::initializeConnection(IrcConnection *connection,
                                     ConnectionType type)
{
    assert(type == Both);

    connection->setSecure(this->data_->ssl);
    connection->setHost(this->data_->host);
    connection->setPort(this->data_->port);

    connection->setUserName(this->data_->user);
    connection->setNickName(this->data_->nick.isEmpty() ? this->data_->user
                                                        : this->data_->nick);
    connection->setRealName(this->data_->real.isEmpty() ? this->data_->user
                                                        : this->data_->nick);

    if (getSettings()->enableExperimentalIrc)
    {
        switch (this->data_->authType)
        {
            case IrcAuthType::Sasl:
                connection->setSaslMechanism("PLAIN");
                [[fallthrough]];
            case IrcAuthType::Pass:
                this->data_->getPassword(
                    this, [conn = new QObjectRef(connection) /* can't copy */,
                           this](const QString &password) mutable {
                        if (*conn)
                        {
                            (*conn)->setPassword(password);
                            this->open(Both);
                        }

                        delete conn;
                    });
                break;
            default:
                this->open(Both);
        }
    }

    QObject::connect(
        connection, &Communi::IrcConnection::socketError, this,
        [this](QAbstractSocket::SocketError error) {
            static int index =
                QAbstractSocket::staticMetaObject.indexOfEnumerator(
                    "SocketError");

            std::lock_guard lock(this->channelMutex);

            for (auto &&weak : this->channels)
                if (auto shared = weak.lock())
                    shared->addMessage(makeSystemMessage(
                        QStringLiteral("Socket error: ") +
                        QAbstractSocket::staticMetaObject.enumerator(index)
                            .valueToKey(error)));
        });

    QObject::connect(connection, &Communi::IrcConnection::nickNameRequired,
                     this, [](const QString &reserved, QString *result) {
                         *result = reserved + (std::rand() % 100);
                     });

    QObject::connect(
        connection, &Communi::IrcConnection::noticeMessageReceived, this,
        [this](Communi::IrcNoticeMessage *message) {
            MessageBuilder builder;

            builder.emplace<TimestampElement>();
            builder.emplace<TextElement>(
                message->nick(), MessageElementFlag::Username,
                getRandomColor(message->nick()), FontStyle::ChatMediumBold);
            builder.emplace<TextElement>(
                "-> you:", MessageElementFlag::Username,
                getRandomColor(message->nick()), FontStyle::ChatMediumBold);
            builder.emplace<TextElement>(message->content(),
                                         MessageElementFlag::Text);

            auto msg = builder.release();

            for (auto &&weak : this->channels)
                if (auto shared = weak.lock())
                    shared->addMessage(msg);
        });
}

std::shared_ptr<Channel> IrcServer::createChannel(const QString &channelName)
{
    return std::make_shared<IrcChannel>(channelName, this);
}

bool IrcServer::hasSeparateWriteConnection() const
{
    return false;
}

void IrcServer::onReadConnected(IrcConnection *connection)
{
    {
        std::lock_guard lock(this->channelMutex);

        for (auto &&command : this->data_->connectCommands)
        {
            connection->sendRaw(command + "\r\n");
        }
    }

    AbstractIrcServer::onReadConnected(connection);
}

void IrcServer::privateMessageReceived(Communi::IrcPrivateMessage *message)
{
    auto target = message->target();
    // target = target.startsWith('#') ? target.mid(1) : target;

    if (auto channel = this->getChannelOrEmpty(target); !channel->isEmpty())
    {
        MessageBuilder builder;

        builder.emplace<TimestampElement>();
        builder.emplace<TextElement>(
            message->nick() + ":", MessageElementFlag::Username,
            getRandomColor(message->nick()), FontStyle::ChatMediumBold);
        std::static_pointer_cast<IrcChannel>(channel)->addMessageContent(
            builder, message->content());
        // builder.emplace<TextElement>(message->content(),
        // MessageElementFlag::Text);

        channel->addMessage(builder.release());
    }
}

void IrcServer::readConnectionMessageReceived(Communi::IrcMessage *message)
{
    AbstractIrcServer::readConnectionMessageReceived(message);

    switch (message->type())
    {
        case Communi::IrcMessage::Join: {
            auto x = static_cast<Communi::IrcJoinMessage *>(message);

            if (auto it =
                    this->channels.find(this->cleanChannelName(x->channel()));
                it != this->channels.end())
            {
                if (auto shared = it->lock())
                {
                    if (message->nick() == this->data_->nick)
                    {
                        shared->addMessage(
                            MessageBuilder(systemMessage, "joined").release());
                    }
                    else
                    {
                        if (auto c =
                                dynamic_cast<ChannelChatters *>(shared.get()))
                            c->addJoinedUser(x->nick());
                    }
                }
            }
            return;
        }

        case Communi::IrcMessage::Part: {
            auto x = static_cast<Communi::IrcPartMessage *>(message);

            if (auto it =
                    this->channels.find(this->cleanChannelName(x->channel()));
                it != this->channels.end())
            {
                if (auto shared = it->lock())
                {
                    if (message->nick() == this->data_->nick)
                    {
                        shared->addMessage(
                            MessageBuilder(systemMessage, "parted").release());
                    }
                    else
                    {
                        if (auto c =
                                dynamic_cast<ChannelChatters *>(shared.get()))
                            c->addPartedUser(x->nick());
                    }
                }
            }
            return;
        }

        case Communi::IrcMessage::Pong:
        case Communi::IrcMessage::Notice:
        case Communi::IrcMessage::Private:
            return;

        default:
            if (getSettings()->showUnhandledIrcMessages)
            {
                MessageBuilder builder;

                builder.emplace<TimestampElement>();
                builder.emplace<TextElement>(message->toData(),
                                             MessageElementFlag::Text);
                builder->flags.set(MessageFlag::Debug);

                auto msg = builder.release();

                for (auto &&weak : this->channels)
                {
                    if (auto shared = weak.lock())
                        shared->addMessage(msg);
                }
            };
    }
}

}  // namespace chatterino
