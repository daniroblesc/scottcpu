/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#pragma once

#include "Signal.h"
#include <vector>

namespace internal
{
    class SignalBus;
}


/**
 * @brief Signal container
 *
 * A SignalBus contains Signals (see Signal). Via the Process_() method, a Component
 * receives signals into it's "inputs" SignalBus and provides signals to it's 
 * "outputs" SignalBus. The SignalBus class provides public getters and setters for 
 * manipulating it's internal Signal values directly, abstracting the need to retrieve 
 * and interface with the contained Signals themself. 
 */

class SignalBus final
{
public:
    NONCOPYABLE( SignalBus );

    SignalBus();
    SignalBus( SignalBus&& );
    ~SignalBus();

    void SetSignalCount(const int signalCount);
    int GetSignalCount() const;

    const std::shared_ptr<Signal>& GetSignal(const int signalIndex) const;
    
    bool HasValue(const int signalIndex) const;

    onebit* GetValue(const int signalIndex) const;
    bool SetValue(const int signalIndex, const onebit& newValue);

    bool CopySignal(const int toSignalIndex, const std::shared_ptr<Signal>& fromSignal);
    bool MoveSignal(const int toSignalIndex, const std::shared_ptr<Signal>& fromSignal);

    void ClearAllValues();

private:

    std::vector<std::shared_ptr<Signal>> signals_;
    std::unique_ptr<internal::SignalBus> p_;
};