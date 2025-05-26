# Contributing to LockSys

Thank you for considering a contribution to LockSys! This project is still in early development, and help is appreciated.

---

## What You Can Contribute

- **Bug reports** – Unexpected behavior, edge cases, or security concerns
- **HAL ports** – Support for new platforms or refactor existing ones
- **Tests** – Unit tests or integration tests
- **Docs** – Clarifying build instructions or platform requirements
- **Examples** – Simple demos for new users

---

## Getting Started

1. Fork the repo and clone your fork:
   ```bash
   git clone https://github.com/knowthebird/locksys
   cd locksys
   ```

2. Install build tools (CMake, a C compiler, optional clang tools).

3. Build a platform example (POSIX or Windows).

4. Make your changes on a feature branch:
   ```bash
   git checkout -b feature/new-hal-support
   ```

5. Format your code using `clang-format`:
   ```bash
   make format
   ```

6. Submit a pull request! Please include:
   - What the change does
   - Why it's useful
   - Any known limitations or edge cases

---

## Guidelines

- Avoid adding dependencies unless absolutely necessary
- Target C11 and keep portability in mind
- Platform-specific code goes in `src/hal/`
- Shared logic belongs in `src/global/` or `src/logging/`
- Do not commit `build/` output or `device_key.generated.h`

---

## Questions or Security Issues?

Open an issue, or contact Ross directly at **knowthebird@gmail.com** for sensitive security concerns.

---

Thanks again for helping build a better embedded security library!
