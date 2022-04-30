/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "AutoTickThread.h"

#include "../Circuit.h"

#include <thread>

using namespace internal;

AutoTickThread::AutoTickThread()
{
}

AutoTickThread::~AutoTickThread()
{
    Stop();
}

::Component::TickMode AutoTickThread::Mode()
{
    return mode_;
}

bool AutoTickThread::IsStopped() const
{
    return stopped_;
}

bool AutoTickThread::IsPaused() const
{
    return pause_;
}

void AutoTickThread::Start(::Circuit* circuit, ::Component::TickMode mode)
{
    if ( !stopped_ )
    {
        return;
    }

    circuit_ = circuit;

    mode_ = mode;
    stop_ = false;
    stopped_ = false;
    pause_ = false;

    thread_ = std::thread(&AutoTickThread::Run, this);
}

void AutoTickThread::Stop()
{
    if (stopped_)
    {
        return;
    }

    Pause();

    stop_ = true;

    Resume();

    if (thread_.joinable())
    {
        thread_.join();
    }
}

void AutoTickThread::Pause()
{
    std::unique_lock<std::mutex> lock(resumeMutex_);

    if (!pause_ && !stopped_)
    {
        pause_ = true;
        pauseCondt_.wait( lock );  // wait for resume
    }
}

void AutoTickThread::Resume()
{
    std::unique_lock<std::mutex> lock(resumeMutex_);

    if (pause_)
    {
        resumeCondt_.notify_all();
        pause_ = false;
    }
}

void AutoTickThread::Run()
{
    if (circuit_ != nullptr)
    {
        while (!stop_)
        {
            circuit_->Tick(mode_);

            if (pause_)
            {
                std::unique_lock<std::mutex> lock(resumeMutex_);

                pauseCondt_.notify_all();

                resumeCondt_.wait(lock);  // wait for resume
            }
        }
    }

    stopped_ = true;
}