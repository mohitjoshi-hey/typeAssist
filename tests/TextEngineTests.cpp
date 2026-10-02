#include <typeassist/TextEngine.h>

#include <iostream>
#include <string>

using namespace std;

// Types a string through a fresh engine and returns what the user would see.
static string simulateTyping(const string& input) {
    TextEngine engine;
    string output;

    for (char c : input) {
        KeyEvent event{c == ' ' ? KeyType::Space : KeyType::Character, c};
        TextAction action = engine.process(event);
        output += (action.type == ActionType::Replace) ? action.text
                                                       : string(1, c);
    }
    return output;
}

static int failures = 0;

static void check(const string& input, const string& expected) {
    const string actual = simulateTyping(input);
    if (actual == expected) {
        cout << "PASS: \"" << input << "\"\n";
    } else {
        ++failures;
        cout << "FAIL: \"" << input << "\"\n"
             << "  expected: \"" << expected << "\"\n"
             << "  actual:   \"" << actual << "\"\n";
    }
}

int main() {
    // Capitalization
    check("hello. world",         "Hello. World");
    check("hello! how are you?",  "Hello! How are you?");
    check("hello? yes.",          "Hello? Yes.");
    check("hello world",          "Hello world");
    check("Hello. World",         "Hello. World");
    check("hello... world",       "Hello... World");
    check("hi. ok. fine",         "Hi. Ok. Fine");

    // Punctuation spacing
    check("hello,world",          "Hello, world"); // space inserted
    check("hello, world",         "Hello, world");//no double space
    check("a,b;c",                "A, b; c");// comma and semicolon
    check("pay 1,000 now",        "Pay 1,000 now");// digits untouched
    check("hi,there. ok,fine",    "Hi, there. Ok, fine"); // both rules together

    if (failures == 0) {
        cout << "\nAll tests passed.\n";
        return 0;
    }
    cout << "\n" << failures << " test(s) failed.\n";
    return 1;
}