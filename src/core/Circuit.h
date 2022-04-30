/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#pragma once

#include "Component.h"

namespace internal
{
    class Circuit;
}

/**
 * @brief Workspace for adding and routing components
 *
 * Components can be added to a Circuit via the AddComponent() method, and 
 * routed to and from other components via the ConnectOutToIn() methods.
 *
 * <b>NOTE:</b> Each component input can only accept a single "wire" at a time. 
 * When a wire is connected to an input that already has a connected wire, that
 * wire is replaced with the new one.
 * One output, on the other hand, can be distributed to multiple inputs.
 * To boost performance in stream processing circuits, multi-buffering can be 
 * enabled via the SetBufferCount() method. A circuit's buffer count can be 
 * adjusted at runtime.
 * The Circuit Tick() method runs through it's internal array of components and
 * calls each component's Tick() and Reset() methods once. A circuit's Tick() 
 * method can be called in a loop from the main application thread, or alternatively, 
 * by calling StartAutoTick(), a separate thread will spawn, automatically calling 
 * Tick() continuously until PauseAutoTick() or StopAutoTick() is called.
 * TickMode::Parallel (default) will spawn a thread per component in a circuit. 
 * The aim of this mode is to improve the performance of circuits that contain 
 * parallel branches. TickMode::Series on the other hand, tells the circuit to 
 * tick its components one-by-one in a single thread. This mode aims to improve 
 * the performance of circuits that do not contain parallel branches.
 */ 

class Circuit final
{
public:
    NONCOPYABLE( Circuit );

    Circuit();
    ~Circuit();

    int AddComponent(const std::shared_ptr<Component>& component);

    void RemoveComponent(const std::shared_ptr<Component const>& component);
    void RemoveComponent(int componentIndex);
    void RemoveAllComponents();

    int GetComponentCount() const;

    bool ConnectOutToIn(const std::shared_ptr<Component const>& fromComponent, int fromOutput, const std::shared_ptr<Component const>& toComponent, int toInput );
    bool ConnectOutToIn(const std::shared_ptr<Component const>& fromComponent, int fromOutput, int toComponent, int toInput);
    bool ConnectOutToIn(int fromComponent, int fromOutput, const std::shared_ptr<Component const>& toComponent, int toInput);
    bool ConnectOutToIn(int fromComponent, int fromOutput, int toComponent, int toInput);

    void DisconnectComponent(const std::shared_ptr<::Component const>& component);
    void DisconnectComponent(int componentIndex);

    void SetBufferCount( int bufferCount );
    int GetBufferCount() const;

    void Tick(Component::TickMode mode = Component::TickMode::Parallel);

    void StartAutoTick(Component::TickMode mode = Component::TickMode::Parallel);
    void StopAutoTick();
    void PauseAutoTick();
    void ResumeAutoTick();

private:
    std::unique_ptr<internal::Circuit> p_;    
};
