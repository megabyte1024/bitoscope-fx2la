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

set(FX2LA_FIRMWARE_VERSION 0.1.7)
set(FX2LA_FIRMWARE_SHA256 c876fd075549e7783a6d5bfc8d99a695cfc583ddbcea0217d8e3f9351d1723af)
set(FX2LA_FIRMWARE_NAME "sigrok-firmware-fx2lafw-bin-${FX2LA_FIRMWARE_VERSION}")

get_filename_component(fx2la_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
if(NOT DESTINATION)
  set(DESTINATION "${fx2la_root}/third_party/sigrok-firmware")
endif()

set(fx2la_work "${DESTINATION}.download")
file(REMOVE_RECURSE "${fx2la_work}")
file(MAKE_DIRECTORY "${fx2la_work}")
set(fx2la_archive "${fx2la_work}/${FX2LA_FIRMWARE_NAME}.tar.gz")
file(DOWNLOAD "https://sigrok.org/download/binary/sigrok-firmware-fx2lafw/${FX2LA_FIRMWARE_NAME}.tar.gz"
     "${fx2la_archive}" EXPECTED_HASH SHA256=${FX2LA_FIRMWARE_SHA256} STATUS fx2la_status)
list(GET fx2la_status 0 fx2la_code)
if(NOT fx2la_code EQUAL 0)
  file(REMOVE_RECURSE "${fx2la_work}")
  message(FATAL_ERROR "Download of ${FX2LA_FIRMWARE_NAME}.tar.gz failed or its SHA-256 is not ${FX2LA_FIRMWARE_SHA256}: ${fx2la_status}")
endif()
file(ARCHIVE_EXTRACT INPUT "${fx2la_archive}" DESTINATION "${fx2la_work}")
file(REMOVE_RECURSE "${DESTINATION}")
file(MAKE_DIRECTORY "${DESTINATION}")
file(GLOB fx2la_files "${fx2la_work}/${FX2LA_FIRMWARE_NAME}/*")
file(COPY ${fx2la_files} DESTINATION "${DESTINATION}")
file(REMOVE_RECURSE "${fx2la_work}")
file(GLOB fx2la_fw "${DESTINATION}/*.fw")
list(LENGTH fx2la_fw fx2la_count)
message(STATUS "fx2lafw firmware ${FX2LA_FIRMWARE_VERSION} is in ${DESTINATION} (${fx2la_count} .fw files)")
