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
#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>

namespace fx2la {

inline constexpr uint8_t kEzusbRequest = 0xa0;
inline constexpr uint16_t kEzusbCpuControlAddress = 0xe600;
inline constexpr size_t kEzusbChunkBytes = 4096;
inline constexpr size_t kEzusbMaxFirmwareBytes = 1u << 16;

class EzusbPort {
 public:
  virtual ~EzusbPort() = default;
  virtual std::expected<void, std::string> vendorOut(uint16_t value, std::span<const uint8_t> data) = 0;
};

std::expected<void, std::string> uploadEzusbFirmware(EzusbPort& port, std::span<const uint8_t> firmware);

}
