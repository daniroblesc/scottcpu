/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

 #include "SignalBus.h"
#include <memory>


namespace internal
{

class SignalBus
{
public:
    std::shared_ptr<Signal> nullSignal = nullptr;
};

}  // namespace internal


SignalBus::SignalBus() 
{
    p_ = std::make_unique<internal::SignalBus>();
}

SignalBus::SignalBus(SignalBus&& rhs)
{
    signals_ = rhs.signals_;
}

SignalBus::~SignalBus()
{  
}

void SignalBus::SetSignalCount(const int signalCount)
{
    int fromSize = signals_.size();

    signals_.resize(signalCount);

    for (int i = fromSize; i < signalCount; ++i)
    {
        signals_[i] = std::make_shared<Signal>();
    }
}

int SignalBus::GetSignalCount() const
{
    return signals_.size();
}

const std::shared_ptr<Signal>& SignalBus::GetSignal(const int signalIndex) const
{
    if ( (size_t)signalIndex < signals_.size() )
    {
        return signals_[signalIndex];
    }
    else
    {
        return p_->nullSignal;
    }    
}

bool SignalBus::HasValue(const int signalIndex) const
{
    if ( (size_t)signalIndex < signals_.size() )
    {
        return signals_[signalIndex]->HasValue();
    }
    else
    {
        return false;
    }    
}

onebit* SignalBus::GetValue(const int signalIndex) const
{
    if ( (size_t)signalIndex < signals_.size() )
    {
        return signals_[signalIndex]->GetValue();
    }
    else
    {
        return nullptr;
    }
}

bool SignalBus::SetValue(const int signalIndex, const onebit& newValue)
{
    if ( (size_t)signalIndex < signals_.size() )
    {
        signals_[signalIndex]->SetValue( newValue );
        return true;
    }
    else
    {
        return false;
    }
}

bool SignalBus::CopySignal(const int toSignalIndex, const std::shared_ptr<Signal>& fromSignal)
{
    if ( (size_t)toSignalIndex < signals_.size() )
    {
        return signals_[toSignalIndex]->CopySignal( fromSignal );
    }
    else
    {
        return false;
    }    
}

bool SignalBus::MoveSignal(const int toSignalIndex, const std::shared_ptr<Signal>& fromSignal)
{
    if ( (size_t)toSignalIndex < signals_.size() )
    {
        return signals_[toSignalIndex]->MoveSignal( fromSignal );
    }
    else
    {
        return false;
    }    
}

void SignalBus::ClearAllValues()
{
    for ( auto& signal : signals_ )
    {
        signal->ClearValue();
    }
}