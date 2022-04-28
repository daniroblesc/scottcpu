/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#pragma once

#include "Common.h"

#include <condition_variable>
#include <thread>
#include <functional>

namespace internal
{


/**
 * @brief Thread class for asynchronously ticking a single circuit component
 *
 * A ComponentThread's primary purpose is to tick parallel circuit components in parallel.
 * Upon Start(), an internal thread will spawn and wait for the first call to 
 * Resume() before executing the tick method provided. A call to Sync() will then
 * block until the thread has completed execution of the tick method. At this 
 * point, the thread will wait until instructed to resume again.
 *
 */

class ComponentThread final
{
public:
    NONCOPYABLE( ComponentThread );

    ComponentThread();
    ~ComponentThread();

    void Start();
    void Stop();
    void Sync();
    void Resume( std::function<void()> const& tick );

private:
    void Run();

private:
    std::thread thread_;
    bool stop_ = false;
    bool stopped_ = true;
    bool gotResume_ = false;
    bool gotSync_ = true;
    std::mutex resumeMutex_;
    std::condition_variable resumeCondt_, syncCondt_;
    std::function<void()> tick_;
};


} // namespace internal