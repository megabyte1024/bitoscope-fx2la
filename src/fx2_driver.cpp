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

#include "fx2_driver.h"

#include <format>
#include <utility>

#include "fx2_analyzer.h"
#include "fx2_identify.h"

namespace fx2la {

namespace {

namespace plugin = bitoscope::plugin;

constexpr const char* kGenericName = "FX2 logic analyzer";

class Fx2Session final : public plugin::Session {
 public:
  Fx2Session(std::shared_ptr<Fx2Device> device, const Fx2Profile& profile, std::string name)
      : name_(std::move(name)), analyzer_(std::move(device), profile) {}
  ~Fx2Session() override { bitoscope_log(BITOSCOPE_LOG_INFO, "closing %s", name_.c_str()); }
  Fx2Session(const Fx2Session&) = delete;
  Fx2Session& operator=(const Fx2Session&) = delete;

  plugin::LogicAnalyzer* logicAnalyzer() override { return &analyzer_; }

 private:
  std::string name_;
  Fx2Analyzer analyzer_;
};

}

std::vector<plugin::UsbId> fx2UsbIds() {
  std::vector<plugin::UsbId> ids;
  for (const Fx2Profile& profile : fx2Profiles()) {
    ids.push_back(plugin::UsbId{profile.vendorId, profile.productId, kInterfaceNumber, kGenericName});
  }
  return ids;
}

Fx2Driver::Fx2Driver(Fx2Bus& bus, FirmwareFolders firmwareFolders, OpenFx2Device openDevice)
    : bus_(bus), firmwareFolders_(std::move(firmwareFolders)), openDevice_(std::move(openDevice)) {}

plugin::DriverDescription Fx2Driver::description() {
  plugin::DriverDescription description;
  description.displayName = std::string(kDriverDisplayName);
  description.version = std::string(kDriverVersion);
  description.transport = plugin::Transport::usb;
  description.finder = plugin::usbFinder(fx2UsbIds());
  description.identify = [this](const plugin::FoundDevice& found, plugin::Device& device) { return identify(found, device); };
  description.open = [this](const plugin::FoundDevice& found, plugin::Device&, plugin::Progress&) { return open(found); };
  return description;
}

std::expected<void, plugin::DeviceFailure> Fx2Driver::identify(const plugin::FoundDevice& found, plugin::Device& device) {
  const std::vector<std::filesystem::path> folders = firmwareFolders_();
  const auto profile = identifyFx2(bus_, found, folders);
  if (!profile) return std::unexpected(profile.error());
  (void)device.setName(displayName(**profile));
  if (auto added = device.add(fx2AnalyzerInfo(**profile)); !added) {
    return std::unexpected(plugin::DeviceFailure{plugin::DeviceProblem::other, std::move(added.error())});
  }
  return {};
}

std::expected<std::unique_ptr<plugin::Session>, plugin::DeviceFailure> Fx2Driver::open(const plugin::FoundDevice& found) {
  const auto profile = profileOf(found);
  if (!profile) return std::unexpected(profile.error());
  auto device = openDevice_(found);
  if (!device) return std::unexpected(std::move(device.error()));
  const std::string name = std::format("{} {}", displayName(**profile), found.id);
  bitoscope_log(BITOSCOPE_LOG_INFO, "connected to %s", name.c_str());
  return std::make_unique<Fx2Session>(std::shared_ptr<Fx2Device>(std::move(*device)), **profile, name);
}

}
