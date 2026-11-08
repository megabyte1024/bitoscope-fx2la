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

#include <expected>
#include <filesystem>
#include <span>

#include "bitoscope_driver_usb_cpp.h"
#include "fx2_attach.h"

namespace fx2la {

std::expected<const Fx2Profile*, bitoscope::plugin::DeviceFailure> profileOf(const bitoscope::plugin::FoundDevice& found);

std::expected<const Fx2Profile*, bitoscope::plugin::DeviceFailure> identifyFx2(
    Fx2Bus& bus, const bitoscope::plugin::FoundDevice& found, std::span<const std::filesystem::path> firmwareFolders);

}
