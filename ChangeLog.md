# Changelog

All notable changes to this project will be documented in this file. The format follows **Keep a Changelog**, and the versioning scheme is based on **SemVer**.

## [Unreleased]

## [0.1.3] - 2026-10-28

### Fixed
- Set the HEX-to-BIN conversion buffer to 64 KiB so data at addresses `0x8000` through `0xFFFF` does not hit makebin's default 32 KiB limit. Packed output is preserved.
- Set the linker code-size limit explicitly to 65536 bytes, matching the conversion buffer and memory report. See [#1](https://github.com/recepsenbas/DWIN-T5L-SDCC-Template/issues/1).
- Initialize `PAGESEL` to `0x01` and `D_PAGESEL` to `0x02` in the reset path before the SDCC runtime starts. This enables execution across the full 64 KiB code space and prevents a black screen when firmware grows beyond the `0x8000` boundary. The fix was validated on a DMG10600F101_01WTC panel with a 37 KiB application.

## [0.1.2] - 2025-12-07

### Fixed
- UART3 error resolved by disabling UART3 definitions in the configuration file (UART3 is not used in DWIN modules).

### Improved
- Enhanced register write functions.


## [0.1.1] - 2025-10-21

### Fixed
- Incorrect LEN byte and CRC handling for `0x83` frames (previously `5A A5 06 83 10 08 01 00 02 28 45` was produced, now corrected to `5A A5 08 83 10 08 01 00 01 4C 4B`).
- Resolved double-CRC issue and aligned output with the official DGUS communication standard.
- Fixed packet structure which is still incorrect in DWIN’s original template.


### Notes
- CRC-OFF behavior is unchanged.
 
---

> This is the first release with a formal changelog. Earlier project history may not be listed retroactively.
