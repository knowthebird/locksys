# LockSys – Secure Access Control for Embedded Systems

**LockSys** is a lightweight, security-focused C library for local PIN/passphrase access control in embedded systems.

Built for hardware hackers, embedded devs, and security engineers, LockSys handles secure credential checks, lockout logic, and cryptographic protection of persistent data. Whether you're building a DIY safe or a hardened kiosk, LockSys is meant to drop in and stay out of your way.

---

## 🚧 Project Status: Developer Alpha

> ⚠️ LockSys is an early-stage security project. Not production-ready.  
> Some features are placeholders or partially implemented.
> Use at your own risk. Contributions, feedback, and testing are very welcome.

## Project Vision

LockSys aims to become a COTS-ready access control library:

    Low-resource for embedded systems

    High-security by default

    Reliable across hardware platforms

    Free and open source for real-world use, inspection, and verification

I’d love support from the community to help make LockSys a trustworthy foundation for physical access systems—whether for DIY safes or production kiosks.

---

## Features
- Secure **device-unique HMAC-based PIN storage** (constant-time comparison)
- **Explicit memory zeroization** of secrets
- **Lockout and disable logic** with persistent attempt tracking
- **Portable HAL** (hardware abstraction layer) for platform support
- Compatible with **EEPROM/Flash** for offline systems
- Builds cleanly on **MCUs, Linux, Windows, Arduino**, etc.

---

## Target Use Cases
- Physical access control for safes, lockers, and kiosks
- Embedded systems, Microcontroller-based, with limited resources and high security demands
- IoT enclosures without cloud dependencies
- Projects needing cryptographic protection without full disk encryption

---

## Build Instructions

This project supports both **desktop platforms** (Windows, Linux, macOS) and **Arduino microcontrollers**.

### Desktop Build (CMake)

#### Requirements
- CMake 3.16+
- A C compiler:
  - Windows: MinGW or Visual Studio
  - Linux/macOS: GCC or Clang

Optional:
- `clang-format` for formatting
- `clang-tidy` for linting

#### POSIX / Linux / macOS
```bash
mkdir build
```
```bash
cd build
```
```bash
cmake -DCRYPTO_BACKEND_TINYCRYPT=ON ..
```
```bash
cmake --build . --target bootstrap_posix
```
```bash
./bin/bootstrap
```
```bash
cmake --build . --target main_posix
```
```bash
./bin/main
```

#### Windows (MinGW)
```cmd
mkdir build
```
```cmd
cd build
```
```cmd
cmake -G "MinGW Makefiles" -DCRYPTO_BACKEND_TINYCRYPT=ON ..
```
```cmd
cmake --build . --target bootstrap_win
```
```cmd
bin\bootstrap.exe
```
```cmd
cmake --build . --target main_win
```
```cmd
bin\main.exe
```

#### What These Steps Do
1. **Build the bootstrap application** – initializes logs, storage, and user database.
2. **Run the bootstrap app** – sets up the initial root admin account.
3. **Build the main application** – the normal operational interface.
4. **The bootstrap app is no longer required** – but can reset the system if storage is retained.

#### Bootstrap and Device Key Binding
- The bootstrap and main applications share a **generated device key** stored in `device_key.generated.h`.
- If this key changes, existing storage becomes unusable.
- To recreate a valid root account, the bootstrap app must match the device key used by the main application.

---

### Arduino IDE

You can use the **Arduino IDE directly** — no CMake needed.

> A default `device_key.generated.h` is included for convenience. Users who are only working within the Arduino IDE (without a CLI or build tools) must manually edit this file to customize the device key for each unit.
>
> ⚠️ After making changes to the library (including `device_key.generated.h`), you must **restart the Arduino IDE** to ensure changes are applied correctly.

#### Arduino Workflow
1. Upload:
    ```
    examples/arduino/BootstrapSystem/BootstrapSystem.ino
    ```
2. Run the sketch to initialize device storage and root admin account.
3. Then upload:
    ```
    examples/arduino/OpenLock/OpenLock.ino
    ```
4. The system is now ready for normal use. The bootstrap firmware is no longer needed.

> The two Arduino programs must use the **same device key**. Customize configuration in:
```
src/global/config.h
```

---

## Project Structure
```
build/            → CMake output (binaries, storage, device key)
doc/              → Design notes and threat model
examples/         → Example applications (main.c, Arduino sketches)
src/              → Core implementation
src/crypto/       → Crypto interface
src/extern/       → Embedded libraries (mbedTLS, TinyCrypt)
src/global/       → Core config, user, and device key logic
src/hal/          → Platform-specific I/O (windows, posix, arduino)
src/logging/      → Logging interfaces (planned)
tests/            → Unit tests and mocks
tools/            → Bootstrap and key generation utilities
```

---

## Security Best Practices
- Regenerate the device key for each deployed device
- Never reuse the same device key across production units
- Do not commit `device_key.generated.h` or generated secrets to version control
- Securely erase provisioning files once the system is deployed:
```
src/global/device_key.generated.h
build/*
```

---

## Capability Matrix

| Capability                          | Supported | Notes                                      |
|-------------------------------------|-----------|--------------------------------------------|
| Constant-time HMAC PIN validation   | ✅         | SHA256 HMAC                                |
| Memory zeroization of secrets       | ✅         | Manual volatile overwrite                  |
| Lockout logic + retry throttling    | ✅         | Configurable thresholds                    |
| Platform HAL abstraction            | ✅         | Arduino, POSIX, Windows                    |
| Persistent lock state               | ✅         | EEPROM/Flash supported                     |
| Secure audit logs                   | ⚠️         | HMAC log storage planned                   |
| Arduino IDE support                 | ✅         | `BootstrapSystem.ino`, `OpenLock.ino`      |
| Networked auth / SSO / OAuth        | ❌         | Not supported, by design                   |
| Secure messaging / file crypto      | ❌         | Out of scope                               |
| HIPAA / FIPS 140-2 / compliance     | ❌         | No certs. Use at your own risk             |

---

## Contributing

We welcome:
- Feedback or security concerns (open an issue)
- Platform ports (new HALs)
- Example integrations or test cases

See [`CONTRIBUTING.md`](CONTRIBUTING.md) for details.

---

## License

MIT – Free for personal and commercial use.

---

_Developed by Ross Kinard – 2025_
