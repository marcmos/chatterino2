#pragma once

#include "Application.hpp"
#include "singletons/Emotes.hpp"
#include "providers/bttv/BttvEmotes.hpp"
#include "providers/ffz/FfzEmotes.hpp"
#include "providers/emoji/Emojis.hpp"
#include "messages/MessageElement.hpp"
#include "messages/Emote.hpp"
#include "common/CompletionModel.hpp"
#include "CustomEmotes.hpp"

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
        customEmotes_.loadEmotes();
    }

    boost::optional<std::unique_ptr<EmoteElement>> emote(const QString &word) const
    {
        boost::optional<EmotePtr> emotePtr;
        EmoteName name = EmoteName{word};
        if ((emotePtr = bttv_.emote(name)))
        {
            return std::make_unique<EmoteElement>(
                emotePtr.get(), MessageElementFlag::BttvEmote);
        }
        else if ((emotePtr = customEmotes_.emote(name)))
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

    std::vector<CompletionModel::TaggedString> words() const
    {
        std::vector<CompletionModel::TaggedString> result;

        for (auto &emote : *bttv_.emotes())
        {
            result.emplace_back(
                emote.first.string,
                CompletionModel::TaggedString::Type::BTTVChannelEmote);
        }

        for (auto &emote : *customEmotes_.emotes())
        {
            result.emplace_back(
                emote.first.string,
                CompletionModel::TaggedString::Type::BTTVChannelEmote);
        }

        for (auto &emote : *ffz_.emotes())
        {
            result.emplace_back(
                emote.first.string,
                CompletionModel::TaggedString::Type::FFZChannelEmote);
        }

        // Emojis
        const auto &emojiShortCodes = emojis_.shortCodes;
        for (auto &m : emojiShortCodes)
        {
            result.emplace_back(":" + m + ":",
                                CompletionModel::TaggedString::Type::Emoji);
        }

        return result;
    }

private:
    BttvEmotes bttv_;
    FfzEmotes ffz_;
    Emojis &emojis_;
    CustomEmotes customEmotes_;
};

}  // namespace chatterino