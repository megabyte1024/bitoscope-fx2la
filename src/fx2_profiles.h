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
#include <span>
#include <string>
#include <string_view>

namespace fx2la {

inline constexpr uint32_t kCapabilityWide16 = 1u << 0;
inline constexpr uint32_t kCapabilityAnalogAx = 1u << 1;

struct Fx2Profile {
  uint16_t vendorId;
  uint16_t productId;
  std::string_view vendor;
  std::string_view model;
  std::string_view firmwareFile;
  uint32_t capabilities;

  uint32_t logicChannels() const;
};

std::span<const Fx2Profile> fx2Profiles();

const Fx2Profile* findProfile(uint16_t vendorId, uint16_t productId);

std::string displayName(const Fx2Profile& profile);

}
