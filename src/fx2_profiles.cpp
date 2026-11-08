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

#include "fx2_profiles.h"

namespace fx2la {

namespace {

constexpr Fx2Profile kProfiles[] = {
    {0x08a9, 0x0014, "CWAV", "USBee AX", "fx2lafw-cwav-usbeeax.fw", kCapabilityAnalogAx},
    {0x08a9, 0x0015, "CWAV", "USBee DX", "fx2lafw-cwav-usbeedx.fw", kCapabilityWide16},
    {0x08a9, 0x0009, "CWAV", "USBee SX", "fx2lafw-cwav-usbeesx.fw", 0},
    {0x08a9, 0x0005, "CWAV", "USBee ZX", "fx2lafw-cwav-usbeezx.fw", 0},
    {0x0925, 0x3881, "Saleae", "Logic", "fx2lafw-saleae-logic.fw", 0},
    {0x04b4, 0x8613, "Cypress", "FX2", "fx2lafw-cypress-fx2.fw", kCapabilityWide16},
    {0x16d0, 0x0498, "Braintechnology", "USB-LPS", "fx2lafw-braintechnology-usb-lps.fw", kCapabilityWide16},
    {0x1d50, 0x608c, "sigrok", "FX2 LA (8ch)", "fx2lafw-sigrok-fx2-8ch.fw", 0},
    {0x1d50, 0x608d, "sigrok", "FX2 LA (16ch)", "fx2lafw-sigrok-fx2-16ch.fw", kCapabilityWide16},
    {0x1d50, 0x608f, "sigrok", "usb-c-grok", "fx2lafw-usb-c-grok.fw", 0},
};

constexpr uint32_t kNarrowChannels = 8;
constexpr uint32_t kWideChannels = 16;

}

uint32_t Fx2Profile::logicChannels() const { return (capabilities & kCapabilityWide16) != 0 ? kWideChannels : kNarrowChannels; }

std::span<const Fx2Profile> fx2Profiles() { return kProfiles; }

const Fx2Profile* findProfile(uint16_t vendorId, uint16_t productId) {
  for (const Fx2Profile& profile : kProfiles) {
    if (profile.vendorId == vendorId && profile.productId == productId) return &profile;
  }
  return nullptr;
}

std::string displayName(const Fx2Profile& profile) {
  std::string name(profile.vendor);
  name += ' ';
  name += profile.model;
  return name;
}

}
