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

#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace fx2la {

struct FirmwareSearch {
  std::optional<std::filesystem::path> environmentFolder;
  std::optional<std::filesystem::path> userDataFolder;
  std::filesystem::path besideDriver;
  std::vector<std::filesystem::path> systemFolders;
};

std::vector<std::filesystem::path> firmwareFolders(const FirmwareSearch& search);

FirmwareSearch systemFirmwareSearch();

std::expected<std::vector<uint8_t>, std::string> loadFirmware(std::string_view fileName,
                                                              std::span<const std::filesystem::path> folders);

}
