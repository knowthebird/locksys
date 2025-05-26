# 🔐 LockSys Threat Model

This document outlines key threats, assumptions, and mitigations for the LockSys access
control system. It is intended to guide development and integration in secure environments,
especially where physical access control is critical.

---

## 📌 Overview

**LockSys** is a modular, platform-agnostic access control framework designed for embedded
and general-purpose systems. It is optimized for microcontrollers but portable to Linux,
Windows, and platforms like Arduino and Raspberry Pi.

Primary use cases include physical locks for:
- Safes
- Lockers
- Vending machines
- ATMs
- Vaults
- Automotive systems
- Physical access doors

LockSys uses cryptographic primitives (e.g., HMAC) to secure PIN data and potentially
prevent replay attacks, but it is not a general-purpose encryption or secure storage library.

---

## 🧱 Architecture

- LockSys exposes a small public API through `locksys.h`.
- Users implement a platform-specific HAL to connect LockSys to their hardware.
- Integrators are expected to wrap LockSys with higher-level UX or application code.
- LockSys does not assume responsibility for the physical integrity of the signal path to
  the actuator; if the unlock signal is bypassed on the PCB, the system is compromised.

---

## 🚀 Deployment Assumptions

- LockSys runs on physical hardware (MCUs or embedded computers).
- If an attacker can access the PCB, they are assumed to have access to the unlock line.
- Encryption of user data protects PIN confidentiality in the case of theft or data dumping.
- Replay protection and secure transport may be required in some input configurations.
- Dumping firmware, RAM, or EEPROM is considered a physical attack and out of scope,
  though input validation is used to resist logic-based tampering.

---

## 🛡️ Assets and Controls

### Protected Assets

| Asset               | Description                                                 | Sensitivity                     |
|---------------------|-------------------------------------------------------------|---------------------------------|
| User PIN Hash       | HMAC of user PIN, stored in non-volatile memory             | Confidentiality                 |
| Device Key          | Secret used to HMAC PINs or authenticate messages           | Confidentiality                 |
| Unlock Signal       | Physical GPIO that actuates bolt or latch                   | Integrity                       |
| Lock State          | Internal system state (LOCKED, UNLOCKED, ALARMED, etc.)     | Integrity                       |
| Retry/Lockout Data  | Tracks attempt counters, timestamps                         | Integrity                       |
| Log Records         | Stored audit trail of activity, HMAC-authenticated          | Integrity ✅, Confidentiality ⚠️ |
| Firmware            | Executable logic and configuration                          | Integrity                       |
| HAL Implementation  | Hardware integration code, potentially security-critical    | Integrity                       |
| Input Source Data   | User-entered PINs or remote tokens                          | Integrity                       |

---

### Current Security Controls

| Control                          | Description                                                                 |
|----------------------------------|-----------------------------------------------------------------------------|
| HMAC-based PIN storage           | PINs are secured using a keyed hash algorithm (HMAC)                        |
| Secure memory wipe               | Vetted library clears PINs and keys from RAM after use                     |
| Lockout logic                    | Throttles access after multiple failed attempts                            |
| Exponential backoff (planned)   | Increases lockout duration per user after repeated failures                 |
| HAL abstraction                  | Interfaces must be implemented per system, with clean separation           |
| Constant-time comparisons        | Defends against timing-based PIN verification attacks                      |
| HMAC logs                        | Audit records are protected from modification or forgery                   |
| Platform-aware key storage       | Uses secure APIs like DPAPI on desktop systems; clear key risk noted for MCUs |
| Input validation                 | Designed to prevent memory corruption or command injection                 |

---

## ⚠️ Threats and Mitigations

| ID  | Asset             | STRIDE Type      | Threat Description                                        | Mitigation                                         | Risk     | Notes                                         |
|-----|-------------------|------------------|------------------------------------------------------------|----------------------------------------------------|----------|-----------------------------------------------|
| T1  | Input Data (PIN)  | Spoofing         | Attacker injects data via keypad or serial                 | Input validation (planned)                         | Medium   | Input origin not authenticated                |
| T2  | Log Records       | Tampering        | Logs altered to hide activity                             | HMAC integrity; deletion still possible            | Low      | Deletion not prevented                        |
| T3  | Log Records       | Info Disclosure  | Sensitive metadata leaked through unencrypted logs         | Optional encryption (future)                       | Medium   | Not yet encrypted                             |
| T4  | Unlock Signal     | Privilege Escalation | GPIO bypass through hardware access                     | Physical attack assumed out of scope               | High     | Hardware access ≈ full compromise             |
| T5  | PIN Hash          | Info Disclosure  | Extracted from EEPROM or flash                            | HMAC, not reversible                               | Low      | Safe if key is secure                         |
| T6  | Firmware          | Tampering        | Malicious firmware disables security                      | Memory-safe code, input validation                 | Medium   | Physical firmware write out of scope          |
| T7  | PIN Retry Logic   | DoS              | Locks out valid users via brute-force                     | Lockout + exponential backoff                      | Medium   | Better UX and protection                      |
| T8  | HAL Code          | Tampering        | Insecure HAL bypasses core logic                          | Static analysis + unit testing                     | Medium   | High-impact integration point                 |
| T9  | Input Data        | Replay           | Valid signal replayed via keypad or token                 | Not yet implemented                                | High     | Known weakness                                |
| T10 | Device Key        | Disclosure       | Extracted via firmware/RAM dump                           | Platform-dependent: DPAPI, secure elements         | Medium   | Input validation helps; physical access needed|

---

## 🔭 Future Enhancements

- Optional encrypted log storage
- Authenticated remote input (HMAC nonce, timestamps)
- Replay protection middleware
- Support for secure boot or firmware signing
- Recommended HAL test suite + CI checks
- Configurable audit log retention and export

---

_Last updated: 2025-05-16_

Maintainer: Ross Kinard
