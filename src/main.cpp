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

#include "bitoscope_driver_usb_cpp.h"
#include "firmware_file.h"
#include "fx2_driver.h"
#include "fx2_libusb_bus.h"

int main(int argc, char** argv) {
  const std::unique_ptr<fx2la::Fx2Bus> bus = fx2la::makeLibusbBus();
  fx2la::Fx2Driver fx2(*bus, [] { return fx2la::firmwareFolders(fx2la::systemFirmwareSearch()); });
  bitoscope::plugin::Driver driver(fx2.description());
  return driver.run(argc, argv);
}
