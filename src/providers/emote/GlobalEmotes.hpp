#pragma once

#include "Application.hpp"
#include "singletons/Emotes.hpp"
#include "providers/bttv/BttvEmotes.hpp"
#include "providers/ffz/FfzEmotes.hpp"
#include "providers/emoji/Emojis.hpp"
#include "messages/MessageElement.hpp"
#include "messages/Emote.hpp"

namespace chatterino {

// FIXME do zmerge'owania z singletons/Emotes.hpp
class GlobalEmotes
{
public:
    GlobalEmotes()
        : emojis_(getApp()->emotes->emojis)
    {
        bttv_.loadEmotes();
        ffz_.loadEmotes();
    }

    boost::optional<std::unique_ptr<EmoteElement>> emote(const QString &word)
    {
        boost::optional<EmotePtr> emotePtr;
        EmoteName name = EmoteName{word};
        if ((emotePtr = bttv_.emote(name)))
        {
            return std::make_unique<EmoteElement>(
                emotePtr.get(), MessageElementFlag::BttvEmote);
        }
        else if ((emotePtr = ffz_.emote(name)))
        {
            return std::make_unique<EmoteElement>(emotePtr.get(),
                                                  MessageElementFlag::FfzEmote);
        }
        else
        {
            auto emoji = emojis_.parse(word);
            EmotePtr *emojiPtr;
            if (emoji.size() > 0 &&
                (emojiPtr = boost::get<EmotePtr>(&emoji[0])))
            {
                return std::make_unique<EmoteElement>(
                    *emojiPtr, MessageElementFlag::EmojiAll);
            }
            else
            {
                return boost::none;
            }
        }
    }

private:
    BttvEmotes bttv_;
    FfzEmotes ffz_;
    Emojis &emojis_;
};

}  // namespace chatterino