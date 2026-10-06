#include <typeassist/TextEngine.h>

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
    check("hello. world",         "Hello. World");
    check("hello! how are you?",  "Hello! How are you?");
    check("hello? yes.",          "Hello? Yes.");
    check("hello world",          "Hello world");
    check("Hello. World",         "Hello. World");
    check("hello... world",       "Hello... World");
    check("hi. ok. fine",         "Hi. Ok. Fine");
    check("hello,world",          "Hello, world");
    check("hello, world",         "Hello, world");
    check("a,b;c",                "A, b; c");
    check("pay 1,000 now",        "Pay 1,000 now");
    check("hi,there. ok,fine",    "Hi, there. Ok, fine");
    check("ok.\b the end",        "Ok the end");
    check("h\bh",                 "H");
    check("a,\bb",                "Ab");
    check("a,b\bc",               "A, c");
    check("a, \bb",               "A, b");
    check("\bhello",              "Hello");
    check("ab\b\bc",              "C");
    check("mr. smith",            "Mr. smith");
    check("see dr. jones. he left", "See dr. jones. He left");
    check("mrs\b. smith",         "Mr. smith");
    check("version 3.5 is out",   "Version 3.5 is out");
    check("pi is 3.14. ok",       "Pi is 3.14. Ok");
    check("i have 3. then 4",     "I have 3. Then 4");
    check("3.5 apples",           "3.5 apples");
    check("1,000 items",          "1,000 items");
    check("x 3.5\b\b. y",         "X 3. Y");

    // --- Enter (a new line starts a new sentence) ---
    check("hello\nworld",         "Hello\nWorld");
    check("hi,\nthere",           "Hi,\nThere");
    check("a.\nb",               "A.\nB");
    check("3.5\nx",              "3.5\nX");
    check("dr.\nsmith",          "Dr.\nSmith");
    check("ok\n\b next",         "Ok next");

    if (failures == 0) {
        cout << "\nAll tests passed.\n";
        return 0;
    }
    cout << "\n" << failures << " test(s) failed.\n";
    return 1;
}