/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#pragma once

#include "../Circuit.h"
#include "../../Common.h"

#include <condition_variable>
#include <thread>

namespace internal
{

/**
 * @brief Thread class for auto-ticking a circuit
 *
 * An AutoTickThread is responsible for ticking a circuit continuously in a 
 * free-running thread. Upon initialisation, a reference to the circuit must 
 * be provided for the thread's Run() method to use. Once Start() has been 
 * called, the thread will begin, repeatedly calling the circuit's Tick()
 * method until instructed to Pause() or Stop().
*/

class AutoTickThread final
{
public:
    NONCOPYABLE( AutoTickThread );

    AutoTickThread();
    ~AutoTickThread();

    ::Component::TickMode Mode();

    bool IsStopped() const;
    bool IsPaused() const;

    void Start( ::Circuit* circuit, ::Component::TickMode mode );
    void Stop();
    void Pause();
    void Resume();

private:
    void Run();

private:
    ::Component::TickMode mode_;
    std::thread thread_;
    ::Circuit* circuit_ = nullptr;
    bool stop_ = false;
    bool pause_ = false;
    bool stopped_ = true;
    std::mutex resumeMutex_;
    std::condition_variable resumeCondt_, pauseCondt_;
};

}  // namespace internal