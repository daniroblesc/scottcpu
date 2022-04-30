/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "core/Component.h"

/**
 * @brief Unit tests for Component class
 */


class WhenWorkingWithComponent : public testing::Test 
{
protected:    
    void SetUp() override 
    {
    }

    void TearDown() override 
    {
    } 

    class AND : public Component
    {
    public:
        AND() : Component( ProcessOrder::OutOfOrder )
        {
            SetInputCount(2);
            SetOutputCount(1);
        }

    protected:
        virtual void Process( SignalBus const& inputs, SignalBus& outputs ) override
        {
            bool in0 = inputs.GetValue(0);
            bool in1 = inputs.GetValue(1);

            onebit retval;
            retval.value = (in0 && in1);
            outputs.SetValue(0, retval);
        }
    };
};

TEST_F(WhenWorkingWithComponent, andComponent) 
{
    auto andComponent = std::make_shared<AND>();
}

