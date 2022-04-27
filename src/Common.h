/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#pragma once

#include <memory>

#define NONCOPYABLE( classname )            \
    classname( classname const& ) = delete; \
    classname& operator=( classname const& ) = delete

struct OneBit {
    unsigned char value:1;
};
typedef struct OneBit onebit;    