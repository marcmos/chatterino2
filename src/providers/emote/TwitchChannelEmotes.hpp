#pragma once

#include "providers/bttv/BttvEmotes.hpp"
#include "providers/ffz/FfzEmotes.hpp"
#include "providers/emoji/Emojis.hpp"
#include "messages/MessageElement.hpp"
#include "messages/Emote.hpp"

namespace chatterino {
class TwitchChannelEmotes
{
public:
    TwitchChannelEmotes(const QString &channelId);
    std::optional<EmotePtr> emote(const QString& name);
};
}  // namespace chatterino

// class TwitchChannelEmotes {
// public:
//     TwitchChannelEmotes(QString& channelId) {
//         BttvEmotes::loadChannel(channelId, [this](auto &&emoteMap) {
//             // emoteMaps[0] = std::make_shared<EmoteMap>(emoteMap);
//             bttv = std::move(emoteMap);
//         });
//         FfzEmotes::loadChannel(
//             channelId,
//             [this](auto &&emoteMap) {
//                 ffz = std::move(emoteMap);
//                 // emoteMaps[1] = std::make_shared<EmoteMap>(emoteMap);
//             },
//             [](auto emotePtr) { return; });
//     }

//     std::optional<EmotePtr> findEmote(const QString& name) {

//     }

// private:
//     boost::optional<EmoteMap> bttv, ffz;
//     // std::vector<std::shared_ptr<const EmoteMap>> emoteMaps = std::vector<std::shared_ptr<const EmoteMap>>(2);
//     // std::unique_ptr<const EmoteMap> bttvChannel_;
//     // std::unique_ptr<const EmoteMap> ffzChannel_;

//     boost::optional<EmotePtr> lookupEmoteMap(const EmoteMap& emoteMap,
//                                              const QString &name)
//     {
//         auto it = emoteMap.find(EmoteName{name});
//         if (it == emoteMap.end())
//             return boost::none;
//         return it->second;
//     }
// };
