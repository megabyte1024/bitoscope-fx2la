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
#include <string>

#include "fx2_profiles.h"
#include "fx2_protocol.h"

namespace fx2la {

struct CapturePlan {
  bool wide = false;
  uint64_t rateHz = 0;
  StartCommand start{};
  uint32_t deviceChannelMask = 0;
  uint32_t strideBytes = 0;
};

std::expected<CapturePlan, std::string> planCapture(const Fx2Profile& profile, uint64_t rateMilliHz,
                                                    uint32_t channelMask);

}
