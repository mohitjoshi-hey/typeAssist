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

// Endings that legitimately follow a '.' without a space: domains and files.
static bool isDomainOrExtension(const string& word) {
    static const char* const kEndings[] = {
        // domain endings
        "com", "org", "net", "edu", "gov", "io", "in", "co", "uk", "us", "de",
        "fr", "app", "dev", "ai", "me", "tv", "info", "biz", "xyz",
        // file extensions
        "txt", "md", "cpp", "hpp", "py", "js", "ts", "json", "html", "css",
        "xml", "csv", "pdf", "doc", "docx", "xls", "xlsx", "ppt", "pptx",
        "png", "jpg", "jpeg", "gif", "svg", "zip", "exe", "dll", "cs", "java",
        "go", "rs", "php", "rb", "sh", "bat", "yml", "yaml", "ini", "log", "sql"
    };
    string lower;
    for (char ch : word) {
        lower += static_cast<char>(tolower(static_cast<unsigned char>(ch)));
    }
    for (const char* ending : kEndings) {
        if (lower == ending) {
            return true;
        }
    }
    return false;
}

TextAction TextEngine::process(const KeyEvent& event) {
    if (event.type == KeyType::Backspace) {
        return handleBackspace();
    }

    // Rule 7: a word stuck to a . ! ? ("hello!world") has just ended with a
    // space or Enter. Put the forgotten space in, by deleting the stuck-on
    // letters and typing them again after a space.
    if (event.type != KeyType::Character && gluedWordNeedsSpace()) {
        string word = state_.glued;
        if (state_.punctEnds) {
            // A new sentence starts here, so capitalize it.
            word[0] = static_cast<char>(
                toupper(static_cast<unsigned char>(word[0])));
        }
        const size_t length = state_.glued.size();

        // Undo the stuck-on letters, then replay them after a space so the
        // state and the Backspace history match what is now on screen.
        state_ = history_[history_.size() - length];
        history_.erase(history_.end() - static_cast<long long>(length),
                       history_.end());
        step(KeyEvent{KeyType::Space, ' '});
        for (char ch : word) {
            step(KeyEvent{KeyType::Character, ch});
        }
        step(event);

        string text = " " + word;
        text += (event.type == KeyType::Enter) ? '\n' : ' ';
        return {ActionType::Replace, text, length};
    }

    return step(event);
}

TextAction TextEngine::step(const KeyEvent& event) {
    const TextState before = state_;

    // Space and Enter: the user handled spacing themselves, and a word ended.
    if (event.type != KeyType::Character) {
        state_.spaceNeeded = false;
        state_.endPending = false;   // whitespace confirms a pending . ! ?
        state_.word.clear();
        state_.token.clear();
        state_.inUrl = false;
        state_.linkChar = false;
        state_.prevChar = '\0';
        state_.lastPunct = '\0';
        state_.glued.clear();
        if (event.type == KeyType::Enter) {
            // Rule 5: a new line starts a new sentence.
            state_.capitalizeNext = true;
        }
        remember(before);
        return {ActionType::PassThrough, ""};
    }

    bool insertedSpace = false;
    TextAction action = processCharacter(event.character, insertedSpace);
    if (insertedSpace) {
        // This key put two characters on screen (a space, then the key),
        // so it gets two history entries.
        remember(before);
        TextState afterSpace = before;
        afterSpace.spaceNeeded = false;
        remember(afterSpace);
    } else {
        remember(before);
    }
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

    // Rule 4: a . ! ? only ends a sentence if a space or Enter follows it.
    // "3.5", "example.com", "main.cpp" and "page?id=5" put a letter or digit
    // right after the punctuation, so undo its "sentence ended" effect.
    if (state_.endPending && isalnum(c)) {
        state_.endPending = false;
        state_.capitalizeNext = state_.capBeforeEnd;
    }

    // Rule 1: capitalize the first letter of a sentence.
    bool sentenceEnd = false;
    string text(1, ch);
    if (state_.capitalizeNext && isalpha(c)) {
        state_.capitalizeNext = false;
        text[0] = static_cast<char>(toupper(c));
    } else if (state_.capitalizeNext && isdigit(c)) {
        // A sentence that starts with a number: nothing to capitalize.
        state_.capitalizeNext = false;
    } else if (c == '.' || c == '!' || c == '?') {
        if (c == '.' && isAbbreviation(state_.word)) {
            // Rule 3: "Mr." does not end a sentence.
        } else {
            sentenceEnd = true;
            if (!state_.endPending) {
                state_.endPending = true;
                state_.capBeforeEnd = state_.capitalizeNext;
            }
            state_.capitalizeNext = true;
        }
    }

    // Rule 7: remember letters stuck to a . ! ? so a forgotten space can be
    // put in when the word ends (see gluedWordNeedsSpace).
    if (c == '.' || c == '!' || c == '?') {
        const bool continuesRun =
            state_.lastPunct != '\0' && state_.glued.empty();  // "..." or "?!"
        if (!continuesRun) {
            // Links, emails, paths, numbers ("3.x"), dotfiles (".gitignore")
            // and "www." can't be a forgotten space.
            state_.punctBlocked =
                state_.token.empty() || state_.inUrl || state_.linkChar ||
                isdigit(static_cast<unsigned char>(state_.prevChar)) ||
                (c == '.' && state_.token == "www");
            state_.punctEnds = false;
        }
        state_.punctEnds = state_.punctEnds || sentenceEnd;
        state_.lastPunct = ch;
        state_.glued.clear();
    } else if (isalpha(c) && state_.lastPunct != '\0' &&
               state_.glued.size() < 16) {
        state_.glued += ch;
    } else {
        state_.lastPunct = '\0';
        state_.glued.clear();
    }

    // Track the word being typed (letters only).
    if (isalpha(c)) {
        if (state_.word.size() < 16) {
            state_.word += static_cast<char>(tolower(c));
        }
    } else {
        state_.word.clear();
    }

    // Rule 6: track the start of the whole token to recognise links.
    if (state_.token.size() < 12) {
        state_.token += static_cast<char>(tolower(c));
    }
    if (!state_.inUrl &&
        (state_.token.compare(0, 4, "www.") == 0 ||
         state_.token.find("://") != string::npos)) {
        state_.inUrl = true;
    }
    if (ch == '@' || ch == '/' || ch == '\\') {
        state_.linkChar = true;
    }
    state_.prevChar = ch;

    // Remember that a space is now expected after this punctuation,
    // except inside a link, where a comma belongs to the address.
    if ((c == ',' || c == ';') && !state_.inUrl) {
        state_.spaceNeeded = true;
    }

    // Nothing changed: let the user's key through untouched.
    if (prefix.empty() && text[0] == ch) {
        return {ActionType::PassThrough, ""};
    }
    return {ActionType::Replace, prefix + text};
}

// True when the word that just ended was stuck to a . ! ? with no space.
bool TextEngine::gluedWordNeedsSpace() const {
    const TextState& s = state_;
    if (s.lastPunct == '\0' || s.punctBlocked || s.glued.size() < 2) {
        return false;
    }
    if (history_.size() < s.glued.size()) {
        return false;
    }
    if (s.lastPunct == '.') {
        // After a dot only a capital letter clearly starts a new sentence;
        // lowercase could be a domain, a file name or code ("self.name").
        if (!isupper(static_cast<unsigned char>(s.glued[0]))) {
            return false;
        }
        if (isDomainOrExtension(s.glued)) {
            return false;
        }
    }
    return true;
}

TextAction TextEngine::handleBackspace() {
    if (!history_.empty()) {
        state_ = history_.back();
        history_.pop_back();
    }
    // Let the Backspace itself through to the application.
    return {ActionType::PassThrough, ""};
}

void TextEngine::remember(const TextState& before) {
    history_.push_back(before);
    if (history_.size() > kMaxHistory) {
        history_.pop_front();
    }
}