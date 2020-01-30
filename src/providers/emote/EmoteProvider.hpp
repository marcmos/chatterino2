#include "providers/bttv/BttvEmotes.hpp"
#include "providers/ffz/FfzEmotes.hpp"
#include "messages/MessageElement.hpp"
#include "messages/Emote.hpp"

namespace chatterino {

class EmoteProvider
{
public:
    EmoteProvider()
    {
        refresh();
    };

    void refresh()
    {
        bttv_.loadEmotes();
        ffz_.loadEmotes();
    }

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

    boost::optional<std::unique_ptr<EmoteElement>> tryEmote(const QString &word)
    {
        EmoteName emoteName = EmoteName{word};
        boost::optional<EmotePtr> emote;

        for (auto& emoteMap : emoteMaps) {
            if ((emote = lookupEmoteMap(emoteMap.get(), word)))
            {
                return std::make_unique<EmoteElement>(
                    emote.get(), MessageElementFlag::BttvEmote);
            }
        }
        if ((emote = bttv_.emote(emoteName)))
        {
            return std::make_unique<EmoteElement>(
                emote.get(), MessageElementFlag::BttvEmote);
        }
        else if ((emote = ffz_.emote(emoteName)))
        {
            return std::make_unique<EmoteElement>(emote.get(),
                                                  MessageElementFlag::FfzEmote);
        }
        else
        {
            return boost::none;
        }
    }

private:
    BttvEmotes bttv_;
    FfzEmotes ffz_;

    // add mutex guard
    std::list<std::shared_ptr<const EmoteMap>> emoteMaps;

    std::shared_ptr<const EmoteMap> bttvChannel_;
    std::shared_ptr<const EmoteMap> ffzChannel_;
};

};