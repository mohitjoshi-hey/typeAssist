#include <typeassist/TextEngine.h>

#include <cctype>
#include <string>

using namespace std;

// Words whose trailing '.' does not end a sentence ("Mr. Smith").
static bool isAbbreviation(const string& word) {
    static const char* const kAbbreviations[] = {
        "mr", "mrs", "ms", "dr", "prof", "sr", "jr", "st", "vs"
    };
    for (const char* abbreviation : kAbbreviations) {
        if (word == abbreviation) {
            return true;
        }
    }
    return false;
}

TextAction TextEngine::process(const KeyEvent& event) {
    if (event.type == KeyType::Backspace) {
        return handleBackspace();
    }

    HistoryEntry entry;
    entry.before = state_;

    // Space and Enter: the user handled spacing themselves, and a word ended.
    if (event.type != KeyType::Character) {
        state_.spaceNeeded = false;
        state_.decimalPending = false;
        state_.lastWasDigit = false;
        state_.word.clear();
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

    // Rule 4: a digit right after "3." means the dot was a decimal point,
    // so undo the "sentence ended" effect of that dot.
    if (state_.decimalPending) {
        state_.decimalPending = false;
        if (isdigit(c)) {
            state_.capitalizeNext = state_.capBeforeDot;
        }
    }

    // Rule 1: capitalize the first letter of a sentence.
    string text(1, ch);
    if (state_.capitalizeNext && isalpha(c)) {
        state_.capitalizeNext = false;
        text[0] = static_cast<char>(toupper(c));
    } else if (state_.capitalizeNext && isdigit(c)) {
        // A sentence that starts with a number: nothing to capitalize.
        state_.capitalizeNext = false;
    } else if (c == '!' || c == '?') {
        state_.capitalizeNext = true;
    } else if (c == '.') {
        if (isAbbreviation(state_.word)) {
            // Rule 3: "Mr." does not end a sentence.
        } else {
            if (state_.lastWasDigit) {
                // Might be a decimal point: wait for the next key.
                state_.decimalPending = true;
                state_.capBeforeDot = state_.capitalizeNext;
            }
            state_.capitalizeNext = true;
        }
    }

    // Track the word being typed, and whether the last key was a digit.
    if (isalpha(c)) {
        if (state_.word.size() < 16) {
            state_.word += static_cast<char>(tolower(c));
        }
    } else {
        state_.word.clear();
    }
    state_.lastWasDigit = isdigit(c) != 0;

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