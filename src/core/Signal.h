/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#pragma once

#include "Common.h"
#include <assert.h>
#include <memory>

/**
 * @brief Value container used to carry data between components
 *
 * Components process and transfer data between each other in the form of "signals"
 * via interconnected wires. The Signal class holds a onebit value. 
 */

class Signal final
{
public:
    NONCOPYABLE( Signal );

    Signal();
    ~Signal();

    bool HasValue() const;
    void ClearValue();

    onebit* GetValue();
    void SetValue(const onebit& newValue);

    bool CopySignal(const std::shared_ptr<Signal>& fromSignal);
    bool MoveSignal(const std::shared_ptr<Signal>& fromSignal);

private:

    struct ValueHolder
    {
        NONCOPYABLE( ValueHolder );

        ValueHolder(const onebit& value) : value_( value )
        {
        }

        ValueHolder* GetCopy() const 
        {
            return new ValueHolder( value_ );
        }

        void SetValue( ValueHolder* valueHolder ) 
        {
            assert(valueHolder);
            value_ = valueHolder->value_;
        }

        onebit value_;
    };

    ValueHolder* valueHolder_ = nullptr;
    bool hasValue_ = false;
};