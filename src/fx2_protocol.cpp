/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is derived from the fx2lafw driver of the libsigrok project.
 * Copyright (C) 2013 Bert Vermeulen <bert@biot.com>
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#include "fx2_protocol.h"

#include <array>
#include <format>

#include "number_text.h"

namespace fx2la {

namespace {

constexpr uint64_t kClock48MHz = 48'000'000;
constexpr uint64_t kClock30MHz = 30'000'000;
constexpr uint64_t kMaxSampleDelay = 6 * 256 - 1;
constexpr uint8_t kFlagWide = 1u << 5;
constexpr uint8_t kFlagClock48MHz = 1u << 6;
constexpr uint8_t kRevisionIdFx2lp = 1;

constexpr std::array<uint64_t, 17> kRatesHz{20'000,    25'000,    50'000,     100'000,    200'000,    250'000,
                                            500'000,   1'000'000, 2'000'000,  3'000'000,  4'000'000,  6'000'000,
                                            8'000'000, 12'000'000, 16'000'000, 24'000'000, 48'000'000};

}

std::span<const uint64_t> sampleRatesHz() { return kRatesHz; }

std::expected<StartCommand, std::string> encodeStart(uint64_t rateHz, bool wide) {
  if (rateHz == 0) return std::unexpected("sample rate 0 Hz is not possible");
  if (wide && rateHz > kMaxWideRateHz) {
    return std::unexpected(std::format("16-bit samples cannot be taken above 12 MHz, rate is {} Hz", groupedNumber(rateHz)));
  }
  struct Clock {
    uint64_t hz;
    uint8_t flag;
  };
  for (const Clock clock : {Clock{kClock48MHz, kFlagClock48MHz}, Clock{kClock30MHz, 0}}) {
    if (clock.hz % rateHz != 0) continue;
    const uint64_t delay = clock.hz / rateHz - 1;
    if (delay > kMaxSampleDelay) continue;
    const auto flags = static_cast<uint8_t>(clock.flag | (wide ? kFlagWide : 0));
    return StartCommand{flags, static_cast<uint8_t>((delay >> 8) & 0xff), static_cast<uint8_t>(delay & 0xff)};
  }
  return std::unexpected(std::format("device clocks cannot make sample rate {} Hz", groupedNumber(rateHz)));
}

std::expected<FirmwareVersion, std::string> parseFirmwareVersion(std::span<const uint8_t> reply) {
  constexpr size_t kReplyBytes = 2;
  if (reply.size() != kReplyBytes) {
    return std::unexpected(std::format("firmware version reply has {} bytes, expected {}", reply.size(), kReplyBytes));
  }
  return FirmwareVersion{reply[0], reply[1]};
}

std::expected<void, std::string> checkFirmwareVersion(FirmwareVersion version) {
  if (version.major != kRequiredFirmwareMajor) {
    return std::unexpected(std::format("expected firmware version {}.x, got {}.{}", kRequiredFirmwareMajor,
                                       version.major, version.minor));
  }
  return {};
}

bool runsFx2lafwFirmware(std::string_view manufacturer, std::string_view product) {
  return manufacturer == kFirmwareManufacturer && product == kFirmwareProduct;
}

std::string_view chipName(uint8_t revisionId) {
  return revisionId == kRevisionIdFx2lp ? "CY7C68013A (FX2LP)" : "CY7C68013 (FX2)";
}

}
