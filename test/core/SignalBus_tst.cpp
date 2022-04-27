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

#include "core/SignalBus.h"

/**
 * @brief Unit tests for SignalBus class
 */


class WhenWorkingWithSignalBus : public testing::Test 
{
protected:    
    void SetUp() override 
    {
        signalBus_ = std::make_shared<SignalBus>();
    }

    void TearDown() override 
    {
    } 

    void initialize(const std::shared_ptr<SignalBus>& signalBus, int signalCount)
    {
        signalBus->SetSignalCount(signalCount);
        ASSERT_EQ(signalBus->GetSignalCount(), signalCount);
    }

    void enableSignal(const std::shared_ptr<Signal>& signal)
    {
        onebit bit;
        bit.value = 1;
        signal->SetValue(bit);
        ASSERT_TRUE(signal->HasValue());
    }

    void enableSignal(const std::shared_ptr<SignalBus>& signalBus, int signalCount)
    {
        onebit b;
        b.value = 1;
        EXPECT_TRUE(signalBus->SetValue(signalCount, b));
    }

    void checkSignalEnabled(const std::shared_ptr<Signal>& signal)
    {
        onebit* ptrBit = signal->GetValue();
        ASSERT_EQ(ptrBit->value, 1);
    }

    void checkSignalEnabled(const std::shared_ptr<SignalBus>& signalBus, int signalCount)
    {
        onebit* ptrBit = signalBus->GetValue(signalCount);
        ASSERT_EQ(ptrBit->value, 1);
    }

    std::shared_ptr<SignalBus> signalBus_;
};

TEST_F(WhenWorkingWithSignalBus, setSignalCount) 
{
    EXPECT_EQ(signalBus_->GetSignalCount(), 0);
    initialize(signalBus_, 8);
}

TEST_F(WhenWorkingWithSignalBus, getInvalidSignal) 
{
    initialize(signalBus_, 8);

    std::shared_ptr<Signal> s = signalBus_->GetSignal(8);
    EXPECT_EQ(s, nullptr);
}

TEST_F(WhenWorkingWithSignalBus, aGivenSignalHasAValue) 
{
    // Arrange
    initialize(signalBus_, 8);

    // Act & Assert
    EXPECT_FALSE(signalBus_->HasValue(0));

    // Arrange
    std::shared_ptr<Signal> s0 = signalBus_->GetSignal(0);
    EXPECT_NE(s0, nullptr);
    enableSignal(s0);

    // Act & Assert
    EXPECT_TRUE(signalBus_->HasValue(0));
}

TEST_F(WhenWorkingWithSignalBus, SetGetASingleSignal_HappyPath) 
{
    initialize(signalBus_, 8);

    int signalIndex = 1;

    enableSignal(signalBus_, signalIndex);

    checkSignalEnabled(signalBus_, signalIndex);
    EXPECT_TRUE(signalBus_->HasValue(signalIndex));
}

TEST_F(WhenWorkingWithSignalBus, GetAnUnassignedSingleSignal) 
{
    initialize(signalBus_, 8);

    int signalIndex = 1;

    onebit* pBit = signalBus_->GetValue(signalIndex);
    EXPECT_EQ(pBit, nullptr);
}

TEST_F(WhenWorkingWithSignalBus, CopyASingleSignal_HappyPath) 
{
    initialize(signalBus_, 8);

    std::shared_ptr<Signal> s = std::make_shared<Signal>();
    enableSignal(s);

    int toSignalIndex = 1;
    signalBus_->CopySignal(toSignalIndex, s);

    checkSignalEnabled(signalBus_, toSignalIndex);
    EXPECT_TRUE(signalBus_->HasValue(toSignalIndex));
}

TEST_F(WhenWorkingWithSignalBus, MoveASingleSignal_HappyPath) 
{
    initialize(signalBus_, 8);

    std::shared_ptr<Signal> fromSignal = std::make_shared<Signal>();
    enableSignal(fromSignal);

    int toSignalIndex = 1;
    signalBus_->MoveSignal(toSignalIndex, fromSignal);

    checkSignalEnabled(signalBus_, toSignalIndex);
    EXPECT_TRUE(signalBus_->HasValue(toSignalIndex));
    EXPECT_FALSE(fromSignal->HasValue());
}

TEST_F(WhenWorkingWithSignalBus, ClearAllValues) 
{
    initialize(signalBus_, 8);

    enableSignal(signalBus_, 1);
    EXPECT_TRUE(signalBus_->HasValue(1));

    signalBus_->ClearAllValues();
    EXPECT_FALSE(signalBus_->HasValue(1));
}
    
    
