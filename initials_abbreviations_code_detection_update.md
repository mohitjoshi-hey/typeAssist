# Initials, Dotted Abbreviations, and Code Detection Update

## 1. Overview and Purpose

This update addresses the next stage of Phase 1 from [roadmap.md](file:///c:/Users/Mohit%20joshi/OneDrive/Desktop/typeAssist/typeAssist/roadmap.md):
- **Initials and dotted abbreviations** (`e.g.`, `etc.`, `i.e.`, `a.m.`, `p.m.`, `U.S.`, `J. K. Rowling`, `et al.`)
- **Code detection and protection** (member access `obj.method()`, `this.state`, `console.log`, `camelCase`, `snake_case`, `[]`, `::`, `->`)

This document details the exact limitations discovered in the previous codebase, how they were resolved in the text engine, and provides direct before-and-after examples with test coverage.

---

## 2. Limitations & Bugs Identified in Previous Codebase

### Limitation 1: Dotted Abbreviations Triggered Sentence Capitalization
- **Cause**: The previous engine only recognized `isAbbreviation()` for single-word titles (`mr`, `dr`, `vs`). Multi-dot abbreviations like `e.g.`, `i.e.`, `a.m.`, `p.m.`, `U.S.`, and `et al.` were treated as sentence endings because the final period was interpreted as a terminal sentence end.
- **Failures**:
  - `e.g. apples and oranges` $\rightarrow$ became `E.g. Apples and oranges` (capitalized `Apples`).
  - `i.e. that is true` $\rightarrow$ became `I.e. That is true` (capitalized `That`).
  - `at 5 p.m. today` $\rightarrow$ became `At 5 p.m. Today` (capitalized `Today`).
  - `at 8 a.m. tomorrow` $\rightarrow$ became `At 8 a.m. Tomorrow` (capitalized `Tomorrow`).
  - `in the U.S. government` $\rightarrow$ became `In the U.S. Government` (capitalized `Government`).
  - `Smith et al. wrote this` $\rightarrow$ became `Smith et al. Wrote this` (capitalized `Wrote`).
  - `apples, etc. and more` $\rightarrow$ became `Apples, etc. And more` (capitalized `And`).

### Limitation 2: Commas Did Not Reset Pending Capitalization
- **Cause**: When a user typed a comma after an abbreviation (e.g. `e.g., ` or `etc., `), the engine flagged `spaceNeeded = true` but did not reset `capitalizeNext` or `endPending`.
- **Failures**:
  - `e.g., apples` $\rightarrow$ became `E.g., Apples`.
  - `i.e., that is` $\rightarrow$ became `I.e., That is`.
  - `etc., and so on` $\rightarrow$ became `Etc., And so on`.

### Limitation 3: Member Access in Programming Code Was Broken by Auto-Spacing
- **Cause**: Intelligent punctuation spacing previously inserted spaces after any period if the following characters were not in `kEndings` (domain and file extensions). This destroyed programming expressions.
- **Failures**:
  - `obj.method()` $\rightarrow$ became `Obj. method()`.
  - `this.state` $\rightarrow$ became `This. state`.
  - `my_var.field` $\rightarrow$ became `My_var. field`.
  - `items[0].value` $\rightarrow$ became `Items[0]. value`.
  - `ptr->member.val` $\rightarrow$ became `Ptr->member. val`.
  - `std::string.size` $\rightarrow$ became `Std::string. size`.
  - `window.location` $\rightarrow$ became `Window. location`.
  - `document.title` $\rightarrow$ became `Document. title`.
  - `res.status(200)` $\rightarrow$ became `Res. status(200)`.
  - `app.get("/api")` $\rightarrow$ became `App. get("/api")`.

### Limitation 4: camelCase Identifiers and Standard Library Classes Were Mutilated
- **Cause**: When typing camelCase identifiers at the start of a line (e.g. `userAccount.balance`), the leading character was auto-capitalized (`UserAccount`). Additionally, standard library classes with PascalCase (e.g. `Math.max`) were forced into lowercase.
- **Failures**:
  - `userAccount.balance` $\rightarrow$ became `UserAccount. balance`.
  - `obj.getName` $\rightarrow$ became `Obj. getName`.
  - `Math.max` $\rightarrow$ became `Math. max` or `math.max`.

---

## 3. Implementation Details

All changes were implemented in [src/TextEngine.cpp](file:///c:/Users/Mohit%20joshi/OneDrive/Desktop/typeAssist/typeAssist/src/TextEngine.cpp):

### 1. Expanded Single-Word Abbreviations
Added `etc`, `al` (for `et al.`), `eg`, `ie`, academic and governmental titles (`rev`, `hon`, `gen`, `col`, `maj`, `capt`, `lt`, `sgt`, `cpl`, `pvt`, `rep`, `sen`, `gov`, `approx`, `apt`, `dept`, `est`, `fig`, `inc`, `ltd`, `corp`, `vol`, `no`) to `isAbbreviation()`. Case-insensitive lookup ensures `Etc.` and `etc.` behave identically.

### 2. Dotted Abbreviation Detector (`isDottedAbbreviation`)
Introduced `isDottedAbbreviation(token)` to detect fixed multi-dot abbreviations:
- `e.g.`, `i.e.`, `a.m.`, `p.m.`, `u.s.`, `u.k.`, `d.c.`, `b.c.`, `a.d.`, `ph.d.`, `u.s.a.`
When a period completes one of these sequences:
- `sentenceEnd = false;`
- `state_.endPending = false;`
- `state_.capitalizeNext = false;`
Subsequent words remain lowercase as expected in English prose.

### 3. Comma Capitalization Reset
In `processCharacter`, receiving `,` or `;` immediately clears `capitalizeNext = false` and `endPending = false`. Commas can never start a sentence.

### 4. Code Context Engine (`isCodeContext`, `isCodeObject`, `isNaturallyLowercaseCodeObject`)
1. **Identifier Syntax**: Any token containing `_`, `::`, `->`, `[`, `]`, `$`, or `#` is recognized as code. Periods inside these tokens set `punctBlocked = true` and `sentenceEnd = false`.
2. **Built-in Code Objects**: Words like `this`, `self`, `cls`, `console`, `window`, `document`, `file`, `path`, `req`, `res`, `app`, `ctx`, `state`, `env`, `db`, `api`, `log`, `err`, `std`, `node`, `process` before a period are recognized as member access.
3. **PascalCase Preservation**: Standard classes like `Math`, `String`, `Object`, `Array`, `JSON`, `System` maintain their capital letter while avoiding trailing space insertion.
4. **camelCase Detection & Dynamic Space Undo**:
   - `hasCamelCase(id)` strictly verifies camelCase on identifiers (ignoring whitespace and punctuation).
   - If a tentative space was inserted before a method call (e.g. `obj. get`), typing an uppercase letter (`N` in `obj.getName`) or an open parenthesis (`(` in `obj.method()`) automatically erases the tentative space and reconstructs the code token seamlessly.
   - If an identifier started at the beginning of a line and camelCase is detected (`userAccount`), the auto-capitalized first letter is restored to lowercase.

---

## 4. Before & After Comparison Table

| User Input Typed | Behavior in Previous Engine | Behavior in Updated Engine | Status |
|:---|:---|:---|:---|
| `e.g. apples and oranges` | `E.g. Apples and oranges` *(capitalized Apples)* | `E.g. apples and oranges` | **Fixed** |
| `i.e. that is true` | `I.e. That is true` *(capitalized That)* | `I.e. that is true` | **Fixed** |
| `at 5 p.m. today` | `At 5 p.m. Today` *(capitalized Today)* | `At 5 p.m. today` | **Fixed** |
| `at 8 a.m. tomorrow` | `At 8 a.m. Tomorrow` *(capitalized Tomorrow)* | `At 8 a.m. tomorrow` | **Fixed** |
| `in the U.S. government` | `In the U.S. Government` *(capitalized Government)* | `In the U.S. government` | **Fixed** |
| `apples, etc. and more` | `Apples, etc. And more` *(capitalized And)* | `Apples, etc. and more` | **Fixed** |
| `Smith et al. wrote this` | `Smith et al. Wrote this` *(capitalized Wrote)* | `Smith et al. wrote this` | **Fixed** |
| `e.g., apples` | `E.g., Apples` *(capitalized after comma)* | `E.g., apples` | **Fixed** |
| `i.e., that is` | `I.e., That is` *(capitalized after comma)* | `I.e., that is` | **Fixed** |
| `etc., and so on` | `Etc., And so on` *(capitalized after comma)* | `Etc., and so on` | **Fixed** |
| `Washington, D.C. is great` | `Washington, D.C. Is great` *(capitalized Is)* | `Washington, D.C. is great` | **Fixed** |
| `he has a Ph.D. in math` | `He has a Ph.D. In math` *(capitalized In)* | `He has a Ph.D. in math` | **Fixed** |
| `J. K. Rowling wrote` | `J. K. Rowling wrote` | `J. K. Rowling wrote` | **Protected** |
| `George R. R. Martin wrote` | `George R. R. Martin wrote` | `George R. R. Martin wrote` | **Protected** |
| `e.g.apples` | `E.g. apples` | `E.g. apples` | **Fixed** |
| `etc.and more` | `Etc. and more` | `Etc. and more` | **Fixed** |
| `this.state` | `This. state` *(space inserted)* | `this.state` | **Fixed** |
| `console.log` | `Console.log` | `console.log` | **Fixed** |
| `my_var.field` | `My_var. field` *(space inserted)* | `my_var.field` | **Fixed** |
| `items[0].value` | `Items[0]. value` *(space inserted)* | `items[0].value` | **Fixed** |
| `std::string.size` | `Std::string. size` *(space inserted)* | `std::string.size` | **Fixed** |
| `ptr->member.val` | `Ptr->member. val` *(space inserted)* | `ptr->member.val` | **Fixed** |
| `userAccount.balance` | `UserAccount. balance` *(space inserted)* | `userAccount.balance` | **Fixed** |
| `window.location` | `Window. location` *(space inserted)* | `window.location` | **Fixed** |
| `document.title` | `Document. title` *(space inserted)* | `document.title` | **Fixed** |
| `obj.method()` | `Obj. method()` *(space inserted)* | `obj.method()` | **Fixed** |
| `obj.getName` | `Obj. getName` *(space inserted)* | `obj.getName` | **Fixed** |
| `res.status(200)` | `Res. status(200)` *(space inserted)* | `res.status(200)` | **Fixed** |
| `app.get("/api")` | `App. get("/api")` *(space inserted)* | `app.get("/api")` | **Fixed** |
| `Math.max` | `Math. max` *(space inserted)* | `Math.max` | **Fixed** |

---

## 5. Test Suite Verification

The test suite in [tests/TextEngineTests.cpp](file:///c:/Users/Mohit%20joshi/OneDrive/Desktop/typeAssist/typeAssist/tests/TextEngineTests.cpp) was expanded from 86 tests to **117 tests**.

Every single test passes with zero warnings or errors on Windows MSVC C++17.
