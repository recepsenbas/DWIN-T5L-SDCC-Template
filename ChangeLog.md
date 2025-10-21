# Changelog

All notable changes to this project will be documented in this file. The format follows **Keep a Changelog**, and the versioning scheme is based on **SemVer**.

## [Unreleased]

—

## [0.1.1] - 2025-10-21

### Fixed
- Incorrect LEN byte and CRC handling for `0x83` frames (previously `5A A5 06 83 10 08 01 00 02 28 45` was produced, now corrected to `5A A5 08 83 10 08 01 00 01 4C 4B`).
- Resolved double-CRC issue and aligned output with the official DGUS communication standard.
- Fixed packet structure which is still incorrect in DWIN’s original template.

### Notes
- CRC-OFF behavior is unchanged.
 
---

> This is the first release with a formal changelog. Earlier project history may not be listed retroactively.