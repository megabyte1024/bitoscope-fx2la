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

#include "fx2_usb_device.h"

#include <libusb.h>

#include <array>
#include <format>
#include <utility>

#include "busy_core_keeper.h"

namespace fx2la {

namespace {

namespace plugin = bitoscope::plugin;

constexpr uint8_t kVendorOut = 0x40;
constexpr uint8_t kVendorIn = 0xc0;
constexpr uint32_t kControlTimeoutMs = 100;

class UsbFx2Device final : public Fx2Device {
 public:
  explicit UsbFx2Device(plugin::UsbDevice usb) : usb_(std::move(usb)) {}

  std::expected<void, DeviceError> resetInterface() override {
    // WHY: Clearing halt did not stop captures after a fast one losing samples every 2048; set-interface did.
    const int code = libusb_set_interface_alt_setting(static_cast<libusb_device_handle*>(usb_.libusbHandle()),
                                                      kInterfaceNumber, 0);
    if (code == LIBUSB_ERROR_NO_DEVICE || code == LIBUSB_ERROR_NOT_FOUND) {
      return std::unexpected(DeviceError{std::format("FX2 was disconnected (USB: {})", libusb_error_name(code)), true});
    }
    if (code != 0) {
      return std::unexpected(DeviceError{std::format("selecting alt setting 0 failed: {}", libusb_error_name(code)), false});
    }
    return {};
  }

  std::expected<void, std::string> start(const StartCommand& command) override {
    StartCommand bytes = command;
    const auto sent = usb_.control({kVendorOut, kCommandStart, 0, 0}, bytes, kControlTimeoutMs);
    if (!sent) return std::unexpected(std::format("START command failed: {}", sent.error().text));
    if (*sent != bytes.size()) return std::unexpected(std::format("START command sent {} of {} bytes", *sent, bytes.size()));
    return {};
  }

  void streamSamples(plugin::Sink& sink, plugin::UsbStream stream) override {
    stream.device = &usb_;
    stream.endpoint = kSampleEndpoint;
    stream.rawIo = true;
#if defined(_WIN32)
    const BusyCoreKeeper busyCoreKeeper;
#endif
    plugin::streamUsb(sink, stream);
  }

  std::expected<std::vector<uint8_t>, std::string> read(uint8_t request, size_t bytes, std::string_view what) {
    std::vector<uint8_t> reply(bytes);
    const auto received = usb_.control({kVendorIn, request, 0, 0}, reply, kControlTimeoutMs);
    if (!received) return std::unexpected(std::format("{} request failed: {}", what, received.error().text));
    reply.resize(*received);
    return reply;
  }

 private:
  plugin::UsbDevice usb_;
};

plugin::DeviceFailure failure(std::string detail) {
  return {plugin::DeviceProblem::other, std::move(detail)};
}

}

std::expected<std::unique_ptr<Fx2Device>, plugin::DeviceFailure> openFx2Device(const plugin::FoundDevice& found) {
  auto usb = plugin::UsbDevice::open(found);
  if (!usb) return std::unexpected(std::move(usb.error()));
  auto device = std::make_unique<UsbFx2Device>(std::move(*usb));
  constexpr const char* kHint = "; device may still run its stock firmware";
  const auto versionReply = device->read(kCommandGetFirmwareVersion, 2, "firmware version");
  if (!versionReply) return std::unexpected(failure(versionReply.error() + kHint));
  const auto version = parseFirmwareVersion(*versionReply);
  if (!version) return std::unexpected(failure(version.error()));
  if (const auto accepted = checkFirmwareVersion(*version); !accepted) return std::unexpected(failure(accepted.error()));
  const auto revision = device->read(kCommandGetRevisionId, 1, "revision id");
  if (!revision) return std::unexpected(failure(revision.error()));
  if (revision->size() != 1) return std::unexpected(failure("revision id reply is empty"));
  bitoscope_log(BITOSCOPE_LOG_INFO, "opened device: firmware %u.%u, chip %.*s", unsigned{version->major},
                unsigned{version->minor}, static_cast<int>(chipName((*revision)[0]).size()),
                chipName((*revision)[0]).data());
  return device;
}

}
