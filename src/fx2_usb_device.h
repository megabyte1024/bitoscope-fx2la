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

#include <expected>
#include <memory>
#include <string>

#include "bitoscope_driver_usb_cpp.h"
#include "fx2_protocol.h"

namespace fx2la {

struct DeviceError {
  std::string text;
  bool gone = false;
};

class Fx2Device {
 public:
  virtual ~Fx2Device() = default;
  virtual std::expected<void, DeviceError> resetInterface() = 0;
  virtual std::expected<void, std::string> start(const StartCommand& command) = 0;
  virtual void streamSamples(bitoscope::plugin::Sink& sink, bitoscope::plugin::UsbStream stream) = 0;
};

std::expected<std::unique_ptr<Fx2Device>, bitoscope::plugin::DeviceFailure> openFx2Device(
    const bitoscope::plugin::FoundDevice& found);

}
