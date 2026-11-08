/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * This file is derived from the fx2lafw driver and the EZ-USB helpers of the libsigrok project.
 * Copyright (C) 2013 Bert Vermeulen <bert@biot.com>
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2010-2012 Bert Vermeulen <bert@biot.com>
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

#include "fx2_libusb_bus.h"

#include <libusb.h>

#include <array>
#include <format>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "ezusb.h"
#include "fx2_protocol.h"

namespace fx2la {

namespace {

constexpr unsigned int kControlTimeoutMs = 100;
constexpr uint8_t kVendorOut = 0x40;
constexpr size_t kStringBytes = 64;
constexpr size_t kMaxPorts = 8;

BusError errorOf(std::string_view what, int code) {
  BusProblem problem = BusProblem::other;
  if (code == LIBUSB_ERROR_ACCESS) problem = BusProblem::noPermission;
  if (code == LIBUSB_ERROR_BUSY) problem = BusProblem::busy;
  if (code == LIBUSB_ERROR_NO_DEVICE || code == LIBUSB_ERROR_NOT_FOUND) problem = BusProblem::gone;
  const std::string name = libusb_error_name(code);
  if (code == LIBUSB_ERROR_ACCESS) return {problem, std::format("{}: no permission ({}; check udev rules)", what, name)};
  return {problem, std::format("{}: {}", what, name)};
}

class OpenedHandle {
 public:
  OpenedHandle() = default;
  explicit OpenedHandle(libusb_device_handle* handle) : handle_(handle) {}
  OpenedHandle(OpenedHandle&& other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}
  OpenedHandle(const OpenedHandle&) = delete;
  OpenedHandle& operator=(const OpenedHandle&) = delete;
  ~OpenedHandle() {
    if (handle_ != nullptr) libusb_close(handle_);
  }
  libusb_device_handle* get() const { return handle_; }

 private:
  libusb_device_handle* handle_ = nullptr;
};

class LibusbPort final : public EzusbPort {
 public:
  explicit LibusbPort(libusb_device_handle* handle) : handle_(handle) {}

  std::expected<void, std::string> vendorOut(uint16_t value, std::span<const uint8_t> data) override {
    std::vector<uint8_t> bytes(data.begin(), data.end());
    const int sent = libusb_control_transfer(handle_, kVendorOut, kEzusbRequest,
                                             value, 0, bytes.data(), static_cast<uint16_t>(bytes.size()),
                                             kControlTimeoutMs);
    if (sent < 0) return std::unexpected(libusb_error_name(sent));
    if (static_cast<size_t>(sent) != bytes.size()) {
      return std::unexpected(std::format("{} of {} bytes sent", sent, bytes.size()));
    }
    return {};
  }

 private:
  libusb_device_handle* handle_;
};

class LibusbBus final : public Fx2Bus {
 public:
  LibusbBus() { initResult_ = libusb_init(&context_); }

  ~LibusbBus() override {
    if (initResult_ == 0) libusb_exit(context_);
  }

  std::expected<UsbStrings, BusError> readStrings(const Fx2UsbPlace& place) override {
    std::lock_guard lock(mutex_);
    auto handle = openAt(place);
    if (!handle) return std::unexpected(handle.error());
    libusb_device_descriptor descriptor{};
    if (const int code = libusb_get_device_descriptor(libusb_get_device(handle->get()), &descriptor); code != 0) {
      return std::unexpected(errorOf("USB device descriptor", code));
    }
    UsbStrings strings;
    if (auto manufacturer = stringOf(*handle, descriptor.iManufacturer, "manufacturer string"); manufacturer) {
      strings.manufacturer = std::move(*manufacturer);
    } else {
      return std::unexpected(manufacturer.error());
    }
    if (auto product = stringOf(*handle, descriptor.iProduct, "product string"); product) {
      strings.product = std::move(*product);
    } else {
      return std::unexpected(product.error());
    }
    return strings;
  }

  std::expected<void, BusError> uploadFirmware(const Fx2UsbPlace& place, std::span<const uint8_t> firmware) override {
    std::lock_guard lock(mutex_);
    auto handle = openAt(place);
    if (!handle) return std::unexpected(handle.error());
    // WHY: libusb's Darwin backend reports an active kernel driver and then always fails to detach it.
#if !defined(__APPLE__)
    if (libusb_kernel_driver_active(handle->get(), kInterfaceNumber) == 1) {
      if (const int code = libusb_detach_kernel_driver(handle->get(), kInterfaceNumber); code < 0) {
        return std::unexpected(errorOf("detaching kernel driver", code));
      }
    }
#endif
    if (const int code = libusb_set_configuration(handle->get(), kConfigurationNumber); code < 0) {
      return std::unexpected(errorOf("setting USB configuration", code));
    }
    LibusbPort port(handle->get());
    if (auto uploaded = uploadEzusbFirmware(port, firmware); !uploaded) {
      return std::unexpected(BusError{BusProblem::other, std::move(uploaded.error())});
    }
    return {};
  }

 private:
  std::expected<OpenedHandle, BusError> openAt(const Fx2UsbPlace& place) {
    if (initResult_ != 0) return std::unexpected(errorOf("libusb initialisation", initResult_));
    libusb_device** list = nullptr;
    const ssize_t count = libusb_get_device_list(context_, &list);
    if (count < 0) return std::unexpected(errorOf("listing USB devices", static_cast<int>(count)));
    libusb_device* chosen = nullptr;
    for (ssize_t i = 0; i < count && chosen == nullptr; ++i) {
      if (libusb_get_bus_number(list[i]) != place.bus) continue;
      if (!place.ports.empty()) {
        std::array<uint8_t, kMaxPorts> ports{};
        const int length = libusb_get_port_numbers(list[i], ports.data(), static_cast<int>(ports.size()));
        if (length > 0 && std::vector<uint8_t>(ports.begin(), ports.begin() + length) == place.ports) chosen = list[i];
      } else if (libusb_get_device_address(list[i]) == place.address) {
        chosen = list[i];
      }
    }
    libusb_device_handle* handle = nullptr;
    int code = LIBUSB_ERROR_NO_DEVICE;
    if (chosen != nullptr) code = libusb_open(chosen, &handle);
    libusb_free_device_list(list, 1);
    if (code != 0) return std::unexpected(errorOf("opening USB device", code));
    OpenedHandle opened(handle);
    libusb_device_descriptor descriptor{};
    if (const int read = libusb_get_device_descriptor(libusb_get_device(handle), &descriptor); read != 0) {
      return std::unexpected(errorOf("USB device descriptor", read));
    }
    if (descriptor.idVendor != place.vendorId || descriptor.idProduct != place.productId) {
      return std::unexpected(BusError{BusProblem::gone, std::format("USB device at this place is {:04x}:{:04x}, not {:04x}:{:04x}", descriptor.idVendor, descriptor.idProduct, place.vendorId, place.productId)});
    }
    return opened;
  }

  static std::expected<std::string, BusError> stringOf(const OpenedHandle& handle, uint8_t index, std::string_view what) {
    if (index == 0) return std::string();
    std::array<unsigned char, kStringBytes> text{};
    const int length = libusb_get_string_descriptor_ascii(handle.get(), index, text.data(), static_cast<int>(text.size()));
    if (length < 0) return std::unexpected(errorOf(what, length));
    return std::string(reinterpret_cast<const char*>(text.data()), static_cast<size_t>(length));
  }

  std::mutex mutex_;
  libusb_context* context_ = nullptr;
  int initResult_ = 0;
};

}

std::unique_ptr<Fx2Bus> makeLibusbBus() { return std::make_unique<LibusbBus>(); }

}
