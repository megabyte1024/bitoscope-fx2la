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

#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string_view>
#include <vector>

#include "bitoscope_driver_usb_cpp.h"
#include "fx2_attach.h"
#include "fx2_usb_device.h"

namespace fx2la {

inline constexpr std::string_view kDriverVersion = "0.4.0";
inline constexpr std::string_view kDriverDisplayName = "FX2 logic analyzers";

std::vector<bitoscope::plugin::UsbId> fx2UsbIds();

using FirmwareFolders = std::function<std::vector<std::filesystem::path>()>;
using OpenFx2Device = std::function<std::expected<std::unique_ptr<Fx2Device>, bitoscope::plugin::DeviceFailure>(
    const bitoscope::plugin::FoundDevice&)>;

class Fx2Driver {
 public:
  Fx2Driver(Fx2Bus& bus, FirmwareFolders firmwareFolders, OpenFx2Device openDevice = openFx2Device);

  bitoscope::plugin::DriverDescription description();

  std::expected<void, bitoscope::plugin::DeviceFailure> identify(const bitoscope::plugin::FoundDevice& found,
                                                                 bitoscope::plugin::Device& device);
  std::expected<std::unique_ptr<bitoscope::plugin::Session>, bitoscope::plugin::DeviceFailure> open(
      const bitoscope::plugin::FoundDevice& found);

 private:
  Fx2Bus& bus_;
  FirmwareFolders firmwareFolders_;
  OpenFx2Device openDevice_;
};

}
