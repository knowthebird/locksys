# LockSys — Embedded Access-Control Library

LockSys is an experimental C library for local PIN/passphrase access control on embedded systems. It explores a portable architecture for authenticated user records, password verification, retry/lockout policy, persistent state, and hardware abstraction across MCU and desktop targets.

> **Status: developer alpha.** LockSys is not production-ready and has not undergone the testing, review, or certification required for deployment as a security boundary.

## Engineering Goals

The project is designed around several embedded-systems constraints:

- limited local resources and offline operation
- portable storage/time/input interfaces through a HAL
- authenticated persistent user records
- HMAC-based password verification
- constant-time comparison of authentication values
- configurable password and retry/lockout policy
- support for desktop targets during development and MCU targets for integration

See [`doc/threat_model.md`](doc/threat_model.md) for the current threat model, trust boundaries, and known limitations.

## Current Capabilities

| Capability | Status | Notes |
| --- | --- | --- |
| HMAC-based password verification | Implemented | Uses the configured crypto backend and internal device key |
| Authenticated user records | Implemented | User records include an HMAC checked when records are loaded |
| Constant-time authentication comparison | Implemented | Used when validating authentication values |
| Password policy validation | Implemented | Configurable limits and requirements |
| Retry / lockout policy | Implemented | Persistent user attempt state is supported |
| Platform HAL | Implemented | Storage/time/platform behavior is separated behind interfaces |
| Arduino examples | Implemented | Bootstrap and normal-operation example sketches are included |
| Desktop development targets | Implemented | CMake supports POSIX/Windows development paths |
| Authenticated audit logs | Planned | Do not treat current logging as tamper-evident |
| Remote authenticated input / replay protection | Not implemented | Must be supplied by the integration if needed |
| Secure boot / firmware signing | Out of scope | Platform/integration responsibility |
| Security certification | None | No FIPS, Common Criteria, or similar certification |

## Repository Structure

```text
CMakeLists.txt        Build configuration
examples/             Desktop and Arduino example applications
src/                  Core implementation
src/crypto/           Cryptographic abstraction/backends
src/global/           User records, policy, configuration, and system logic
src/hal/              Platform-specific storage/time/I/O abstraction
src/logging/          Logging-related interfaces/work in progress
tests/                Early test harness
tools/                Bootstrap/key-generation utilities
doc/                  Security and design documentation
```

## Desktop Build

### Requirements

- CMake 3.16+
- GCC/Clang on POSIX platforms or MinGW/Visual Studio on Windows

### POSIX / Linux / macOS

```bash
mkdir build
cd build
cmake -DCRYPTO_BACKEND_TINYCRYPT=ON ..
cmake --build . --target bootstrap_posix
./bin/bootstrap
cmake --build . --target main_posix
./bin/main
```

### Windows with MinGW

```cmd
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCRYPTO_BACKEND_TINYCRYPT=ON ..
cmake --build . --target bootstrap_win
bin\bootstrap.exe
cmake --build . --target main_win
bin\main.exe
```

The bootstrap application initializes persistent state and the first administrative user. The normal application uses the same device key and persistent storage format.

## Arduino Workflow

The Arduino examples can be used without CMake:

1. Upload `examples/arduino/BootstrapSystem/BootstrapSystem.ino`.
2. Initialize storage and the root administrative account.
3. Upload `examples/arduino/OpenLock/OpenLock.ino`.
4. Continue normal operation using the same device key and configuration.

## Development Device Key

The repository intentionally includes `src/global/device_key.generated.h` with a predictable example key so the project and Arduino examples can build without a separate provisioning step. The header emits a compiler warning stating that the key is unsafe for production.

That checked-in value is **not a real secret**. It is a development fixture.

For any real deployment:

- generate a unique random key per device
- keep the real key out of source control
- provision/store it using protection appropriate to the target platform
- do not reuse the repository's development key

The generated key and local build output should not be committed when using real provisioning data.

## Security Boundaries

LockSys does not solve every layer of physical access control. Important assumptions include:

- Direct PCB or actuator access may bypass application logic.
- Firmware extraction or modification can defeat software controls unless the platform provides additional protections.
- Remote-input authentication and replay resistance are integration concerns and are not currently provided by the core library.
- Secure audit logging is planned, not complete.
- The current automated test suite is early and does not yet justify a production-security claim.

The detailed rationale is in [`doc/threat_model.md`](doc/threat_model.md).

## Testing and Static Analysis

The repository includes an early test harness plus `clang-format` and `clang-tidy` configuration. Expanding automated tests around authentication, record integrity, lockout behavior, malformed storage, and failure paths is an important next step before stronger security claims are appropriate.

## Intended Use

LockSys is most useful as:

- an embedded-security architecture experiment
- a portable HAL/design example for local access control
- a basis for testing authentication and persistence approaches on MCU and desktop targets
- a project for review, extension, and security analysis

It should not currently be used as an unreviewed production access-control component.

## Contributing

Feedback, tests, platform ports, and security review are welcome. See [`CONTRIBUTING.md`](CONTRIBUTING.md).

## License

MIT. See [`LICENSE.md`](LICENSE.md).
