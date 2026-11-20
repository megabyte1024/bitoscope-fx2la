# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2026 megabyte1024
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.30...4.4)

project(bitoscope_fx2la VERSION 0.4.0 LANGUAGES CXX)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT MSVC AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "The driver is built with MSVC on Windows and with GCC or Clang on Linux (README.md).")
endif()

include(cmake/BitoscopeBuildJobs.cmake)

include(cmake/BitoscopeOptimization.cmake)

add_subdirectory(sdk)

include(cmake/BitoscopeOwnTargetWarnings.cmake)

function(fx2la_compile_settings target)
  target_compile_features(${target} PUBLIC cxx_std_23)
  if(MSVC)
    # WHY: C4324 only reports padding an alignas asks for, GCC and Clang have no such warning (owner, 2026-10-21).
    target_compile_options(${target} PRIVATE /W4 /wd4324 /utf-8)
    target_compile_definitions(${target} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endfunction()

add_library(fx2la_logic STATIC
    src/ezusb.cpp
    src/firmware_file.cpp
    src/fx2_attach.cpp
    src/fx2_capture_plan.cpp
    src/fx2_profiles.cpp
    src/fx2_protocol.cpp
    src/busy_core_keeper.cpp
    src/number_text.cpp
)
target_include_directories(fx2la_logic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(fx2la_logic PUBLIC bitoscope_driver_c_sdk)
fx2la_compile_settings(fx2la_logic)

add_library(fx2la_libusb STATIC src/fx2_libusb_bus.cpp)
target_link_libraries(fx2la_libusb PUBLIC fx2la_logic PRIVATE bitoscope_libusb)
fx2la_compile_settings(fx2la_libusb)

add_library(fx2la_driver STATIC
    src/fx2_analyzer.cpp
    src/fx2_driver.cpp
    src/fx2_identify.cpp
    src/fx2_usb_device.cpp
)
target_link_libraries(fx2la_driver PUBLIC fx2la_logic bitoscope_driver_usb_sdk PRIVATE fx2la_libusb bitoscope_libusb)
fx2la_compile_settings(fx2la_driver)

add_executable(fx2la src/main.cpp)
target_link_libraries(fx2la PRIVATE fx2la_driver)
fx2la_compile_settings(fx2la)
set_target_properties(fx2la PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/drivers")

set(fx2la_firmware_dir "${CMAKE_CURRENT_SOURCE_DIR}/third_party/sigrok-firmware")
if(IS_DIRECTORY "${fx2la_firmware_dir}")
  if(MSVC)
    set(fx2la_driver_dir "${CMAKE_BINARY_DIR}/drivers/$<CONFIG>")
  else()
    set(fx2la_driver_dir "${CMAKE_BINARY_DIR}/drivers")
  endif()
  add_custom_target(fx2la_firmware
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${fx2la_firmware_dir}" "${fx2la_driver_dir}/sigrok-firmware"
      VERBATIM)
  add_dependencies(fx2la fx2la_firmware)
  install(DIRECTORY "${fx2la_firmware_dir}" DESTINATION drivers)
endif()

install(TARGETS fx2la RUNTIME DESTINATION drivers)

if(EXISTS "${CMAKE_SOURCE_DIR}/private/CMakeLists.txt")
  # WHY: Here, not in the subdirectory: ctest looks for its test file in the top build directory.
  enable_testing()
  add_subdirectory(private)
endif()

# WHY: Last, after every add_subdirectory: a target added below this line would get no warnings or optimization.
bitoscope_enable_warnings_on_own_targets("${CMAKE_SOURCE_DIR}")
bitoscope_optimize_own_targets("${CMAKE_SOURCE_DIR}")
