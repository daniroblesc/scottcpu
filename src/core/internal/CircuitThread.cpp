/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "CircuitThread.h"

using namespace internal;

CircuitThread::CircuitThread()
{
}

CircuitThread::~CircuitThread()
{
    Stop();
}

void CircuitThread::Start(std::vector<std::shared_ptr<::Component>>* components, int threadNo)
{
    if (!stopped_)
    {
        return;
    }

    components_ = components;
    threadNo_ = threadNo;

    stop_ = false;
    stopped_ = false;
    gotResume_ = false;
    gotSync_ = false;

    thread_ = std::thread(&CircuitThread::Run, this);

    Sync();
}

void CircuitThread::Stop()
{
    if (stopped_)
    {
        return;
    }

    Sync();

    stop_ = true;

    SyncAndResume(mode_);

    if (thread_.joinable())
    {
        thread_.join();
    }
}

void CircuitThread::Sync()
{
    if (stopped_)
    {
        return;
    }

    std::unique_lock<std::mutex> lock(resumeMutex_);

    if (!gotSync_)  // if haven't already got sync
    {
        syncCondt_.wait(lock);  // wait for sync
    }
}

void CircuitThread::SyncAndResume(::Component::TickMode mode)
{
    if (stopped_)
    {
        return;
    }

    std::unique_lock<std::mutex> lock(resumeMutex_);

    if (!gotSync_)  // if haven't already got sync
    {
        syncCondt_.wait(lock);  // wait for sync
    }
    gotSync_ = false;  // reset the sync flag

    mode_ = mode;

    gotResume_ = true;  // set the resume flag
    resumeCondt_.notify_all();
}

void CircuitThread::Run()
{
    if (components_ != nullptr)
    {
        while (!stop_)
        {
            {
                std::unique_lock<std::mutex> lock(resumeMutex_);

                gotSync_ = true;  // set the sync flag
                syncCondt_.notify_all();

                if (!gotResume_)  // if haven't already got resume
                {
                    resumeCondt_.wait(lock);  // wait for resume
                }
                gotResume_ = false;  // reset the resume flag
            }

            if (!stop_)
            {
                // You might be thinking: Can't we have each thread start on a different component?

                // Well no. Because threadNo == bufferNo, in order to maintain synchronisation
                // within the circuit, when a component wants to process its buffers in-order, it
                // requires that every other in-order component in the system has not only
                // processed its buffers in the same order, but has processed the same number of
                // buffers too.

                // E.g. 1,2,3 and 1,2,3. Not 1,2,3 and 2,3,1,2,3.

                for (auto& component : *components_ )
                {
                    component->Tick(mode_, threadNo_);
                }
                for (auto& component : *components_)
                {
                    component->Reset(threadNo_);
                }
            }
        }
    }

    stopped_ = true;
}