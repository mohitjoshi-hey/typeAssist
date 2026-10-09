#include <typeassist/TextEngine.h>

#include <algorithm>
#include <iostream>
#include <string>

using namespace std;

static KeyType keyTypeFor(char c) {
    if (c == ' ') return KeyType::Space;
    if (c == '\b') return KeyType::Backspace;
    if (c == '\n') return KeyType::Enter;
    return KeyType::Character;
}

// Types a string through a fresh engine and returns what the screen shows.
// A '\b' in the input means the user pressed Backspace.
static string simulateTyping(const string& input) {
    TextEngine engine;
    string screen;

    for (char c : input) {
        KeyEvent event{keyTypeFor(c), c};
        TextAction action = engine.process(event);

        if (event.type == KeyType::Backspace) {
            if (!screen.empty()) screen.pop_back();
        } else if (action.type == ActionType::Replace) {
            // delete `erase` characters, then type the replacement text
            screen.erase(screen.size() - min(action.erase, screen.size()));
            screen += action.text;
        } else {
            screen += c;
        }
    }
    return screen;
}

static int failures = 0;

// Shows Backspace as <BS> and Enter as <Enter> so output stays on one line.
static string printable(const string& s) {
    string out;
    for (char c : s) {
        if (c == '\b') out += "<BS>";
        else if (c == '\n') out += "<Enter>";
        else out += c;
    }
    return out;
}

static void check(const string& input, const string& expected) {
    const string actual = simulateTyping(input);
    if (actual == expected) {
        cout << "PASS: \"" << printable(input) << "\"\n";
    } else {
        ++failures;
        cout << "FAIL: \"" << printable(input) << "\"\n"
             << "  expected: \"" << printable(expected) << "\"\n"
             << "  actual:   \"" << printable(actual) << "\"\n";
    }
}

int main() {
    // --- Capitalization ---
    check("hello. world",         "Hello. World");
    check("hello! how are you?",  "Hello! How are you?");
    check("hello? yes.",          "Hello? Yes.");
    check("hello world",          "Hello world");
    check("Hello. World",         "Hello. World");
    check("hello... world",       "Hello... World");
    check("hi. ok. fine",         "Hi. Ok. Fine");

    // --- Punctuation spacing ---
    check("hello,world",          "Hello, world");
    check("hello, world",         "Hello, world");
    check("a,b;c",                "A, b; c");
    check("pay 1,000 now",        "Pay 1,000 now");
    check("hi,there. ok,fine",    "Hi, there. Ok, fine");

    // --- Backspace ---
    check("ok.\b the end",        "Ok the end");   // deleted '.', no capital
    check("h\bh",                 "H");            // deleted first letter
    check("a,\bb",                "Ab");           // deleted ',', no space
    check("a,b\bc",               "A, c");         // auto space kept, not doubled
    check("a, \bb",               "A, b");         // deleted the space, re-added
    check("\bhello",              "Hello");        // Backspace with no history
    check("ab\b\bc",              "C");            // several Backspaces

    // --- Abbreviations ---
    check("mr. smith",            "Mr. smith");                // not a sentence end
    check("see dr. jones. he left", "See dr. jones. He left"); // real end still works
    check("mrs\b. smith",         "Mr. smith");    // Backspace restores the word

    // --- Decimal numbers ---
    check("version 3.5 is out",   "Version 3.5 is out");       // not a sentence end
    check("pi is 3.14. ok",       "Pi is 3.14. Ok");           // decimal, then real end
    check("i have 3. then 4",     "I have 3. Then 4");         // "3." really ends it
    check("3.5 apples",           "3.5 apples");               // starts with a number
    check("1,000 items",          "1,000 items");              // digits start a sentence
    check("x 3.5\b\b. y",         "X 3. Y");       // Backspace restores decimal state

    // --- Enter (a new line starts a new sentence) ---
    check("hello\nworld",         "Hello\nWorld");
    check("hi,\nthere",           "Hi,\nThere");
    check("a.\nb",                "A.\nB");
    check("3.5\nx",               "3.5\nX");       // after a decimal number
    check("dr.\nsmith",           "Dr.\nSmith");   // after an abbreviation
    check("ok\n\b next",          "Ok next");      // Backspace undoes the Enter

    // --- Domains, files, emails: a . ! ? needs a space after it to end a sentence ---
    check("visit example.com now",  "Visit example.com now");
    check("open main.cpp",          "Open main.cpp");
    check("mail john@example.com. thanks", "Mail john@example.com. Thanks");
    check("search?q=cats",          "Search?q=cats");
    check("wait...what",            "Wait...what");

    // --- Links: no space is added after a comma inside one ---
    check("see https://a.com/x,y now", "See https://a.com/x,y now");
    check("go to www.a.com, then,fine", "Go to www.a.com, then, fine"); // ends at the space
    check("see http://\b\b,b",      "See http:, b"); // Backspace undoes link detection

    // --- Backspace steps back one character on screen, including inserted spaces ---
    check("a,b\b\bc",               "A, c");         // deleted the auto space too

    // --- A word stuck to ! ? . gets its forgotten space ---
    check("hello!world now",       "Hello! World now");
    check("really?yes sir",        "Really? Yes sir");
    check("i went home.Then i slept.", "I went home. Then i slept.");
    check("mr.Smith left",         "Mr. Smith left");
    check("see dr.Jones now",      "See dr. Jones now");
    check("J.K.Rowling wrote",     "J.K. Rowling wrote");
    check("hi!there\nnext",        "Hi! There\nNext");  // Enter ends the word too
    check("what?!no way",          "What?! No way");

    // --- ...but not for domains, files, links, emails, numbers, code-like words ---
    check("hello.world now",       "Hello. world now");
    check("open Notes.TXT now",    "Open Notes.TXT now");  // file extension
    check("see Node.JS docs",      "See Node.JS docs");
    check("page?id=5 ok",          "Page?id=5 ok");
    check("mail john@example.Com now", "Mail john@example.Com now");
    check("version 3.Beta now",    "Version 3.Beta now");
    check("open .Gitignore now",   "Open .Gitignore now");
    check("see www.Site.Com now",  "See www.Site.Com now");

    // --- Backspace after a forgotten space was fixed ---
    check("hi!there \b\b\b\b\b\b\bx", "Hi! X");        // all the way back to the "!"
    check("hi!there \b\bs now",   "Hi! Thers now");     // no second space added

    // --- Intelligent . ! ? Spacing (Next Feature Specification) ---
    // Sentence spacing
    check("hello.world",           "Hello. world");
    check("hello!world",           "Hello! World");
    check("hello?world",           "Hello? World");
    check("hello!how are you",     "Hello! How are you");
    check("really?yes",            "Really? Yes");

    // Domains
    check("example.com",           "example.com");
    check("example.org",           "example.org");
    check("google.co.in",          "google.co.in");
    check("github.com",            "github.com");
    check("openai.com",            "openai.com");
    check("example.dev",           "example.dev");

    // File names
    check("main.cpp",              "main.cpp");
    check("main.c",                "main.c");
    check("main.h",                "main.h");
    check("program.exe",           "program.exe");
    check("README.md",             "README.md");
    check("file.txt",              "file.txt");
    check("data.json",             "data.json");
    check("config.yaml",           "config.yaml");
    check("script.py",             "script.py");
    check("app.js",                "app.js");
    check("Open main.cpp",         "Open main.cpp");

    // Emails
    check("john@example.com",      "john@example.com");
    check("john.doe@example.com",  "john.doe@example.com");
    check("first.last@company.co.in", "first.last@company.co.in");
    check("Email me at john.doe@example.com", "Email me at john.doe@example.com");

    // Decimals
    check("3.5",                   "3.5");
    check("10.25",                 "10.25");

    // Versions
    check("v1.2.3",                "v1.2.3");
    check("1.0.0",                 "1.0.0");
    check("I am using v1.2.3",     "I am using v1.2.3");

    // URLs
    check("https://example.com",     "https://example.com");
    check("https://example.com/a,b", "https://example.com/a,b");
    check("www.example.com",         "www.example.com");

    // Mixed text
    check("Visit example.com. next sentence", "Visit example.com. Next sentence");

    if (failures == 0) {
        cout << "\nAll tests passed.\n";
        return 0;
    }
    cout << "\n" << failures << " test(s) failed.\n";
    return 1;
}