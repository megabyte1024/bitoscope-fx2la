# FX2 logic analyzer driver for bitoscope

Driver for the logic analyzers built on the Cypress FX2 (CY7C68013) that run the sigrok project's
[fx2lafw](https://sigrok.org/wiki/Fx2lafw) firmware, for use with the bitoscope host. It talks to the
device through libusb itself; libsigrok is not needed.

## Features

- Logic analyzer: 8 or 16 channels (by model), 17 sample rates from 20 kHz to 48 MHz, streamed over USB.
- 16-bit samples (any of channels 8-15 captured) are limited to 12 MHz.
- No analog channel (CWAV USBee AX), no hardware trigger, no buffered capture.

### Supported devices

| Name | USB id (vendor:product) | Channels | Firmware file |
|---|---|---|---|
| CWAV USBee AX | `08a9:0014` | 8 | `fx2lafw-cwav-usbeeax.fw` |
| CWAV USBee DX | `08a9:0015` | 16 | `fx2lafw-cwav-usbeedx.fw` |
| CWAV USBee SX | `08a9:0009` | 8 | `fx2lafw-cwav-usbeesx.fw` |
| CWAV USBee ZX | `08a9:0005` | 8 | `fx2lafw-cwav-usbeezx.fw` |
| Saleae Logic and clones | `0925:3881` | 8 | `fx2lafw-saleae-logic.fw` |
| Cypress FX2 without EEPROM | `04b4:8613` | 16 | `fx2lafw-cypress-fx2.fw` |
| Braintechnology USB-LPS | `16d0:0498` | 16 | `fx2lafw-braintechnology-usb-lps.fw` |
| sigrok FX2 LA (8ch) | `1d50:608c` | 8 | `fx2lafw-sigrok-fx2-8ch.fw` |
| sigrok FX2 LA (16ch) | `1d50:608d` | 16 | `fx2lafw-sigrok-fx2-16ch.fw` |
| usb-c-grok | `1d50:608f` | 8 | `fx2lafw-usb-c-grok.fw` (not in firmware release 0.1.7) |

## Prerequisites

Linux and Windows; macOS is not supported yet.

- CMake 3.24 or newer
- A C++23 compiler: GCC 13 or newer on Linux, MSVC from Visual Studio 2022 (or its Build Tools, with
  the "Desktop development with C++" workload) on Windows
- Git, to fetch Cap'n Proto
- libusb-1.0

### Linux

Package names are Debian/Ubuntu ones:

```sh
sudo apt install build-essential cmake git pkg-config libusb-1.0-0-dev
```

The device's USB node must be readable for your user (udev rules, for example those of the
`libsigrok-dev` and `sigrok-firmware-fx2lafw` packages).

### Windows

libusb from vcpkg, installed into `C:\vcpkg`; the device needs the WinUSB driver (Zadig, for example):

```bat
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
C:\vcpkg\vcpkg install libusb:x64-windows
```

## How to compile

### Linux

```sh
git clone https://github.com/megabyte1024/bitoscope-fx2la.git
cd bitoscope-fx2la
cmake -P cmake/FetchSigrokFirmware.cmake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel $(cmake -P cmake/BitoscopeBuildJobs.cmake)
```

### Windows

Command Prompt or PowerShell:

```bat
git clone https://github.com/megabyte1024/bitoscope-fx2la.git
cd bitoscope-fx2la
cmake -P cmake/FetchSigrokFirmware.cmake
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_CONFIGURATION_TYPES=Release
cmake --build build --config Release
```

`cmake -P cmake/FetchSigrokFirmware.cmake` downloads the fx2lafw firmware release 0.1.7 from sigrok.org, checks
its SHA-256 and puts it into `third_party/sigrok-firmware/`. The firmware is a separate work of the sigrok
project (GPL-2.0-or-later, with LGPL parts; its `COPYING` files come with it); it is not part of this repository.

## Install

bitoscope looks for drivers in the `drivers/` folder next to `bitoscope_gui`. Copy the whole content of the
driver folder (see Output) there, or let CMake do it by passing the folder that holds `bitoscope_gui` as the
prefix:

```sh
cmake --install build --prefix /path/to/bitoscope_gui_folder
```

On Windows add `--config Release`.

Firmware is looked up in the folder named by `SIGROK_FIRMWARE_DIR`, then in `sigrok-firmware/` beside the
driver, then, on Linux, in `$XDG_DATA_HOME/sigrok-firmware` or `~/.local/share/sigrok-firmware`, `/usr/local/share/sigrok-firmware` and
`/usr/share/sigrok-firmware`.

## Output

- `build/drivers/`: the driver folder (`build\drivers\Release\` on Windows)
  - `fx2la`: the driver executable (`fx2la.exe` on Windows)
  - `sigrok-firmware/`: the firmware, if fetched

## License

GPL-3.0-or-later, see `COPYING`; every such source file carries the GPL notice in its header. The driver's device code derives from libsigrok's `fx2lafw` driver
(GPL-3.0-or-later, copyright its authors, among them Bert Vermeulen and Joel Holdsworth). The bitoscope SDK
sources in `sdk/` are MIT.
