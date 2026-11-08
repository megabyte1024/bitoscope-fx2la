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

#include "fx2_analyzer.h"

#include <format>
#include <utility>

#include "fx2_protocol.h"

namespace fx2la {

namespace {

namespace plugin = bitoscope::plugin;

constexpr uint64_t kMilliHzPerHz = 1000;
constexpr uint64_t kDefaultRateHz = 1'000'000;
constexpr uint32_t kNarrowChannels = 8;
constexpr const char* kDeviceName = "FX2";

}

plugin::LogicAnalyzerInfo fx2AnalyzerInfo(const Fx2Profile& profile) {
  plugin::LogicAnalyzerInfo info;
  info.numChannels = profile.logicChannels();
  for (const uint64_t rate : sampleRatesHz()) info.sampleRatesMilliHz.push_back(rate * kMilliHzPerHz);
  info.defaultSampleRateMilliHz = kDefaultRateHz * kMilliHzPerHz;
  if (info.numChannels > kNarrowChannels) {
    const uint64_t highestRate = sampleRatesHz().back();
    for (uint32_t channels = 1; channels <= info.numChannels; ++channels) {
      info.rateCeilingsMilliHz.push_back((channels > kNarrowChannels ? kMaxWideRateHz : highestRate) * kMilliHzPerHz);
    }
    info.rateCeilingReason = "channels 8-15 are captured: FX2 takes 16-bit samples up to 12 MHz";
  }
  return info;
}

Fx2Analyzer::Fx2Analyzer(std::shared_ptr<Fx2Device> device, const Fx2Profile& profile)
    : device_(std::move(device)), profile_(profile) {}

std::expected<void, std::string> Fx2Analyzer::arm(const CapturePlan& plan) {
  plan_ = plan;
  stream_ = plugin::UsbStream{};
  stream_.deviceName = kDeviceName;
  stream_.drain = true;
  stream_.started = [this]() -> std::expected<void, std::string> {
    if (auto started = device_->start(plan_.start); !started) return started;
    bitoscope_log(BITOSCOPE_LOG_INFO, "capture started: %llu Hz, %u-bit samples",
                  static_cast<unsigned long long>(plan_.rateHz), plan_.strideBytes * 8);
    return {};
  };
  return {};
}

std::expected<void, std::string> Fx2Analyzer::prepare(const plugin::Capture& capture, plugin::Sink& sink) {
  if (capture.buffered) return std::unexpected(std::format("{} streams samples: buffered capture is not possible", displayName(profile_)));
  auto plan = planCapture(profile_, capture.sampleRateMilliHz, capture.channelMask);
  if (!plan) return std::unexpected(std::move(plan.error()));
  if (!sink.setDeviceBytes({plugin::Layout::sampleMajor, plan->deviceChannelMask, plan->strideBytes})) {
    return std::unexpected("SDK refused FX2's device bytes");
  }
  return arm(*plan);
}

std::expected<void, std::string> Fx2Analyzer::capture(const plugin::Capture&, plugin::Sink& sink) {
  if (auto reset = device_->resetInterface(); !reset) {
    if (!reset.error().gone) return std::unexpected(std::move(reset.error().text));
    sink.deviceGone(reset.error().text);
    return {};
  }
  device_->streamSamples(sink, stream_);
  bitoscope_log(BITOSCOPE_LOG_INFO, "capture stopped");
  return {};
}

}
