# LockSys Threat Model

This document describes the security assumptions, current controls, known limitations, and planned mitigations for LockSys. LockSys is an early-stage embedded access-control project and is **not production-ready**.

## Scope

LockSys is intended for local PIN/passphrase access control on embedded or general-purpose systems. The current implementation focuses on credential verification, authenticated user records, retry/lockout policy, persistent state, and platform abstraction.

The project does not attempt to provide a complete physical-security boundary. In particular, direct access to an actuator, PCB traces, firmware programming interfaces, RAM, flash, or other hardware may bypass software controls.

## Assets

| Asset | Security property |
| --- | --- |
| Password-derived HMAC values | Confidentiality |
| Device key | Confidentiality |
| User records | Integrity |
| Lockout / retry state | Integrity |
| System state | Integrity |
| Unlock signal | Integrity |
| Firmware and HAL implementation | Integrity |

## Current Controls

The repository currently implements or exposes the following controls:

- HMAC-based password verification and authenticated user records.
- Constant-time comparison for authentication values.
- Password-policy validation.
- Retry/lockout state and persistent user/system records.
- Platform abstraction through HAL interfaces.
- A clearly marked development-only default device key for example/debug builds.

These controls should not be interpreted as a security certification or as evidence that the system is ready for deployment.

## Known Limitations

| ID | Area | Limitation | Risk / consequence |
| --- | --- | --- | --- |
| T1 | Physical access | Direct hardware access is outside the software trust boundary. | An attacker with PCB/actuator access may bypass software controls. |
| T2 | Device key storage | MCU key protection depends on the target platform and integration. | Firmware/flash extraction can compromise the key. |
| T3 | Input authenticity | The core library does not authenticate a remote input channel. | Replay or injected input must be handled by the integrating system. |
| T4 | Logging | Secure authenticated audit logging is planned rather than complete. | Audit records should not yet be treated as tamper-evident. |
| T5 | Verification | Automated security-critical test coverage is currently limited. | Security claims require additional tests and review before deployment. |
| T6 | Secure boot / firmware authenticity | Not provided by LockSys. | A compromised firmware image can bypass application-level protections. |

## Development Key

`src/global/device_key.generated.h` contains an intentionally predictable development key so examples can build without a provisioning step. The file emits a compiler warning that it is unsafe for production.

A real deployment must generate and protect a unique key for each device. The checked-in development key must never be treated as a secret or reused in a deployed system.

## Planned Security Work

- Expand automated tests around authentication, record integrity, lockout behavior, and malformed storage/input cases.
- Add authenticated audit logging.
- Document platform-specific key provisioning and storage guidance.
- Add replay-resistant patterns for integrations that accept remote/authenticated inputs.
- Define recommended secure-boot / firmware-signing integration guidance where supported by the target platform.
- Review cryptographic and storage behavior before any production-use claim.

## Security Posture

LockSys should currently be evaluated as an **experimental security-oriented embedded library**. Its value is in the architecture, threat modeling, portable HAL design, and implementation work; it should not be used as a drop-in security boundary without independent review and additional validation.
