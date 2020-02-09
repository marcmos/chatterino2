#pragma once

#include <memory>
#include "boost/optional.hpp"
#include "common/Aliases.hpp"
#include "common/Atomic.hpp"

#include <QJsonArray>
#include <QThread>

#include "common/Common.hpp"
#include "common/NetworkRequest.hpp"
#include "messages/Emote.hpp"
#include "messages/Image.hpp"
#include "messages/ImageSet.hpp"


namespace chatterino {

struct Emote;
using EmotePtr = std::shared_ptr<const Emote>;
class EmoteMap;

class CustomEmotes final
{
    static constexpr const char *globalEmoteApiUrl =
        "https://emotes.memleak.pl/3/cached/emotes/global";

public:
    CustomEmotes()
        : global_(std::make_shared<EmoteMap>())
    {
    }

    std::shared_ptr<const EmoteMap> emotes() const
    {
        return this->global_.get();
    }

    boost::optional<EmotePtr> emote(const EmoteName &name) const
    {
        auto emotes = this->global_.get();
        auto it = emotes->find(name);

        if (it == emotes->end())
            return boost::none;
        return it->second;
    }

    void loadEmotes()
    {
        NetworkRequest(QString(globalEmoteApiUrl))
            .timeout(30000)
            .onSuccess([this](auto result) -> Outcome {
                auto emotes = this->global_.get();
                auto pair = parseGlobalEmotes(result.parseJsonArray(), *emotes);
                if (pair.first)
                    this->global_.set(
                        std::make_shared<EmoteMap>(std::move(pair.second)));
                return pair.first;
            })
            .execute();
    }

private:
    Atomic<std::shared_ptr<const EmoteMap>> global_;

    QString emoteLinkFormat = QString("https://betterttv.com/emotes/%1");

    Url getEmoteLink(QString urlTemplate, const EmoteId &id,
                     const QString &emoteScale)
    {
        urlTemplate.detach();

        return {urlTemplate.replace("{{id}}", id.string)
                    .replace("{{image}}", emoteScale)};
    }

    Url getEmoteLinkV3(const EmoteId &id, const QString &emoteScale)
    {
        static const QString urlTemplate(
            "https://emotes.memleak.pl/emote/%1/%2");

        return {urlTemplate.arg(id.string, emoteScale)};
    }
    EmotePtr cachedOrMake(Emote &&emote, const EmoteId &id)
    {
        static std::unordered_map<EmoteId, std::weak_ptr<const Emote>> cache;
        static std::mutex mutex;

        return cachedOrMakeEmotePtr(std::move(emote), cache, mutex, id);
    }
    std::pair<Outcome, EmoteMap> parseGlobalEmotes(
        const QJsonArray &jsonEmotes, const EmoteMap &currentEmotes)
    {
        auto emotes = EmoteMap();

        for (auto jsonEmote : jsonEmotes)
        {
            auto id = EmoteId{jsonEmote.toObject().value("id").toString()};
            auto name =
                EmoteName{jsonEmote.toObject().value("code").toString()};

            auto emote = Emote({
                name,
                ImageSet{Image::fromUrl(getEmoteLinkV3(id, "1x"), 1),
                         Image::fromUrl(getEmoteLinkV3(id, "2x"), 0.5),
                         Image::fromUrl(getEmoteLinkV3(id, "3x"), 0.25)},
                Tooltip{name.string + "<br />Global BetterTTV Emote"},
                Url{emoteLinkFormat.arg(id.string)},
            });

            emotes[name] =
                cachedOrMakeEmotePtr(std::move(emote), currentEmotes);
        }

        return {Success, std::move(emotes)};
    }

 
};

}  // namespace chatterino
