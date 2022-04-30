/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#pragma once

#include "../Component.h"

#include <condition_variable>
#include <thread>

namespace internal
{

/**
 * @brief Thread class for asynchronously ticking circuit components
 *
 * A CircuitThread is responsible for ticking and reseting all components 
 * within a Circuit. Upon initialisation, a reference to the vector of circuit
 * components must be provided for the thread Run() method to loop through. 
 * Each CircuitThread has a thread number (threadNo), which is also provided upon
 * initialisation. When creating multiple CircuitThreads, each thread must have 
 * their own unique thread number, beginning at 0 and incrementing by 1 for every
 * thread added. This thread number corresponds with the Component's buffer number 
 * when calling it's Tick() and Reset() methods in the CircuitThread's component loop. 
 * Hence, for every circuit thread created, each component's buffer count within
 * that circuit must be incremented to match.
 * The SyncAndResume() method causes the CircuitThread to tick and reset all 
 * circuit components once, after which the thread will wait until instructed 
 * to resume again. As each component is done processing it hands over control 
 * to the next waiting circuit thread, therefore, from an external control loop 
 * (I.e. Circuit's Tick() method) we can simply loop through our array of 
 * CircuitThreads calling SyncAndResume() on each. If a circuit thread is busy 
 * processing, a call to SyncAndResume() will block momentarily until that thread 
 * is done processing.
 */

class CircuitThread final
{
public:
    NONCOPYABLE( CircuitThread );
    
    CircuitThread();
    ~CircuitThread();

    void Start(std::vector<std::shared_ptr<::Component>>* components, int threadNo);
    void Stop();
    void Sync();
    void SyncAndResume(::Component::TickMode mode);

private:
    void Run();

private:
    ::Component::TickMode mode_;
    std::thread thread_;
    std::vector<std::shared_ptr<::Component>>* components_ = nullptr;
    int threadNo_ = 0;
    bool stop_ = false;
    bool stopped_ = true;
    bool gotResume_ = false;
    bool gotSync_ = true;
    std::mutex resumeMutex_;
    std::condition_variable resumeCondt_, syncCondt_;
};

}  // namespace internal