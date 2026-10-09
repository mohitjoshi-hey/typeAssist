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
static const char* const kEndings[] = {
    // domain endings
    "com", "org", "net", "edu", "gov", "io", "in", "co", "uk", "us", "de",
    "fr", "app", "dev", "ai", "me", "tv", "info", "biz", "xyz",
    // file extensions
    "txt", "md", "cpp", "hpp", "c", "h", "py", "js", "ts", "json", "html", "css",
    "xml", "csv", "pdf", "doc", "docx", "xls", "xlsx", "ppt", "pptx",
    "png", "jpg", "jpeg", "gif", "svg", "zip", "exe", "dll", "cs", "java",
    "go", "rs", "php", "rb", "sh", "bat", "yml", "yaml", "ini", "log", "sql"
};

static string toLower(const string& str) {
    string lower;
    lower.reserve(str.size());
    for (char ch : str) {
        lower += static_cast<char>(tolower(static_cast<unsigned char>(ch)));
    }
    return lower;
}

static bool isDomainOrExtension(const string& word) {
    string lower = toLower(word);
    for (const char* ending : kEndings) {
        if (lower == ending) {
            return true;
        }
    }
    return false;
}

static bool isDomainOrExtensionPrefix(const string& word) {
    if (word.empty()) return false;
    string lower = toLower(word);
    for (const char* ending : kEndings) {
        if (string(ending).rfind(lower, 0) == 0) {
            return true;
        }
    }
    return false;
}

static bool isVersionPattern(const string& token) {
    if (token.empty()) return false;
    size_t i = 0;
    if (token[0] == 'v' || token[0] == 'V') {
        i = 1;
    }
    if (i >= token.size() || !isdigit(static_cast<unsigned char>(token[i]))) {
        return false;
    }
    bool hasDot = false;
    bool hasDigitAfterDot = false;
    while (i < token.size()) {
        if (isdigit(static_cast<unsigned char>(token[i]))) {
            if (hasDot) hasDigitAfterDot = true;
            ++i;
        } else if (token[i] == '.') {
            hasDot = true;
            ++i;
        } else {
            return false;
        }
    }
    return hasDot && hasDigitAfterDot;
}

TextAction TextEngine::process(const KeyEvent& event) {
    if (event.type == KeyType::Backspace) {
        return handleBackspace();
    }
    return step(event);
}

TextAction TextEngine::step(const KeyEvent& event) {
    const TextState before = state_;

    // Space and Enter: word ended.
    if (event.type != KeyType::Character) {
        state_.spaceNeeded = false;
        state_.endPending = false;   // whitespace confirms a pending . ! ?
        state_.word.clear();
        state_.token.clear();
        state_.inUrl = false;
        state_.linkChar = false;
        state_.prevChar = '\0';
        state_.lastPunct = '\0';
        state_.punctBlocked = false;
        state_.glued.clear();
        state_.gluedSpaced = false;
        state_.tokenAutoCapitalized = false;
        if (event.type == KeyType::Enter) {
            // Rule 5: a new line starts a new sentence.
            state_.capitalizeNext = true;
        }
        remember(before);
        return {ActionType::PassThrough, ""};
    }

    size_t charsAdded = 1;
    size_t charsErased = 0;
    TextAction action = processCharacter(event.character, charsAdded, charsErased);

    while (charsErased > 0 && !history_.empty()) {
        history_.pop_back();
        --charsErased;
    }
    if (charsAdded == 2) {
        remember(before);
        TextState afterSpace = before;
        afterSpace.spaceNeeded = false;
        remember(afterSpace);
    } else {
        for (size_t i = 0; i < charsAdded; ++i) {
            remember(before);
        }
    }
    return action;
}

TextAction TextEngine::processCharacter(char ch, size_t& charsAdded, size_t& charsErased) {
    const unsigned char c = static_cast<unsigned char>(ch);
    charsAdded = 1;
    charsErased = 0;

    // --- Query delimiter '=' handling (e.g. "search?q=cats", "page?id=5") ---
    if (ch == '=') {
        if (state_.gluedSpaced) {
            size_t erase = state_.glued.size() + 1; // erase space + glued letters
            string text = state_.glued + "=";
            charsErased = erase;
            charsAdded = text.size();
            state_.glued.clear();
            state_.gluedSpaced = false;
            state_.lastPunct = '\0';
            state_.inUrl = true;
            state_.token += "=";
            state_.prevChar = '=';
            return {ActionType::Replace, text, erase};
        }
    }

    // --- Email delimiter '@' handling (e.g. "john@example.com", "john.doe@example.com") ---
    if (ch == '@') {
        state_.linkChar = true;
        if (state_.gluedSpaced) {
            size_t erase = state_.token.size();
            string uncap;
            for (char tc : state_.token) {
                if (tc != ' ') uncap += static_cast<char>(tolower(static_cast<unsigned char>(tc)));
            }
            string text = uncap + "@";
            charsErased = erase;
            charsAdded = text.size();
            state_.token = text;
            state_.tokenAutoCapitalized = false;
            state_.glued.clear();
            state_.gluedSpaced = false;
            state_.lastPunct = '\0';
            state_.prevChar = '@';
            return {ActionType::Replace, text, erase};
        } else if (state_.tokenAutoCapitalized) {
            size_t erase = state_.token.size();
            string text = toLower(state_.token) + "@";
            charsErased = erase;
            charsAdded = text.size();
            state_.token = text;
            state_.tokenAutoCapitalized = false;
            state_.glued.clear();
            state_.gluedSpaced = false;
            state_.lastPunct = '\0';
            state_.prevChar = '@';
            return {ActionType::Replace, text, erase};
        }
    }

    // --- Rule 2: a letter right after , or ; needs a space before it.
    string prefix;
    if (state_.spaceNeeded) {
        state_.spaceNeeded = false;
        if (isalpha(c)) {
            prefix = " ";
        }
    }

    // --- Rule 4: a . ! ? only ends a sentence if a space or Enter follows it.
    if (state_.endPending && isalnum(c)) {
        state_.endPending = false;
        state_.capitalizeNext = state_.capBeforeEnd;
    }

    // --- Real-time sentence spacing for '!' and '?' ---
    if ((state_.lastPunct == '!' || state_.lastPunct == '?') && isalpha(c) && !state_.inUrl) {
        char upper = static_cast<char>(toupper(c));
        string text = string(" ") + upper;
        charsAdded = 2;
        charsErased = 0;
        state_.lastPunct = '\0'; // Cleared so subsequent letters are typed normally
        state_.glued = string(1, static_cast<char>(tolower(c)));
        state_.gluedSpaced = true;
        state_.endPending = false;
        state_.capitalizeNext = false;
        state_.word = string(1, static_cast<char>(tolower(c)));
        state_.token += " ";
        state_.token += upper;
        state_.prevChar = upper;
        return {ActionType::Replace, text, 0};
    }

    // If query letters continue after '?' was spaced (e.g. "?id="):
    if (state_.gluedSpaced && isalpha(c)) {
        state_.glued += static_cast<char>(tolower(c));
    }

    // --- Real-time sentence spacing for '.' ---
    if (state_.lastPunct == '.' && isalpha(c) && !state_.punctBlocked) {
        string nextGlued = state_.glued + ch;
        string nextGluedLower = toLower(nextGlued);

        if (nextGlued.size() >= 2 && !isDomainOrExtensionPrefix(nextGluedLower)) {
            // Not a domain or file extension prefix: sentence punctuation with forgotten space.
            size_t erase = state_.glued.size();
            string text = " " + state_.glued + ch;
            charsErased = erase;
            charsAdded = text.size();
            state_.glued = nextGlued;
            state_.gluedSpaced = true;
            state_.lastPunct = '\0'; // sentence spacing completed
            state_.endPending = false;
            state_.capitalizeNext = false;
            state_.word = nextGluedLower;
            if (state_.token.size() >= erase) {
                state_.token.erase(state_.token.size() - erase);
            }
            state_.token += text;
            state_.prevChar = ch;
            return {ActionType::Replace, text, erase};
        } else {
            // Could still be a domain/extension prefix (or 1st letter).
            state_.glued = nextGlued;
        }
    } else if (c == '.' || c == '!' || c == '?') {
        // Punctuation tracking handled below
    } else {
        if (!isalpha(c)) {
            state_.lastPunct = '\0';
            state_.glued.clear();
        }
    }

    // --- Rule 1: capitalize the first letter of a sentence.
    bool sentenceEnd = false;
    string text(1, ch);
    if (state_.capitalizeNext && isalpha(c)) {
        state_.capitalizeNext = false;
        text[0] = static_cast<char>(toupper(c));
        if (state_.token.empty()) {
            state_.tokenAutoCapitalized = true;
        }
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

    // If user types a second uppercase letter, it was intentional (e.g. "README")
    if (isupper(c) && state_.token.size() > 0 && state_.tokenAutoCapitalized) {
        state_.tokenAutoCapitalized = false;
    }

    // Track punctuation state
    if (c == '.' || c == '!' || c == '?') {
        const bool continuesRun =
            state_.lastPunct != '\0' && state_.glued.empty(); // "..." or "?!"
        if (continuesRun) {
            // Multi-punctuation run ("..", "...", "?!")
            if (c == '.') {
                state_.punctBlocked = true; // Protect ellipsis from being split
            }
        } else {
            state_.punctBlocked =
                state_.token.empty() || state_.inUrl || state_.linkChar ||
                isdigit(static_cast<unsigned char>(state_.prevChar)) ||
                (c == '.' && toLower(state_.token) == "www");
            state_.punctEnds = false;
        }
        state_.punctEnds = state_.punctEnds || sentenceEnd;
        state_.lastPunct = ch;
        state_.glued.clear();
        state_.gluedSpaced = false;
    }

    // Track word
    if (isalpha(c)) {
        if (state_.word.size() < 32) {
            state_.word += static_cast<char>(tolower(c));
        }
    } else {
        state_.word.clear();
    }

    // Track token
    if (state_.token.size() < 64) {
        state_.token += text[0];
    }
    state_.prevChar = ch;

    // URL detection
    if (!state_.inUrl &&
        (state_.token.compare(0, 4, "www.") == 0 ||
         state_.token.compare(0, 4, "Www.") == 0 ||
         state_.token.find("://") != string::npos)) {
        state_.inUrl = true;
    }
    if (ch == '@' || ch == '/' || ch == '\\') {
        state_.linkChar = true;
    }

    // Comma / semicolon spacing
    if ((c == ',' || c == ';') && !state_.inUrl) {
        state_.spaceNeeded = true;
    }

    // Uncapitalize start of token if recognized as URL, domain, file, or version
    if (state_.tokenAutoCapitalized) {
        if (state_.token.size() >= 4 && state_.token.compare(0, 4, "Www.") == 0) {
            size_t erase = state_.token.size();
            string uncap = toLower(state_.token);
            charsErased = erase;
            charsAdded = uncap.size();
            state_.token = uncap;
            state_.tokenAutoCapitalized = false;
            return {ActionType::Replace, uncap, erase};
        }
        if (state_.token.find("://") != string::npos) {
            size_t erase = state_.token.size();
            string uncap = toLower(state_.token);
            charsErased = erase;
            charsAdded = uncap.size();
            state_.token = uncap;
            state_.tokenAutoCapitalized = false;
            return {ActionType::Replace, uncap, erase};
        }
        if (!state_.glued.empty() && isDomainOrExtension(state_.glued)) {
            size_t erase = state_.token.size();
            string uncap = toLower(state_.token);
            charsErased = erase;
            charsAdded = uncap.size();
            state_.token = uncap;
            state_.tokenAutoCapitalized = false;
            return {ActionType::Replace, uncap, erase};
        }
        if (isVersionPattern(state_.token)) {
            size_t erase = state_.token.size();
            string uncap = toLower(state_.token);
            charsErased = erase;
            charsAdded = uncap.size();
            state_.token = uncap;
            state_.tokenAutoCapitalized = false;
            return {ActionType::Replace, uncap, erase};
        }
    }

    if (!prefix.empty()) {
        charsAdded = 2;
        return {ActionType::Replace, prefix + text};
    }
    if (text[0] != ch) {
        return {ActionType::Replace, text};
    }
    return {ActionType::PassThrough, ""};
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