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

#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

#include "bitoscope_driver_cpp.h"
#include "fx2_profiles.h"

namespace fx2la {

struct Fx2UsbPlace {
  uint16_t vendorId = 0;
  uint16_t productId = 0;
  uint8_t bus = 0;
  uint8_t address = 0;
  std::vector<uint8_t> ports;
};

enum class BusProblem { busy, noPermission, gone, other };

struct BusError {
  BusProblem problem = BusProblem::other;
  std::string text;
};

struct UsbStrings {
  std::string manufacturer;
  std::string product;
};

class Fx2Bus {
 public:
  virtual ~Fx2Bus() = default;
  virtual std::expected<UsbStrings, BusError> readStrings(const Fx2UsbPlace& place) = 0;
  virtual std::expected<void, BusError> uploadFirmware(const Fx2UsbPlace& place,
                                                       std::span<const uint8_t> firmware) = 0;
};

enum class AttachState { ready, firmwareLoaded };

std::expected<AttachState, bitoscope::plugin::DeviceFailure> attachDevice(
    Fx2Bus& bus, const Fx2Profile& profile, const Fx2UsbPlace& place,
    std::span<const std::filesystem::path> firmwareFolders);

}
