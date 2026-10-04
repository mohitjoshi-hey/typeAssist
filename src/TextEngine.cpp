#include <typeassist/TextEngine.h>

#include <cctype>
#include <string>

using namespace std;

TextAction TextEngine::process(const KeyEvent& event) {
    if (event.type == KeyType::Backspace) {
        return handleBackspace();
    }

    HistoryEntry entry;
    entry.before = state_;

    // Space and Enter: the user handled spacing themselves.
    if (event.type != KeyType::Character) {
        state_.spaceNeeded = false;
        remember(entry);
        return {ActionType::PassThrough, ""};
    }

    TextAction action = processCharacter(event.character, entry.insertedSpace);
    remember(entry);
    return action;
}

TextAction TextEngine::processCharacter(char ch, bool& insertedSpace) {
    const unsigned char c = static_cast<unsigned char>(ch);

    // Rule 2: a letter right after , or ; needs a space before it.
    string prefix;
    if (state_.spaceNeeded) {
        state_.spaceNeeded = false;
        if (isalpha(c)) {
            prefix = " ";
        }
    }
    insertedSpace = !prefix.empty();

    // Rule 1: capitalize the first letter of a sentence.
    string text(1, ch);
    if (state_.capitalizeNext && isalpha(c)) {
        state_.capitalizeNext = false;
        text[0] = static_cast<char>(toupper(c));
    } else if (c == '.' || c == '!' || c == '?') {
        state_.capitalizeNext = true;
    }

    // Remember that a space is now expected after this punctuation.
    if (c == ',' || c == ';') {
        state_.spaceNeeded = true;
    }

    // Nothing changed: let the user's key through untouched.
    if (prefix.empty() && text[0] == ch) {
        return {ActionType::PassThrough, ""};
    }
    return {ActionType::Replace, prefix + text};
}

TextAction TextEngine::handleBackspace() {
    if (!history_.empty()) {
        const HistoryEntry last = history_.back();
        history_.pop_back();

        state_ = last.before;

        // Backspace removes only the user's key. A space we inserted before
        // it is still on screen, so we must not insert another one.
        if (last.insertedSpace) {
            state_.spaceNeeded = false;
        }
    }
    // Let the Backspace itself through to the application.
    return {ActionType::PassThrough, ""};
}

void TextEngine::remember(const HistoryEntry& entry) {
    history_.push_back(entry);
    if (history_.size() > kMaxHistory) {
        history_.pop_front();
    }
}