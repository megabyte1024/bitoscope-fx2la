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
#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace fx2la {

inline constexpr uint8_t kCommandGetFirmwareVersion = 0xb0;
inline constexpr uint8_t kCommandStart = 0xb1;
inline constexpr uint8_t kCommandGetRevisionId = 0xb2;
inline constexpr uint8_t kRequiredFirmwareMajor = 1;
inline constexpr uint8_t kSampleEndpoint = 0x82;
inline constexpr uint8_t kInterfaceNumber = 0;
inline constexpr uint8_t kConfigurationNumber = 1;
inline constexpr uint64_t kMaxWideRateHz = 12'000'000;
inline constexpr std::string_view kFirmwareManufacturer = "sigrok";
inline constexpr std::string_view kFirmwareProduct = "fx2lafw";

inline constexpr size_t kStartCommandBytes = 3;
using StartCommand = std::array<uint8_t, kStartCommandBytes>;

std::span<const uint64_t> sampleRatesHz();

std::expected<StartCommand, std::string> encodeStart(uint64_t rateHz, bool wide);

struct FirmwareVersion {
  uint8_t major = 0;
  uint8_t minor = 0;
  bool operator==(const FirmwareVersion&) const = default;
};

std::expected<FirmwareVersion, std::string> parseFirmwareVersion(std::span<const uint8_t> reply);

std::expected<void, std::string> checkFirmwareVersion(FirmwareVersion version);

bool runsFx2lafwFirmware(std::string_view manufacturer, std::string_view product);

std::string_view chipName(uint8_t revisionId);

}
