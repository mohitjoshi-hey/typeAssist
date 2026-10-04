#include <typeassist/TextEngine.h>

#include <iostream>
#include <string>

using namespace std;

static KeyType keyTypeFor(char c) {
    if (c == ' ') return KeyType::Space;
    if (c == '\b') return KeyType::Backspace;
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
            screen += action.text;
        } else {
            screen += c;
        }
    }
    return screen;
}

static int failures = 0;

// Shows Backspace as <BS> so the terminal doesn't act on it.
static string printable(const string& s) {
    string out;
    for (char c : s) {
        if (c == '\b') out += "<BS>";
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
             << "  expected: \"" << expected << "\"\n"
             << "  actual:   \"" << actual << "\"\n";
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
    check("mr. smith",            "Mr. smith");              // not a sentence end
    check("see dr. jones. he left", "See dr. jones. He left"); // real end still works
    check("mrs\b. smith",         "Mr. smith");    // Backspace restores the word

    // --- Decimal numbers ---
    check("version 3.5 is out",   "Version 3.5 is out");     // not a sentence end
    check("pi is 3.14. ok",       "Pi is 3.14. Ok");         // decimal, then real end
    check("i have 3. then 4",     "I have 3. Then 4");       // "3." really ends it
    check("3.5 apples",           "3.5 apples");             // starts with a number
    check("1,000 items",          "1,000 items");            // digits start a sentence
    check("x 3.5\b\b. y",         "X 3. Y");       // Backspace restores decimal state

    if (failures == 0) {
        cout << "\nAll tests passed.\n";
        return 0;
    }
    cout << "\n" << failures << " test(s) failed.\n";
    return 1;
}