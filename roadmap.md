# typeAssist Roadmap

A system-wide typing assistant in C++.

```
KeyEvent -> TextEngine -> TextAction
```

The engine is independent of the OS and of AI. Windows and AI plug in around it.

**Current stage:** Phase 1 (core engine)
**Next step:** Undo support

Legend: `[x]` done, `[ ]` to do

---

## Phase 0: Setup and architecture

- [x] Decide architecture (Input layer -> Text Engine -> Action layer)
- [x] Create GitHub repo `typeAssist`
- [x] Write `CMakeLists.txt` (core library + demo executable)
- [x] Add placeholder `src/` files so the project builds
- [x] [PR #5](https://github.com/mohitjoshi-hey/typeAssist/pull/5) Add `README.md`
- [x] First commit: project builds successfully

## Phase 1: Core engine (no Windows, no AI)

Core types
- [x] `KeyEvent`, `KeyType` (declared in `TextEngine.h`)
- [x] `TextAction`, `ActionType` (declared in `TextEngine.h`)
- [x] `TextState` (what the engine remembers)
- [x] `TextEngine::process()` (capitalization, `,` `;` spacing, `. ! ?` intelligent spacing, Backspace, abbreviations, decimals, Enter, links, domains, file extensions, emails, versions, initials, dotted abbreviations, code detection, and cursor navigation done)

Rules, one at a time, each with tests
- [x] Capitalize first letter after `.` `!` `?`
- [x] Punctuation spacing for `,` and `;` (`hello,world` -> `hello, world`)
- [x] Punctuation spacing for `.` `!` `?` with token protection (`hello.world` -> `Hello. world`, `example.com` left untouched)
- [x] [PR #13](https://github.com/mohitjoshi-hey/typeAssist/pull/13) Backspace / editing behavior (state history, last 256 keys, replacement & auto-spacing preservation)
- [x] Unit tests (`tests/`), enabled in CMake

Edge cases to handle
- [x] [PR #1](https://github.com/mohitjoshi-hey/typeAssist/pull/1) Abbreviations (`Mr. smith`): `mr mrs ms dr prof sr jr st vs`
- [x] [PR #1](https://github.com/mohitjoshi-hey/typeAssist/pull/1) Decimal numbers (`3.14`) and sentences that start with a number
- [x] [PR #9](https://github.com/mohitjoshi-hey/typeAssist/pull/9) Multi-part numbers and versions (`3.5`, `10.25`, `1.0.0`, `v1.2.3`, `I am using v1.2.3`)
- [x] [PR #9](https://github.com/mohitjoshi-hey/typeAssist/pull/9) Intelligent punctuation spacing for `. ! ?` (`hello.world` -> `Hello. world`, `hello!world` -> `Hello! World`, `really?yes` -> `Really? Yes`)
- [x] [PR #9](https://github.com/mohitjoshi-hey/typeAssist/pull/9) Domain & file extension protection (`example.com`, `google.co.in`, `main.cpp`, `README.md`, `data.json`, `config.yaml` left untouched)
- [x] [PR #9](https://github.com/mohitjoshi-hey/typeAssist/pull/9) Email protection (`john@example.com`, `john.doe@example.com`, `first.last@company.co.in`)
- [x] [PR #7](https://github.com/mohitjoshi-hey/typeAssist/pull/7) [PR #9](https://github.com/mohitjoshi-hey/typeAssist/pull/9) URL protection (`https://example.com`, `https://example.com/a,b`, `www.example.com`)
- [x] [PR #9](https://github.com/mohitjoshi-hey/typeAssist/pull/9) Start-of-sentence token protection: uncapitalize leading character when token is recognized as domain, file, email, URL, or version (`example.com`, `main.cpp`)
- [x] [PR #11](https://github.com/mohitjoshi-hey/typeAssist/pull/11) Initials and dotted abbreviations (`J. K. Rowling`, `e.g.`, `etc.`, `i.e.`, `a.m.`, `p.m.`, `U.S.`, `et al.`)
- [x] [PR #2](https://github.com/mohitjoshi-hey/typeAssist/pull/2) Capitalize after Enter (new line)
- [x] [PR #11](https://github.com/mohitjoshi-hey/typeAssist/pull/11) Code (don't "correct" it: member access `obj.method()`, `this.state`, `console.log`, `camelCase`, `snake_case`, `[]`, `::`, `->`)
- [x] Cursor movement (`ArrowLeft`, `ArrowRight`, `ArrowUp`, `ArrowDown`, `Home`, `End` navigation with pending state cancellation and mid-text editing history)
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
- [ ] History state preservation across multi-character replacements on backspace
- [ ] Quote-aware and bracket-aware sentence capitalization (`"hello"` -> `"Hello"`, `(note)` -> `(Note)`)
- [ ] Multi-character ligature and arrow expansion (`->` to `→`, `=>` to `⇒`)

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
| 2026-10-19 | Initials and dotted abbreviations (`e.g.`, `i.e.`, `etc.`, `a.m.`, `p.m.`, `U.S.`, `J. K. Rowling`), comma capitalization reset, and code protection (`obj.prop`, `this.state`, `console.log`, `camelCase`, `snake_case`, `[]`, `::`, `->`); 117 tests passing |
| 2026-10-24 | Fix history state preservation on character replacement backspace (real-time spacing for `.`, `!`, `?` and token uncapitalization); 124 tests passing |
| 2026-10-28 | Cursor movement support (Arrow keys, Home, End navigation, cancellation of in-flight punctuation/spacing, and mid-text editing history); 133 tests passing |
