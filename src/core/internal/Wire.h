/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#pragma once

#include "../Component.h"

namespace internal
{

/**
 * @brief Connection between two components
 *
 * Components process and transfer data between each other in the form of signals
 * via interconnected "wires". Each wire contains references to the fromComponent, 
 * the fromOutput signal, and the toInput signal. The Wire struct simply stores 
 * these references for use in retrieving and providing signals across component 
 * connections.
 *
 */

struct Wire final
{
    Wire(const std::shared_ptr<::Component>& newFromComponent, int newFromOutput, int newToInput )
        : fromComponent_( newFromComponent )
        , fromOutput_( newFromOutput )
        , toInput_( newToInput )
    {
    }

    std::shared_ptr<::Component> fromComponent_;
    int fromOutput_;
    int toInput_;
};

}  // namespace internal