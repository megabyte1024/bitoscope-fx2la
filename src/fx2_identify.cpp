/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
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

#include "fx2_identify.h"

#include <format>

namespace fx2la {

namespace {

namespace plugin = bitoscope::plugin;

}

std::expected<const Fx2Profile*, plugin::DeviceFailure> profileOf(const plugin::FoundDevice& found) {
  const auto location = plugin::usbLocationOf(found.location);
  if (!location) {
    return std::unexpected(plugin::DeviceFailure{plugin::DeviceProblem::other, std::format("device {} has no USB location", found.id)});
  }
  const Fx2Profile* profile = findProfile(location->vendorId, location->productId);
  if (profile == nullptr) {
    return std::unexpected(plugin::DeviceFailure{
        plugin::DeviceProblem::notSupported,
        std::format("USB device {:04x}:{:04x} is no FX2 logic analyzer this driver knows", location->vendorId, location->productId)});
  }
  return profile;
}

std::expected<const Fx2Profile*, plugin::DeviceFailure> identifyFx2(
    Fx2Bus& bus, const plugin::FoundDevice& found, std::span<const std::filesystem::path> firmwareFolders) {
  const auto profile = profileOf(found);
  if (!profile) return profile;
  const auto location = plugin::usbLocationOf(found.location);
  const Fx2UsbPlace place{location->vendorId, location->productId, location->bus, location->address, location->ports};
  const auto state = attachDevice(bus, **profile, place, firmwareFolders);
  if (!state) return std::unexpected(state.error());
  if (*state == AttachState::firmwareLoaded) {
    return std::unexpected(plugin::DeviceFailure{plugin::DeviceProblem::firmwareLoading,
                                                 std::format("firmware loaded into {}; it enumerates again", displayName(**profile))});
  }
  return *profile;
}

}
