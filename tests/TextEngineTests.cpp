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
    check("ok.\b the end",        "Ok the end"); // deleted '.', no capital
    check("h\bh",                 "H"); // deleted first letter
    check("a,\bb",                "Ab");  // deleted ',', no space
    check("a,b\bc",               "A, c");  // auto space kept, not doubled
    check("a, \bb",               "A, b");// deleted the space, re-added
    check("\bhello",              "Hello");//Backspace with no history
    check("ab\b\bc",              "C"); // several Backspaces

    if (failures == 0) {
        cout << "\nAll tests passed.\n";
        return 0;
    }
    cout << "\n" << failures << " test(s) failed.\n";
    return 1;
}