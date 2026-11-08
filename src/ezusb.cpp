/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is derived from the EZ-USB helpers of the libsigrok project.
 * Copyright (C) 2010-2012 Bert Vermeulen <bert@biot.com>
 * Copyright (C) 2026 megabyte1024
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ezusb.h"

#include <algorithm>
#include <array>
#include <format>
#include <string>
#include <string_view>

#include "number_text.h"

namespace fx2la {

namespace {

std::expected<void, std::string> step(EzusbPort& port, std::string_view name, uint16_t value,
                                      std::span<const uint8_t> data) {
  if (auto sent = port.vendorOut(value, data); !sent) return std::unexpected(std::format("{}: {}", name, sent.error()));
  return {};
}

}

std::expected<void, std::string> uploadEzusbFirmware(EzusbPort& port, std::span<const uint8_t> firmware) {
  if (firmware.empty()) return std::unexpected("firmware image is empty");
  if (firmware.size() > kEzusbMaxFirmwareBytes) {
    return std::unexpected(std::format("firmware image has {} bytes: EZ-USB takes at most {}", groupedNumber(firmware.size()),
                                       groupedNumber(kEzusbMaxFirmwareBytes)));
  }
  constexpr std::array<uint8_t, 1> kHold{1};
  constexpr std::array<uint8_t, 1> kRelease{0};
  if (auto held = step(port, "CPU reset", kEzusbCpuControlAddress, kHold); !held) return held;
  for (size_t offset = 0; offset < firmware.size(); offset += kEzusbChunkBytes) {
    const size_t bytes = std::min(firmware.size() - offset, kEzusbChunkBytes);
    if (auto sent = step(port, std::format("bytes at {}", offset), static_cast<uint16_t>(offset),
                         firmware.subspan(offset, bytes));
        !sent) {
      return sent;
    }
  }
  return step(port, "CPU release", kEzusbCpuControlAddress, kRelease);
}

}
