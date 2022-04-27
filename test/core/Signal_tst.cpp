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
#include <memory>

#include "core/Signal.h"

/**
 * @brief Unit tests for Signal class
 */

class WhenWorkingWithSignal : public testing::Test 
{
protected:    
    void SetUp() override 
    {
        signal_ = std::make_shared<Signal>();
        ASSERT_FALSE(signal_->HasValue());
    }    

    void TearDown() override 
    {
    } 

    void enableSignal(const std::shared_ptr<Signal>& signal)
    {
        onebit bit;
        bit.value = 1;
        signal->SetValue(bit);
        ASSERT_TRUE(signal->HasValue());
    }

    void disableSignal(const std::shared_ptr<Signal>& signal)
    {
        onebit bit;
        bit.value = 0;
        signal->SetValue(bit);
        ASSERT_TRUE(signal->HasValue());
    }

    void checkSignalEnabled(const std::shared_ptr<Signal>& signal)
    {
        onebit* ptrBit = signal->GetValue();
        ASSERT_EQ(ptrBit->value, 1);
    }

    void checkSignalDisabled(const std::shared_ptr<Signal>& signal)
    {
        onebit* ptrBit = signal->GetValue();
        ASSERT_EQ(ptrBit->value, 0);
    }

    std::shared_ptr<Signal> signal_;
};

TEST_F(WhenWorkingWithSignal, enableACreatedSignal) 
{
    enableSignal(signal_);
    checkSignalEnabled(signal_);
}

TEST_F(WhenWorkingWithSignal, disableAnEnabledSignal) 
{
    enableSignal(signal_);
    disableSignal(signal_);
    checkSignalDisabled(signal_);
}

TEST_F(WhenWorkingWithSignal, clearAnEnabledSignal) 
{
    // Arrange
    enableSignal(signal_);
    // Act
    signal_->ClearValue();
    // Assert
    EXPECT_FALSE(signal_->HasValue());
}

TEST_F(WhenWorkingWithSignal, copyFromAnEnabledSignalToASignalWithoutValue) 
{
    // Arrange
    enableSignal(signal_);

    // Act
    std::shared_ptr<Signal> s = std::make_shared<Signal>();
    EXPECT_TRUE(s->CopySignal(signal_));

    // Assert
    checkSignalEnabled(s);
}

TEST_F(WhenWorkingWithSignal, copyFromAnEnabledSignalToASignalWithValue) 
{
    // Arrange
    enableSignal(signal_);

    std::shared_ptr<Signal> s = std::make_shared<Signal>();
    disableSignal(s);
    checkSignalDisabled(s);

    // Act
    EXPECT_TRUE(s->CopySignal(signal_));

    // Assert
    checkSignalEnabled(s);
}

TEST_F(WhenWorkingWithSignal, copyFromAnUnitializedSignal) 
{
    Signal s;
    EXPECT_FALSE(s.CopySignal(signal_));
}

TEST_F(WhenWorkingWithSignal, moveFromAnEnabledSignalToASignalWithoutValue) 
{
    // Arrange
    enableSignal(signal_);

    // Act
    std::shared_ptr<Signal> s = std::make_shared<Signal>();
    EXPECT_TRUE(s->MoveSignal(signal_));

    // Assert
    checkSignalEnabled(s);
    ASSERT_FALSE(signal_->HasValue());
}

TEST_F(WhenWorkingWithSignal, moveFromAnEnabledSignalToASignalWithValue) 
{
    // Arrange
    enableSignal(signal_);

    std::shared_ptr<Signal> s = std::make_shared<Signal>();
    disableSignal(s);
    checkSignalDisabled(s);

    // Act
    EXPECT_TRUE(s->MoveSignal(signal_));

    // Assert
    checkSignalEnabled(s);
    ASSERT_FALSE(signal_->HasValue());
}