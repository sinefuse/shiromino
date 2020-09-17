/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE-src file at the top level
 * directory for the full text of the license.
 */
#pragma once
#include "Settings.h"

namespace Shiro {
    void GlobalInit(const Shiro::Settings& settings);
    void GlobalQuit();
}
