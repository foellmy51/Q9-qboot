# Q-Boot Development Mandates

This document outlines the architecture, style guide, and developer workflows for the **Q-Boot** bootloader and BIOS.

---

## 1. Architectural Principles

*   **Bare-metal Safety:** Keep memory usage minimal and deterministic. Avoid dynamic allocation (`malloc`/`free`) during boot stages. Use static, pre-allocated memory or stack-allocated buffers.
*   **Modular Stage Separation:** Keep Stage 1 (assembly bootstrap) strictly separated from Stage 2 (C-based BIOS services).
*   **Target Agnostic Common Layer:** Any drivers or libraries under `src/lib/` or `src/setup/` should be target-agnostic and use hardware abstraction layers (HAL).
*   **Setup/NVRAM Reliability:** Reading/writing setup from I2C EEPROM or mass storage must include checksum (CRC32 or simple parity) verification before applying settings. If a checksum fails, fall back to hardcoded defaults immediately.

---

## 2. Code Style & Conventions

*   **Language:** Pure ANSI C (C89/C99 compatible) and target-specific Assembly (e.g., 68k).
*   **Naming Conventions:**
    *   Functions: Lowercase with underscores (e.g., `bios_put_char`, `eeprom_read_byte`).
    *   Variables: camelCase or lowercase with underscores.
    *   Constants & Macros: All uppercase with underscores (e.g., `BOOT_DELAY_DEFAULT`).
*   **Comments:** Use Javadoc-style headers for BIOS API functions to document input parameters, output registers, and side effects.

---

## 3. Development Workflow

*   **Adding Features:** Write clear unit or integration tests if applicable (in `test/` directory once set up).
*   **Verification:** Ensure no compiler warnings are present. Maintain high compatibility with the `Q9-QCC` compiler toolchain.
