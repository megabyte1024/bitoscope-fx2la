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

#include "fx2_attach.h"

#include <format>
#include <string_view>

#include "firmware_file.h"
#include "fx2_protocol.h"

namespace fx2la {

namespace {

using bitoscope::plugin::DeviceFailure;
using bitoscope::plugin::DeviceProblem;

DeviceFailure failureOf(const BusError& error, std::string_view context) {
  DeviceProblem problem = DeviceProblem::other;
  if (error.problem == BusProblem::busy) problem = DeviceProblem::busy;
  if (error.problem == BusProblem::noPermission) problem = DeviceProblem::noPermission;
  return {problem, context.empty() ? error.text : std::format("{}: {}", context, error.text)};
}

}

std::expected<AttachState, DeviceFailure> attachDevice(Fx2Bus& bus, const Fx2Profile& profile, const Fx2UsbPlace& place,
                                                       std::span<const std::filesystem::path> firmwareFolders) {
  const auto strings = bus.readStrings(place);
  if (!strings) return std::unexpected(failureOf(strings.error(), ""));
  if (runsFx2lafwFirmware(strings->manufacturer, strings->product)) return AttachState::ready;
  const auto firmware = loadFirmware(profile.firmwareFile, firmwareFolders);
  if (!firmware) return std::unexpected(DeviceFailure{DeviceProblem::other, firmware.error()});
  if (const auto uploaded = bus.uploadFirmware(place, *firmware); !uploaded) {
    return std::unexpected(failureOf(uploaded.error(), std::format("firmware upload to {} failed", displayName(profile))));
  }
  return AttachState::firmwareLoaded;
}

}
