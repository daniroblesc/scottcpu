/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "Signal.h"

#include <iostream>

Signal::Signal()
{}

Signal::~Signal()
{
    if (valueHolder_ != nullptr)
    {
        delete valueHolder_;
    }    
}

bool Signal::HasValue() const
{
    return hasValue_;
}

void Signal::ClearValue()
{
    hasValue_ = false;
}

onebit* Signal::GetValue()
{
    if (hasValue_)
    {
        return &(valueHolder_ ->value_);
    }
    else 
    {
        return nullptr;
    }    
}

void Signal::SetValue(const onebit& newValue)
{
    if (valueHolder_ == nullptr)
    {
        valueHolder_ = new ValueHolder( newValue );
    }
    else 
    {
        valueHolder_->value_ = newValue;    
    }
    
    hasValue_ = true;
}

bool Signal::CopySignal(const std::shared_ptr<Signal>& fromSignal)
{
    if (fromSignal != nullptr && fromSignal->hasValue_)
    {
        if (valueHolder_ != nullptr && fromSignal->valueHolder_ != nullptr)
        {
            valueHolder_->SetValue(fromSignal->valueHolder_);
        }
        else
        {
            delete valueHolder_;
            valueHolder_ = fromSignal->valueHolder_->GetCopy();
        }

        hasValue_ = true;
        return true;
    }
    else
    {
        return false;
    }    
}

bool Signal::MoveSignal(const std::shared_ptr<Signal>& fromSignal)
{
    if (fromSignal != nullptr && fromSignal->hasValue_)
    {
        std::swap(fromSignal->valueHolder_, valueHolder_);
        fromSignal->hasValue_ = false;

        hasValue_ = true;
        return true;
    }
    else
    {
        return false;
    }
}