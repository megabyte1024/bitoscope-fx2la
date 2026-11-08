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
#include "fx2_capture_plan.h"
#include "fx2_profiles.h"
#include "fx2_usb_device.h"

namespace fx2la {

bitoscope::plugin::LogicAnalyzerInfo fx2AnalyzerInfo(const Fx2Profile& profile);

class Fx2Analyzer final : public bitoscope::plugin::LogicAnalyzer {
 public:
  Fx2Analyzer(std::shared_ptr<Fx2Device> device, const Fx2Profile& profile);

  std::expected<void, std::string> prepare(const bitoscope::plugin::Capture& capture,
                                           bitoscope::plugin::Sink& sink) override;
  std::expected<void, std::string> capture(const bitoscope::plugin::Capture& capture,
                                           bitoscope::plugin::Sink& sink) override;

  std::expected<void, std::string> arm(const CapturePlan& plan);
  const bitoscope::plugin::UsbStream& stream() const { return stream_; }

 private:
  std::shared_ptr<Fx2Device> device_;
  const Fx2Profile& profile_;
  CapturePlan plan_;
  bitoscope::plugin::UsbStream stream_;
};

}
