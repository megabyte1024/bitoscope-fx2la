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

function(bitoscope_is_own_source path out)
  set(own FALSE)
  foreach(root IN ITEMS "${CMAKE_BINARY_DIR}" "${CMAKE_SOURCE_DIR}")
    cmake_path(IS_PREFIX root "${path}" NORMALIZE inside)
    if(inside)
      file(RELATIVE_PATH relative "${root}" "${path}")
      if(NOT "/${relative}" MATCHES "/(third_party|deps|vendor|_deps)/")
        set(own TRUE)
      endif()
      break()
    endif()
  endforeach()
  set(${out} ${own} PARENT_SCOPE)
endfunction()

function(bitoscope_enable_warnings_on_own_targets dir)
  get_property(subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
  foreach(subdir IN LISTS subdirs)
    bitoscope_enable_warnings_on_own_targets("${subdir}")
  endforeach()
  get_property(targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
  foreach(target IN LISTS targets)
    get_target_property(type ${target} TYPE)
    if(NOT type MATCHES "^(STATIC_LIBRARY|SHARED_LIBRARY|MODULE_LIBRARY|OBJECT_LIBRARY|EXECUTABLE)$")
      continue()
    endif()
    get_target_property(source_dir ${target} SOURCE_DIR)
    get_target_property(sources ${target} SOURCES)
    foreach(source IN LISTS sources)
      if(source MATCHES "^\\$<" OR NOT source MATCHES "\\.(c|cc|cpp|cxx|c\\+\\+|m|mm)$")
        continue()
      endif()
      cmake_path(ABSOLUTE_PATH source BASE_DIRECTORY "${source_dir}" NORMALIZE OUTPUT_VARIABLE absolute)
      bitoscope_is_own_source("${absolute}" own)
      if(own)
        bitoscope_enable_warnings(${target})
        break()
      endif()
    endforeach()
  endforeach()
endfunction()
