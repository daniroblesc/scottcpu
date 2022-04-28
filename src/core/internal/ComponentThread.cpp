/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include <iostream>

#include "ComponentThread.h"

using namespace internal;

ComponentThread::ComponentThread()
{
}

ComponentThread::~ComponentThread()
{
    Stop();
}

void ComponentThread::Start()
{
    std::cout << "Start()\n";

    if (!stopped_)
    {
        return;
    }

    stop_ = false;
    stopped_ = false;
    gotResume_ = false;
    gotSync_ = false;

    thread_ = std::thread(&ComponentThread::Run, this);

    Sync();

    std::cout << "Start( done )\n";
}

void ComponentThread::Stop()
{
    std::cout << "Stop()\n";

    if (stopped_)
    {
        return;
    }

    Sync();

    stop_ = true;

    Resume(tick_);

    if (thread_.joinable())
    {
        thread_.join();
    }

    std::cout << "Stop( done )\n";
}

void ComponentThread::Sync()
{
    std::cout << "Sync()\n";

    if (stopped_)
    {
        return;
    }

    std::unique_lock<std::mutex> lock(resumeMutex_);

    if (!gotSync_)  // if haven't already got sync
    {
        std::cout << "Sync( wait for sync... )\n";
        syncCondt_.wait(lock);  // wait for sync
        std::cout << "Sync( syncCondt_ signalled! )\n";
    }

    std::cout << "Sync( done )\n";
}

void ComponentThread::Resume( std::function<void()> const& tick )
{
    if (stopped_)
    {
        Start();
    }

    std::unique_lock<std::mutex> lock(resumeMutex_);

    gotSync_ = false;  // reset the sync flag

    tick_ = tick;

    gotResume_ = true;  // set the resume flag
    resumeCondt_.notify_all();
}


void ComponentThread::Run()
{
    while (!stop_)
    {
        {
            std::unique_lock<std::mutex> lock(resumeMutex_);

            gotSync_ = true;  // set the sync flag
            syncCondt_.notify_all();

            if (!gotResume_)  // if haven't already got resume
            {
                std::cout << "Run( waiting for resume... )\n";
                resumeCondt_.wait( lock );  // wait for resume
                std::cout << "Run( resumeCondt_ signalled! )\n";
            }
            gotResume_ = false;  // reset the resume flag
        }

        if (!stop_ )
        {
            tick_();
        }
    }

    stopped_ = true;
}