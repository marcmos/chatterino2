#pragma once

#include "providers/bttv/BttvEmotes.hpp"
#include "providers/ffz/FfzEmotes.hpp"
#include "providers/emoji/Emojis.hpp"
#include "messages/MessageElement.hpp"
#include "messages/Emote.hpp"
#include "providers/emote/GlobalEmotes.hpp"

namespace chatterino {

class EmoteProvider
{
public:
    EmoteProvider() {}

    void loadChannelEmotes(const std::vector<QString> channelIds)
    {
        for (auto &channelId : channelIds)
        {
            BttvEmotes::loadChannel(channelId, [this](auto &&emoteMap) {
                emoteMaps.push_back(
                    std::make_shared<EmoteMap>(std::move(emoteMap)));
            });
            FfzEmotes::loadChannel(
                channelId,
                [this](auto &&emoteMap) {
                    emoteMaps.push_back(
                        std::make_shared<EmoteMap>(std::move(emoteMap)));
                },
                [](auto emotePtr) { return; });
        }
    }

    boost::optional<EmotePtr> lookupEmoteMap(const EmoteMap *emoteMap,
                                             const QString &name)
    {
        auto it = emoteMap->find(EmoteName {name});
        if (it == emoteMap->end())
            return boost::none;
        return it->second;
    }

    boost::optional<std::unique_ptr<EmoteElement>> tryEmote(const QString &word) {
        if (auto emoteElement = globalEmotes_.emote(word)) {
            return emoteElement;
        } else {
            for (auto& emoteMap : emoteMaps) {
                if (auto emote = lookupEmoteMap(emoteMap.get(), word))
                {
                    return std::make_unique<EmoteElement>(
                        // broken tag
                        emote.get(), MessageElementFlag::BttvEmote);
                }
            }
            return boost::none;
        }
    }

private:
    GlobalEmotes globalEmotes_;

    // add mutex guard
    std::list<std::shared_ptr<const EmoteMap>> emoteMaps;
};

};