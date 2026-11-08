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

#include "busy_core_keeper.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace fx2la {

namespace {

constexpr uint64_t kSpinsPerRound = 1u << 16;

void runAtLowestPriority() {
#if defined(_WIN32)
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
#endif
}

}

BusyCoreKeeper::BusyCoreKeeper(std::atomic<uint64_t>* roundCounter) {
  thread_ = std::thread([this, roundCounter] {
    runAtLowestPriority();
    while (!stopping_.load()) {
      for (uint64_t spin = 0; spin < kSpinsPerRound && !stopping_.load(); ++spin) {
      }
      if (roundCounter != nullptr) roundCounter->fetch_add(1);
    }
  });
}

BusyCoreKeeper::~BusyCoreKeeper() {
  stopping_.store(true);
  thread_.join();
}

}
