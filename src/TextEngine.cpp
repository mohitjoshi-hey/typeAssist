#include <typeassist/TextEngine.h>

#include <cctype>
#include <string>

using namespace std;

static string toLower(const string& str) {
    string lower;
    lower.reserve(str.size());
    for (char ch : str) {
        lower += static_cast<char>(tolower(static_cast<unsigned char>(ch)));
    }
    return lower;
}

// Words whose trailing '.' does not end a sentence ("Mr. Smith", "etc.").
static bool isAbbreviation(const string& word) {
    static const char* const kAbbreviations[] = {
        "mr", "mrs", "ms", "dr", "prof", "sr", "jr", "st", "vs",
        "etc", "al", "eg", "ie",
        "rev", "hon", "gen", "col", "maj", "capt", "lt", "sgt", "cpl", "pvt", "rep", "sen", "gov",
        "approx", "apt", "dept", "est", "fig", "inc", "ltd", "corp", "vol", "no"
    };
    string lower = toLower(word);
    for (const char* abbreviation : kAbbreviations) {
        if (lower == abbreviation) {
            return true;
        }
    }
    return false;
}

// Multi-dot abbreviations whose trailing '.' does not end a sentence ("e.g.", "i.e.", "p.m.").
static bool isDottedAbbreviation(const string& token) {
    if (token.empty() || token.back() != '.') return false;
    string lower = toLower(token);
    // Find where the dotted sequence starts (e.g. after space or '(')
    size_t start = 0;
    while (start < lower.size() && !isalpha(static_cast<unsigned char>(lower[start]))) {
        ++start;
    }
    string sub = lower.substr(start);
    static const char* const kDottedAbbrs[] = {
        "e.g.", "i.e.", "a.m.", "p.m.", "u.s.", "u.k.", "d.c.", "b.c.", "a.d.", "ph.d.", "u.s.a."
    };
    for (const char* abbr : kDottedAbbrs) {
        if (sub == abbr) return true;
    }
    return false;
}

static bool isNaturallyLowercaseCodeObject(const string& word) {
    static const char* const kLowerCodeObjects[] = {
        "this", "self", "cls", "super",
        "console", "window", "document", "element", "event",
        "file", "path", "fs", "os", "io", "arr",
        "req", "res", "app", "ctx", "props", "state",
        "env", "db", "api", "log", "err", "std",
        "config", "server", "client", "node", "process"
    };
    string lower = toLower(word);
    for (const char* obj : kLowerCodeObjects) {
        if (lower == obj) return true;
    }
    return false;
}

// Built-in objects and keywords frequently used with dot member access in code.
static bool isCodeObject(const string& word) {
    if (isNaturallyLowercaseCodeObject(word)) return true;
    static const char* const kPascalCodeObjects[] = {
        "math", "string", "object", "array", "json", "system"
    };
    string lower = toLower(word);
    for (const char* obj : kPascalCodeObjects) {
        if (lower == obj) return true;
    }
    return false;
}

static bool hasCamelCase(const string& str) {
    bool hasLower = false;
    for (char ch : str) {
        if (!isalpha(static_cast<unsigned char>(ch))) {
            return false;
        }
        if (islower(static_cast<unsigned char>(ch))) {
            hasLower = true;
        } else if (isupper(static_cast<unsigned char>(ch)) && hasLower) {
            return true;
        }
    }
    return false;
}

static bool isCodeContext(const string& token, const string& wordBeforeDot) {
    if (token.empty()) return false;
    // Contains underscores (e.g. my_var, user_id, is_valid)
    if (token.find('_') != string::npos) return true;
    // Contains C++ scope resolution or arrow (e.g. std::vector, ptr->field)
    if (token.find("::") != string::npos || token.find("->") != string::npos) return true;
    // Contains array brackets (e.g. items[0], arr[i])
    if (token.find('[') != string::npos || token.find(']') != string::npos) return true;
    // Contains code symbols like $, #
    if (token.find('$') != string::npos || token.find('#') != string::npos) return true;
    // Check identifier before dot for camelCase
    size_t dotPos = token.find('.');
    string id = (dotPos != string::npos) ? token.substr(0, dotPos) : token;
    if (hasCamelCase(id)) return true;
    if (token.find('.') != string::npos && isCodeObject(wordBeforeDot)) return true;
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

TextAction TextEngine::handleArrowLeft() {
    if (cursorOffset_ < history_.size()) {
        ++cursorOffset_;
    }
    state_.spaceNeeded = false;
    state_.endPending = false;
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
    state_.capitalizeNext = false;
    return {ActionType::PassThrough, ""};
}

TextAction TextEngine::handleArrowRight() {
    if (cursorOffset_ > 0) {
        --cursorOffset_;
    }
    state_.spaceNeeded = false;
    state_.endPending = false;
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
    state_.capitalizeNext = false;
    return {ActionType::PassThrough, ""};
}

TextAction TextEngine::handleArrowUpDown() {
    cursorOffset_ = 0;
    history_.clear();
    state_ = TextState{};
    state_.capitalizeNext = false;
    return {ActionType::PassThrough, ""};
}

TextAction TextEngine::handleHome() {
    cursorOffset_ = history_.size();
    state_.spaceNeeded = false;
    state_.endPending = false;
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
    state_.capitalizeNext = true;
    return {ActionType::PassThrough, ""};
}

TextAction TextEngine::handleEnd() {
    cursorOffset_ = 0;
    state_.spaceNeeded = false;
    state_.endPending = false;
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
    state_.capitalizeNext = false;
    return {ActionType::PassThrough, ""};
}

TextAction TextEngine::process(const KeyEvent& event) {
    if (event.type == KeyType::Backspace) {
        return handleBackspace();
    }
    if (event.type == KeyType::ArrowLeft) {
        return handleArrowLeft();
    }
    if (event.type == KeyType::ArrowRight) {
        return handleArrowRight();
    }
    if (event.type == KeyType::ArrowUp || event.type == KeyType::ArrowDown) {
        return handleArrowUpDown();
    }
    if (event.type == KeyType::Home) {
        return handleHome();
    }
    if (event.type == KeyType::End) {
        return handleEnd();
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

    // Case 1: Uncapitalization of token (e.g. main.cpp, userAccount)
    if (before.tokenAutoCapitalized && !state_.tokenAutoCapitalized && charsErased > 0) {
        size_t count = min(charsErased, history_.size());
        auto it = history_.end() - count;
        if (count > 0 && it->token.empty()) {
            ++it;
        }
        while (it != history_.end()) {
            if (!it->token.empty()) {
                it->token[0] = static_cast<char>(tolower(static_cast<unsigned char>(it->token[0])));
            }
            it->tokenAutoCapitalized = false;
            ++it;
        }
        if (charsAdded > charsErased) {
            TextState modifiedBefore = before;
            if (!modifiedBefore.token.empty()) {
                modifiedBefore.token[0] = static_cast<char>(tolower(static_cast<unsigned char>(modifiedBefore.token[0])));
            }
            modifiedBefore.tokenAutoCapitalized = false;
            for (size_t i = 0; i < charsAdded - charsErased; ++i) {
                remember(modifiedBefore);
            }
        }
        return action;
    }

    // Case 2: Real-time sentence spacing for period (e.g. "hello.wo" -> " wo", erasing 'w')
    if (before.lastPunct == '.' && action.type == ActionType::Replace &&
        !action.text.empty() && action.text[0] == ' ' && charsErased > 0 &&
        charsAdded == charsErased + 2) {
        size_t toPop = charsErased - 1;
        while (toPop > 0 && !history_.empty()) {
            history_.pop_back();
            --toPop;
        }
        TextState afterSpace = history_.empty() ? before : history_.back();
        afterSpace.token += " ";
        afterSpace.prevChar = ' ';
        afterSpace.gluedSpaced = true;
        afterSpace.lastPunct = '\0';
        afterSpace.endPending = false;
        afterSpace.capitalizeNext = false;
        remember(afterSpace);

        TextState cur = afterSpace;
        for (size_t i = 0; i < before.glued.size(); ++i) {
            char gc = before.glued[i];
            cur.token += gc;
            cur.prevChar = gc;
            cur.glued = before.glued.substr(0, i + 1);
            cur.word = toLower(cur.glued);
            if (i + 1 < before.glued.size()) {
                remember(cur);
            }
        }
        remember(cur);
        return action;
    }

    // Case 3: Real-time sentence spacing for ! and ? (e.g. "hello!w" -> " W")
    if ((before.lastPunct == '!' || before.lastPunct == '?') && charsAdded == 2 && charsErased == 0) {
        remember(before);
        TextState afterSpace = before;
        afterSpace.lastPunct = '\0';
        afterSpace.token += " ";
        afterSpace.prevChar = ' ';
        afterSpace.word.clear();
        afterSpace.glued.clear();
        afterSpace.gluedSpaced = true;
        afterSpace.capitalizeNext = true;
        afterSpace.endPending = false;
        remember(afterSpace);
        return action;
    }

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

    // --- Method call '(' handling (e.g. "obj.method()", "cat.speak()") ---
    if (ch == '(') {
        if (state_.gluedSpaced) {
            size_t erase = state_.glued.size() + 1; // erase space + glued letters
            string text = state_.glued + "(";
            charsErased = erase;
            charsAdded = text.size();
            state_.glued.clear();
            state_.gluedSpaced = false;
            state_.lastPunct = '\0';
            state_.token += "(";
            state_.prevChar = '(';
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

    // If letters after '.' become camelCase (e.g. "obj.getName"), undo forgotten space.
    if (state_.gluedSpaced && isupper(c) && !state_.glued.empty() &&
        islower(static_cast<unsigned char>(state_.glued.back()))) {
        if (state_.tokenAutoCapitalized) {
            size_t erase = state_.token.size();
            string uncap = state_.token;
            uncap[0] = static_cast<char>(tolower(static_cast<unsigned char>(uncap[0])));
            size_t spacePos = uncap.rfind(' ');
            if (spacePos != string::npos) {
                uncap.erase(spacePos, 1);
            }
            uncap += ch;
            charsErased = erase;
            charsAdded = uncap.size();
            state_.glued += ch;
            state_.gluedSpaced = false;
            state_.lastPunct = '\0';
            state_.token = uncap;
            state_.tokenAutoCapitalized = false;
            state_.prevChar = ch;
            return {ActionType::Replace, uncap, erase};
        } else {
            size_t erase = state_.glued.size() + 1; // erase space + glued letters
            string text = state_.glued + ch;
            charsErased = erase;
            charsAdded = text.size();
            state_.glued = text;
            state_.gluedSpaced = false;
            state_.lastPunct = '\0';
            state_.token += ch;
            state_.prevChar = ch;
            return {ActionType::Replace, text, erase};
        }
    }

    // If chained '.' in code/identifier arrives after gluedSpaced (e.g. "a.b.c"):
    if (ch == '.' && state_.gluedSpaced) {
        size_t erase = state_.glued.size() + 1; // erase space + glued letters
        string text = state_.glued + ".";
        charsErased = erase;
        charsAdded = text.size();
        state_.glued.clear();
        state_.gluedSpaced = false;
        state_.lastPunct = '.';
        state_.punctBlocked = true;
        state_.token += ".";
        state_.prevChar = '.';
        return {ActionType::Replace, text, erase};
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
        bool isAbbr = (c == '.' && (isAbbreviation(state_.word) || isDottedAbbreviation(state_.token + ".")));
        bool isCode = (c == '.' && (isCodeContext(state_.token, state_.word) || isCodeObject(state_.word)));
        if (isAbbr || isCode) {
            // Rule 3: Abbreviations and code member access do not end a sentence.
            sentenceEnd = false;
            state_.endPending = false;
            state_.capitalizeNext = false;
        } else {
            sentenceEnd = true;
            if (!state_.endPending) {
                state_.endPending = true;
                state_.capBeforeEnd = state_.capitalizeNext;
            }
            state_.capitalizeNext = true;
        }
    }

    // If user types a second uppercase letter:
    // If preceded by lowercase letters, it's camelCase (e.g. "userAccount"), so uncapitalize first letter.
    // If all preceding letters were uppercase, it's an intentional acronym (e.g. "README").
    if (isupper(c) && state_.token.size() > 0 && state_.tokenAutoCapitalized) {
        bool hasLowerSoFar = false;
        for (char tc : state_.token) {
            if (islower(static_cast<unsigned char>(tc))) {
                hasLowerSoFar = true;
                break;
            }
        }
        if (hasLowerSoFar && state_.token.find('.') == string::npos) {
            // camelCase detected! Uncapitalize the leading letter that was auto-capitalized.
            size_t erase = state_.token.size();
            string uncap = state_.token;
            uncap[0] = static_cast<char>(tolower(static_cast<unsigned char>(uncap[0])));
            uncap += ch;
            charsErased = erase;
            charsAdded = uncap.size();
            state_.token = uncap;
            state_.tokenAutoCapitalized = false;
            state_.word += static_cast<char>(tolower(c));
            state_.prevChar = ch;
            return {ActionType::Replace, uncap, erase};
        } else {
            // Acronym like "README"
            state_.tokenAutoCapitalized = false;
        }
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
            bool codeToken = (c == '.' && (isCodeContext(state_.token, state_.word) || isCodeObject(state_.word)));
            state_.punctBlocked =
                state_.token.empty() || state_.inUrl || state_.linkChar ||
                codeToken ||
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
        state_.capitalizeNext = false;
        state_.endPending = false;
    }

    // Uncapitalize start of token if recognized as URL, domain, file, version, or code
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
            size_t dotPos = state_.token.rfind('.');
            string prefixBeforeDot = (dotPos != string::npos) ? state_.token.substr(0, dotPos + 1) : "";
            if (!isDottedAbbreviation(prefixBeforeDot) && !isAbbreviation(toLower(state_.token.substr(0, dotPos)))) {
                size_t erase = state_.token.size();
                string uncap = toLower(state_.token);
                charsErased = erase;
                charsAdded = uncap.size();
                state_.token = uncap;
                state_.tokenAutoCapitalized = false;
                return {ActionType::Replace, uncap, erase};
            }
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
        if (isCodeContext(state_.token, state_.word) ||
            (state_.token.find('.') != string::npos && isNaturallyLowercaseCodeObject(state_.token.substr(0, state_.token.find('.'))))) {
            size_t dotPos = state_.token.find('.');
            string prefixBeforeDot = (dotPos != string::npos) ? state_.token.substr(0, dotPos) : state_.token;
            if (isNaturallyLowercaseCodeObject(prefixBeforeDot) ||
                state_.token.find('_') != string::npos ||
                state_.token.find("::") != string::npos ||
                state_.token.find("->") != string::npos ||
                state_.token.find('[') != string::npos ||
                hasCamelCase(prefixBeforeDot)) {
                size_t erase = state_.token.size();
                string uncap = state_.token;
                uncap[0] = static_cast<char>(tolower(static_cast<unsigned char>(uncap[0])));
                charsErased = erase;
                charsAdded = uncap.size();
                state_.token = uncap;
                state_.tokenAutoCapitalized = false;
                return {ActionType::Replace, uncap, erase};
            }
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
        if (cursorOffset_ == 0) {
            state_ = history_.back();
            history_.pop_back();
        } else {
            if (cursorOffset_ < history_.size()) {
                size_t index = history_.size() - cursorOffset_ - 1;
                state_ = history_[index];
                history_.erase(history_.begin() + index);
            }
        }
    }
    // Let the Backspace itself through to the application.
    return {ActionType::PassThrough, ""};
}

void TextEngine::remember(const TextState& before) {
    if (cursorOffset_ == 0) {
        history_.push_back(before);
    } else {
        size_t index = history_.size() - cursorOffset_;
        history_.insert(history_.begin() + index, before);
    }
    if (history_.size() > kMaxHistory) {
        history_.pop_front();
    }
}