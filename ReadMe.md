<p align="center">
  <img src="https://img.shields.io/badge/Build-Stable-brightgreen?style=for-the-badge" alt="Build Status"/>
  <img src="https://img.shields.io/badge/Version-0.1.1-blue?style=for-the-badge" alt="Version"/>
  <img src="https://img.shields.io/badge/License-CC_BY--NC--SA_4.0-orange?style=for-the-badge" alt="License"/>
  <img src="https://img.shields.io/badge/Platform-macOS%20|%20Linux%20|%20Windows-lightgrey?style=for-the-badge" alt="Platform"/>
  <a href="https://buymeacoffee.com/recepsenbas" target="_blank" rel="noopener">
    <img src="https://img.shields.io/badge/Buy_Me_A_Coffee-F7CA88?logo=buymeacoffee&logoColor=black&style=for-the-badge" alt="Buy Me A Coffee"/>
  </a>
</p>

# T5L + SDCC Starter Template

> Open-source firmware template for DWIN T5L/T5L51 panels using SDCC — clean, portable, and Keil-free.

A production‑ready starter for building firmware on DWIN T5L/T5L51 controllers using the open‑source SDCC toolchain. It includes a clean project layout, startup code, portable libraries (UART, CRC, timer, RTC, SYS), and a reproducible Makefile that auto‑detects SDCC include/lib paths and prints a concise memory‑usage report after each build.

> Why this exists: DWIN officially recommends Keil C (uVision5) for firmware development, which is paid and runs only on Windows. This project provides a fully open-source alternative using SDCC that works on all major operating systems (macOS, Linux, and Windows). It also preserves full compatibility with the UART functions provided by DWIN’s original SDK, enabling developers to build and test firmware without Keil or any proprietary toolchain.

---

## 🎥 Video Tutorial

A complete video walkthrough of the setup and build process for this template is available on YouTube:

👉 [Watch on YouTube](https://www.youtube.com/watch?v=ZnZoBKaqA0A)

This video demonstrates SDCC and Make installation on macOS and Windows, project structure overview, and how to compile your code using `make`.

---

## Features
- Structured C project with clear separation of **startup**, **libs**, and **app** layers
- SDCC **large model** configuration with XRAM placed at `0x8000`
- Startup for T5L with ISR support
- Portable libraries: `uart`, `sys`, `crc16`, `timer`, `rtc`
- `Makefile` that:
  - auto‑detects SDCC mcs51 include and large‑model lib folders
  - builds `.rel` objects and links with map output
  - converts IHX → HEX → BIN (`T5L51.bin`)
  - prints a deterministic **memory usage** summary (CODE, XDATA, DATA, IDATA, BIT)
- Example application scaffold under `src/app`
- Provides a fully open DGUS core layer with RAM/NOR access, page control, and graph update routines — equivalent to vendor SDK, written from scratch.

---

## Repository Layout
```
.
├─ include/                # Shared headers (project‑wide)
├─ lib/
│  ├─ uart/                # UART driver (multi‑port ready)
│  ├─ sys/                 # System init, clock, low‑level utils
│  ├─ crc16/               # CRC utilities
│  ├─ timer/               # Timer helpers
│  └─ rtc/                 # RTC helpers
├─ src/
│  ├─ app/
│  │  ├─ app_defs/         # App‑specific definitions
│  │  └─ functions/        # App logic utilities
│  └─ main.c               # Entry point
├─ startup/
│  └─ startup_T5L.s        # Reset vector, ISRs, segments
├─ artifacts/            # Prebuilt binaries for quick flashing
│  └─ v0.1.1/            # Versioned folder (e.g., 9600/115200, CRC on/off)
├─ build/
│  ├─ obj/                 # Compiled objects (.rel)
│  └─ dist/                # Final artifacts (.ihx/.hex/.bin/.map)
├─ Makefile                # Build rules, memory summary
└─ ReadMe.md               # This file
```

---

## Prerequisites
- **SDCC** 4.5.0+ (mcs51 target)
- **GNU make**
- **sdas8051** (assembler packaged with SDCC)
- `packihx`, `makebin` (bundled with SDCC on most platforms)

### macOS (Homebrew)
```bash
brew install sdcc make
```
> The Makefile auto‑detects SDCC include/lib paths. If Homebrew layout changes, it also falls back to a standard path.

### Windows
#### Install SDCC
1. Install **SDCC** → https://sourceforge.net/projects/sdcc/

#### Install MSYS2
1. Install **MSYS2** → https://www.msys2.org/
2. Open *MSYS2 MinGW 64‑bit* shell and install:
   ```bash
    pacman -S make
   ```

3. Ensure `C:\msys64\usr\bin` and `C:\msys64\mingw64\bin` are in PATH.

### Linux (Debian/Ubuntu)
```bash
sudo apt-get update
sudo apt-get install sdcc make
```

---

## Building
### One‑line build
```bash
make
```
Artifacts go to `build/dist/`:
- `output.ihx`  (linked image with map)
- `output.hex`  (Intel HEX)
- `T5L51.bin`   (final binary)
- `output.map`  (for memory analysis)

### Clean
```bash
make clean
```

### Manual (reference)
```bash
sdcc -mmcs51 --model-large --xram-loc 0x8000 --xram-size 0x8000 \
-Isrc -Isrc/app -Isrc/app/app_defs -Iinclude -Istartup -Ilib/uart -Ilib/sys \
-Ilib/crc16 -Ilib/timer -Ilib/rtc -Isrc/app/functions \
-c src/main.c -o build/obj/main.rel
sdas8051 -plos build/obj/startup_T5L.rel startup/startup_T5L.s
# ... compile other libs ...
cd build/dist && sdcc -mmcs51 --model-large --xram-loc 0x8000 --xram-size 0x8000 -Wl-m -o output.ihx ../obj/*.rel
cd ../..
packihx build/dist/output.ihx > build/dist/output.hex
makebin -p build/dist/output.hex build/dist/T5L51.bin
```

---

## Configuration
Compiler and linker flags are centralized in the **Makefile**:
- Model: `--model-large`
- XRAM base/size: `--xram-loc 0x8000 --xram-size 0x8000`
- Include paths: `-Isrc -Iinclude -Istartup -Ilib/...`

Adjust these if your hardware variant changes the memory map.

---

## Flashing

DWIN panels accept two common flows:

1) **SD card (recommended, OS‑agnostic)**  
   Copy `T5L51.bin` to an SD card under `DWIN_SET/` (FAT32). Insert the card and power‑cycle the panel; the loader picks it up automatically.

2) **Serial download (Windows only)**  
   Use **DownLoadFor8051‑V1.4** to program via **Serial1** of the MCU. Not every model exposes Serial1 on a header; many reserve it for the on‑board Wi‑Fi footprint. If your model has that Wi‑Fi footprint, you can wire a USB‑UART there and program with DownLoadFor8051.

   *Important wiring note:* on some boards the Wi‑Fi pads are labeled from the MCU’s point of view. That means you should connect **TX→TX** and **RX→RX** on those pads (counter‑intuitive), plus common **GND**. Use **3.3V TTL** levels (not RS‑232). Always confirm your model’s schematic/manual.

This template only produces the **BIN**; choose either method above based on your hardware access.

## Prebuilt Binaries
Ready‑to‑flash `.bin` files built for common settings. Files are versioned under `artifacts/<version>/`.

**v0.1.1**  
`artifacts/v0.1.1/`
- [T5L51_9600_CrcOff_ResponseOff_noRTC.bin](./artifacts/v0.1.1/T5L51_9600_CrcOff_ResponseOff_noRTC.bin)
- [T5L51_9600_CrcOff_ResponseOn_noRTC.bin](./artifacts/v0.1.1/T5L51_9600_CrcOff_ResponseOn_noRTC.bin)
- [T5L51_9600_CrcOn_ResponseOn_noRTC.bin](./artifacts/v0.1.1/T5L51_9600_CrcOn_ResponseOn_noRTC.bin)
- [T5L51_115200_CrcOff_ResponseOff_noRTC.bin](./artifacts/v0.1.1/T5L51_115200_CrcOff_ResponseOff_noRTC.bin)
- [T5L51_115200_CrcOff_ResponseOn_noRTC.bin](./artifacts/v0.1.1/T5L51_115200_CrcOff_ResponseOn_noRTC.bin)
- [T5L51_115200_CrcON_ResponseOn_noRTC.bin](./artifacts/v0.1.1/T5L51_115200_CrcON_ResponseOn_noRTC.bin)

> **⚠️ Important:** After flashing, the panel will immediately operate with those settings and they persist across reboots. Ensure your MCU/host UART is set to the **same baud/CRC/response** before testing. If you lose communication, re‑flash a known variant (e.g., 9600/CRC‑Off/Response‑Off) or temporarily switch your host to the flashed settings.

---

## Debugging
- Use a serial console to validate `uart` output and ISRs.
- If you rely on `printf`-style logs, ensure stack/heap are sized appropriately in large model.
- Review `build/dist/output.map` to verify segment placement.

---

## Memory Usage Report
After `make`, a summary like below is printed from the linker map:
```
📊 Memory usage (build/dist/output.map):
  CODE      22299 / 65536 (34.0%)
  XDATA      5841 / 32768 (17.8%)
  DATA        123 / 256   (48.0%)
  IDATA         0 / 256   (0.0%)
  BIT           2 / 256   (0.8%)
```
If DATA+IDATA exceeds 256 bytes or CODE > 64KB, the Makefile emits warnings.

---

## Changelog

**1.0.0**  
Initial stable release. The entire codebase has been verified to compile cleanly and run reliably on multiple T5L/T5L51 devices using SDCC 4.5.0+. All core libraries (UART, SYS, CRC, TIMER, RTC) have been tested and validated under the large memory model configuration.

- Fully working startup and interrupt routines
- Verified UART communication and CRC integrity
- Stable and maintainable Makefile with automatic SDCC path detection
- Memory summary verified against linker output for accuracy

---

## Roadmap
- Add wrappers for common functions from the DWIN DGUS development guide directly as C functions within this template
- Introduce a modular extension layer for integrating new libraries (e.g., sensor communication, RS485 support)
- Improve example application coverage and documentation clarity
- Prepare a reference project demonstrating end-to-end DWIN HMI + SDCC firmware integration
- Expand compatibility tests across various DGUS panel models

---

### Community & Collaboration
This project welcomes contributions, feedback, and pull requests from the community. Developers are encouraged to enhance the codebase, share test results, or extend functionality. 

#### Pro Version
This repository represents the base open-source edition of the T5L SDCC template.
For the Pro version with extended DGUS function wrappers and advanced integration examples, please reach out me.

### Support & Donations
If this open-source work helps your project, you can support future improvements through voluntary contributions. 

<a href="https://buymeacoffee.com/recepsenbas" target="_blank" rel="noopener"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" height="40"></a>

---

## Troubleshooting
- **make: command not found (Windows):** install MSYS2 and run in the MinGW 64‑bit shell; ensure `make` is in PATH.
- **Missing SDCC libs:** the Makefile tries to auto‑detect; if it fails, set `SDCC_MCS51_INCLUDEDIR`/`SDCC_MCS51_LIBDIR` manually or adjust the fallback path in the Makefile.
- **Strange UART output:** verify XRAM base, stack location, and that your startup code matches the selected memory model.
- **Code size looks inflated:** macros and const data may land in code segments; inspect `output.map` to confirm segment attribution.
- - **No response from DGUS panel:** Check power-on sequence; DWIN panels require stable 5V and valid UART level (3.3V TTL).

---

## License
This project is licensed under the **CC BY-NC-SA 4.0** License.  
See the [LICENSE](./LICENSE) file for details.

---

## Acknowledgments
This template stands on the shoulders of the SDCC toolchain and the broader open‑source community around 8051 development.

---

## Contact & Support
- Issues: use the GitHub Issues tab with reproducible steps and your build log
- For commercial support or custom integrations (bootloaders, build pipelines, DGUS flows), reach out privately.

💬 Contact & Copyright

For collaboration, commercial inquiries, or advanced DGUS / embedded development support:


📧 **recepsenbas@gmail.com** • © 2025 **Recep Şenbaş**  
Released under the **CC BY-NC-SA 4.0** License — redistribution permitted with attribution and non-commercial intent.