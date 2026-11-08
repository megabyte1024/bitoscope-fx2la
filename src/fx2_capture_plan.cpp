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

#include "fx2_capture_plan.h"

#include <bit>
#include <format>

#include "number_text.h"

namespace fx2la {

namespace {

constexpr uint32_t kHighChannelsMask = 0xff00;
constexpr uint32_t kNarrowDeviceMask = 0x00ff;
constexpr uint32_t kWideDeviceMask = 0xffff;
constexpr uint64_t kMilliHzPerHz = 1000;

}

std::expected<CapturePlan, std::string> planCapture(const Fx2Profile& profile, uint64_t rateMilliHz,
                                                    uint32_t channelMask) {
  if (channelMask == 0) return std::unexpected("no channel is selected");
  const uint32_t channels = profile.logicChannels();
  if (static_cast<uint32_t>(std::bit_width(channelMask)) > channels) {
    return std::unexpected(std::format("{} has {} channels: channel mask {:#x} goes beyond them", displayName(profile),
                                       channels, channelMask));
  }
  if (rateMilliHz % kMilliHzPerHz != 0) {
    return std::unexpected(std::format("sample rate {} mHz is not whole Hz", groupedNumber(rateMilliHz)));
  }
  CapturePlan plan;
  plan.rateHz = rateMilliHz / kMilliHzPerHz;
  plan.wide = (channelMask & kHighChannelsMask) != 0;
  if (plan.wide && plan.rateHz > kMaxWideRateHz) {
    return std::unexpected(std::format("channels 8-15 are captured: sample rate cannot be above 12 MHz, is {} Hz",
                                       groupedNumber(plan.rateHz)));
  }
  auto start = encodeStart(plan.rateHz, plan.wide);
  if (!start) return std::unexpected(std::move(start.error()));
  plan.start = *start;
  plan.deviceChannelMask = plan.wide ? kWideDeviceMask : kNarrowDeviceMask;
  plan.strideBytes = plan.wide ? 2 : 1;
  return plan;
}

}
