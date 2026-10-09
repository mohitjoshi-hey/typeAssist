# typeAssist Roadmap

A system-wide typing assistant in C++.

```
KeyEvent -> TextEngine -> TextAction
```

The engine is independent of the OS and of AI. Windows and AI plug in around it.

**Current stage:** Phase 1 (core engine)
**Next step:** Initials and dotted abbreviations (`e.g.`, `etc.`, `i.e.`) and code detection

Legend: `[x]` done, `[ ]` to do

---

## Phase 0: Setup and architecture

- [x] Decide architecture (Input layer -> Text Engine -> Action layer)
- [x] Create GitHub repo `typeAssist`
- [x] Write `CMakeLists.txt` (core library + demo executable)
- [x] Add placeholder `src/` files so the project builds
- [ ] Add `README.md`
- [x] First commit: project builds successfully

## Phase 1: Core engine (no Windows, no AI)

Core types
- [x] `KeyEvent`, `KeyType` (declared in `TextEngine.h`)
- [x] `TextAction`, `ActionType` (declared in `TextEngine.h`)
- [x] `TextState` (what the engine remembers)
- [x] `TextEngine::process()` (capitalization, `,` `;` spacing, `. ! ?` intelligent spacing, Backspace, abbreviations, decimals, Enter, links, domains, file extensions, emails, and versions done)

Rules, one at a time, each with tests
- [x] Capitalize first letter after `.` `!` `?`
- [x] Punctuation spacing for `,` and `;` (`hello,world` -> `hello, world`)
- [x] Punctuation spacing for `.` `!` `?` with token protection (`hello.world` -> `Hello. world`, `example.com` left untouched)
- [x] Backspace / editing behavior (state history, last 256 keys)
- [x] Unit tests (`tests/`), enabled in CMake

Edge cases to handle
- [x] Abbreviations (`Mr. smith`): `mr mrs ms dr prof sr jr st vs`
- [x] Decimal numbers (`3.14`) and sentences that start with a number
- [x] Multi-part numbers and versions (`3.5`, `10.25`, `1.0.0`, `v1.2.3`, `I am using v1.2.3`)
- [x] Intelligent punctuation spacing for `. ! ?` (`hello.world` -> `Hello. world`, `hello!world` -> `Hello! World`, `really?yes` -> `Really? Yes`)
- [x] Domain & file extension protection (`example.com`, `google.co.in`, `main.cpp`, `README.md`, `data.json`, `config.yaml` left untouched)
- [x] Email protection (`john@example.com`, `john.doe@example.com`, `first.last@company.co.in`)
- [x] URL protection (`https://example.com`, `https://example.com/a,b`, `www.example.com`)
- [x] Start-of-sentence token protection: uncapitalize leading character when token is recognized as domain, file, email, URL, or version (`example.com`, `main.cpp`)
- [ ] Initials and dotted abbreviations (`J. K. Rowling`, `e.g.`, `etc.`)
- [x] Capitalize after Enter (new line)
- [ ] Code (don't "correct" it)
- [ ] Cursor movement
- [ ] Undo

## Phase 2: Windows integration

- [ ] Windows keyboard hook -> `KeyEvent`
- [ ] Action executor (insert / replace / delete / pass-through)
- [ ] Test in Chrome, VS Code, Word, etc.

## Phase 3: AI layer

- [ ] Spelling detection (flag suspicious words)
- [ ] AI API call for corrections
- [ ] Replace misspelled word (`goign` -> `going`)
- [ ] Grammar suggestions

---

## Feature ideas (not committed)

- [ ] Per-app enable/disable
- [ ] Hotkey to pause the assistant
- [ ] User-configurable rules
- [ ] Custom dictionary
- [ ] Per-app Enter behavior: Enter = new line in chat apps (WhatsApp, ChatGPT),
      send with Shift+Enter or Ctrl+Enter (needs Phase 2: modifier keys in `KeyEvent`,
      foreground-app detection, key-swap action)

---

## Changelog

| Date | Update |
|------|--------|
| 2026-09-30 | Repo created, architecture decided, `CMakeLists.txt` written |
| 2026-10-01 | Project builds with MSVC + CMake; placeholder `TextEngine` committed and pushed (`ca8bd27`) |
| 2026-10-03 | Capitalization rule, `,` `;` spacing rule, and first 12 tests passing |
| 2026-10-04 | Backspace handling with `TextState` history; 19 tests passing |
| 2026-10-04 | Abbreviation and decimal rules; 28 tests passing |
| 2026-10-06 | Capitalize after Enter; 34 tests passing |
| 2026-10-06 | Sentence ends need a following space; link detection; 42 tests passing |
| 2026-10-10 | Intelligent `. ! ?` spacing with token protection (domains, files, emails, numbers, versions, URLs); 86 tests passing |