/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#pragma once

#include "SignalBus.h"
#include <string>

namespace internal
{
    class Component;
}  // namespace internal


/**
 * @brief Abstract base class for core components
 *
 * Classes derived from Component can be added to a Circuit and routed to and 
 * from other Components.
 *
 * On construction, derived classes must configure the component's IO buses by
 * calling SetInputCount() and SetOutputCount() respectively.
 * 
 * Derived classes must also implement the virtual method: Process(). 
 * The Process() method is a callback from the core engine that occurs when a 
 * new set of input signals is ready for processing. 
 * The Process() method has 2 arguments: the input bus, and the output bus. 
 * This method's purpose is to pull its required inputs out of the input bus, 
 * process these inputs, and populate the output bus with the results 
 * (see SignalBus).
 *
 * In order for a component to do any work it must be ticked. 
 * This is performed by repeatedly calling the Tick() and Reset() methods. 
 * The Tick() method is responsible for acquiring the next set of input signals
 * from component input wires and populating the component's input bus. 
 * To insure that these inputs are up-to-date, the dependent component first calls
 * all of its input components' Tick() methods - hence recursively called in all 
 * components going backward through the circuit. 
 * The acquired input bus is then passed to the Process() method. 
 * The Reset() method informs the component that the last circuit traversal has 
 * completed and hence can execute the next Tick() request.
 *
 * <b>PERFORMANCE TIP:</b> If a component's Process() method is capable of 
 * processing buffers out-of-order within a stream processing circuit, consider
 * initialising its base with ProcessOrder::OutOfOrder to improve performance. 
 * Note however that Process() must be thread-safe to operate in this mode.
 */
class Component
{
public:
    NONCOPYABLE( Component );

    enum class ProcessOrder
    {
        InOrder,
        OutOfOrder
    };

    enum class TickMode
    {
        Series,
        Parallel
    };

    Component(ProcessOrder processOrder = ProcessOrder::InOrder);
    virtual ~Component();

    bool ConnectInput(const std::shared_ptr<Component>& fromComponent, int fromOutput, int toInput );

    void DisconnectInput(int inputNo);
    void DisconnectInput(const std::shared_ptr<Component const>& fromComponent);
    void DisconnectAllInputs();

    int GetInputCount() const;
    int GetOutputCount() const;

    std::string GetInputName( int inputNo ) const;
    std::string GetOutputName( int outputNo ) const;

    void SetBufferCount(int bufferCount);
    int GetBufferCount() const;

    bool Tick( TickMode mode = TickMode::Parallel, int bufferNo = 0 );
    void Reset( int bufferNo = 0 );

protected:

    virtual void Process( SignalBus const&, SignalBus& ) = 0;

    void SetInputCount(const int inputCount,   const std::vector<std::string>& inputNames  = {});
    void SetOutputCount(const int outputCount, const std::vector<std::string>& outputNames = {});

private:

    std::unique_ptr<internal::Component> p_;
};