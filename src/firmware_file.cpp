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

#include "firmware_file.h"

#include <cstdlib>
#include <format>
#include <fstream>
#include <iterator>
#include <memory>
#include <system_error>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ezusb.h"
#include "number_text.h"

namespace fs = std::filesystem;

namespace fx2la {

namespace {

constexpr const char* kFirmwareFolderName = "sigrok-firmware";
constexpr const char* kEnvironmentName = "SIGROK_FIRMWARE_DIR";

std::optional<std::string> environmentValue(const char* name) {
#ifdef _MSC_VER
  char* raw = nullptr;
  size_t length = 0;
  if (_dupenv_s(&raw, &length, name) != 0 || raw == nullptr) return std::nullopt;
  const std::unique_ptr<char, decltype(&std::free)> value(raw, &std::free);
  if (value.get()[0] == '\0') return std::nullopt;
  return std::string(value.get());
#else
  const char* value = std::getenv(name);
  if (value == nullptr || value[0] == '\0') return std::nullopt;
  return std::string(value);
#endif
}

#ifdef _WIN32
fs::path executableFolder() {
  std::wstring path(MAX_PATH, L'\0');
  for (;;) {
    const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (length == 0) return {};
    if (length < path.size()) {
      path.resize(length);
      break;
    }
    path.resize(path.size() * 2);
  }
  return fs::path(path).parent_path();
}
#else
fs::path executableFolder() {
  std::error_code error;
  const fs::path path = fs::read_symlink("/proc/self/exe", error);
  return error ? fs::path() : path.parent_path();
}
#endif

bool plainFileName(std::string_view name) {
  if (name.empty() || name == "." || name == "..") return false;
  return fs::path(name).filename() == fs::path(name);
}

}

std::vector<fs::path> firmwareFolders(const FirmwareSearch& search) {
  std::vector<fs::path> folders;
  if (search.environmentFolder) folders.push_back(*search.environmentFolder);
  if (!search.besideDriver.empty()) folders.push_back(search.besideDriver);
  if (search.userDataFolder) folders.push_back(*search.userDataFolder);
  folders.insert(folders.end(), search.systemFolders.begin(), search.systemFolders.end());
  return folders;
}

FirmwareSearch systemFirmwareSearch() {
  FirmwareSearch search;
  if (const auto folder = environmentValue(kEnvironmentName)) search.environmentFolder = fs::path(*folder);
  if (const fs::path beside = executableFolder(); !beside.empty()) search.besideDriver = beside / kFirmwareFolderName;
#ifdef _WIN32
  search.systemFolders = {fs::path(L"C:\\Program Files (x86)\\sigrok\\PulseView\\share\\sigrok-firmware")};
#else
  if (const auto data = environmentValue("XDG_DATA_HOME")) {
    search.userDataFolder = fs::path(*data) / kFirmwareFolderName;
  } else if (const auto home = environmentValue("HOME")) {
    search.userDataFolder = fs::path(*home) / ".local" / "share" / kFirmwareFolderName;
  }
  search.systemFolders = {"/usr/local/share/sigrok-firmware", "/usr/share/sigrok-firmware"};
#endif
  return search;
}

std::expected<std::vector<uint8_t>, std::string> loadFirmware(std::string_view fileName,
                                                              std::span<const fs::path> folders) {
  if (!plainFileName(fileName)) return std::unexpected(std::format("'{}' is no firmware file name", fileName));
  for (const fs::path& folder : folders) {
    const fs::path path = folder / fs::path(std::string(fileName));
    std::error_code error;
    if (!fs::is_regular_file(path, error)) continue;
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return std::unexpected(std::format("firmware file {} could not be read", path.string()));
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (in.bad()) return std::unexpected(std::format("firmware file {} could not be read", path.string()));
    if (bytes.empty()) return std::unexpected(std::format("firmware file {} is empty", path.string()));
    if (bytes.size() > kEzusbMaxFirmwareBytes) {
      return std::unexpected(std::format("firmware file {} has {} bytes: EZ-USB takes at most {}", path.string(),
                                         groupedNumber(bytes.size()), groupedNumber(kEzusbMaxFirmwareBytes)));
    }
    return bytes;
  }
  const std::string remedy = std::format("fetch fx2lafw firmware or set {}", kEnvironmentName);
  if (folders.empty()) {
    return std::unexpected(std::format("firmware file '{}' not found: no folder to look in; {}", fileName, remedy));
  }
  std::string where;
  for (const fs::path& folder : folders) where += (where.empty() ? "" : ", ") + folder.string();
  return std::unexpected(std::format("firmware file '{}' not found in: {}; {}", fileName, where, remedy));
}

}
