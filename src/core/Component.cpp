/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "Component.h"

#include "internal/ComponentThread.h"
#include "internal/Wire.h"

#include <mutex>
#include <condition_variable>
#include <unordered_set>

namespace internal
{

class Component
{
public:

    enum class TickStatus
    {
        NotTicked,
        TickStarted,
        Ticking
    };

    Component(::Component::ProcessOrder processOrder) : processOrder_(processOrder)
    {}

    void WaitForRelease( int threadNo );
    void ReleaseThread( int threadNo );

    void GetOutput( int bufferNo, int fromOutput, int toInput, ::SignalBus& toBus, ::Component::TickMode mode );

    void IncRefs( int output );
    void DecRefs(int output);

    const ::Component::ProcessOrder processOrder_;

    int bufferCount_ = 0;

    std::vector<::SignalBus> inputBuses_;
    std::vector<::SignalBus> outputBuses_;

    std::vector<std::vector<std::pair<int, int>>> refs_;  // ref_total:ref_counter per output, per buffer
    std::vector<std::vector<std::unique_ptr<std::mutex>>> refMutexes_;

    std::vector<Wire> inputWires_;

    std::vector<std::unique_ptr<ComponentThread>> componentThreads_;
    std::vector<std::unordered_set<Wire*>> feedbackWires_;

    std::vector<TickStatus> tickStatuses_;
    std::vector<bool> gotReleases_;
    std::vector<std::unique_ptr<std::mutex>> releaseMutexes_;
    std::vector<std::unique_ptr<std::condition_variable>> releaseCondts_;

    std::vector<std::string> inputNames_;
    std::vector<std::string> outputNames_;
};

}  // namespace internal

Component::Component(ProcessOrder processOrder)
{
    p_ = std::make_unique<internal::Component>(processOrder); 
    SetBufferCount(1);
}

Component::~Component()
{
    DisconnectAllInputs();    
}

bool Component::ConnectInput(const std::shared_ptr<Component>& fromComponent, int fromOutput, int toInput )
{
    if (fromOutput >= fromComponent->GetOutputCount() || 
        toInput >= p_->inputBuses_[0].GetSignalCount() )
    {
        return false;
    }

    // first make sure there are no wires already connected to this input
    DisconnectInput( toInput );

    p_->inputWires_.emplace_back( fromComponent, fromOutput, toInput );

    // update source output's reference count
    fromComponent->p_->IncRefs( fromOutput );

    return true;
}
void Component::DisconnectInput(int inputNo)
{
    // remove wires connected to inputNo from inputWires
    for ( auto it = p_->inputWires_.begin(); it != p_->inputWires_.end(); ++it )
    {
        if ( it->toInput_ == inputNo )
        {
            // update source output's reference count
            it->fromComponent_->p_->DecRefs( it->fromOutput_ );

            p_->inputWires_.erase( it );
            break;
        }
    }    
}

void Component::DisconnectInput(const std::shared_ptr<Component const>& fromComponent)
{
    // remove fromComponent from inputWires
    for ( auto it = p_->inputWires_.begin(); it != p_->inputWires_.end(); )
    {
        if ( it->fromComponent_ == fromComponent )
        {
            // update source output's reference count
            fromComponent->p_->DecRefs( it->fromOutput_ );

            it = p_->inputWires_.erase( it );
        }
        else
        {
            ++it;
        }
    }    
}

void Component::DisconnectAllInputs()
{
    // remove all wires from inputWires
    for ( int i = 0; i < p_->inputBuses_[0].GetSignalCount(); ++i )
    {
        DisconnectInput( i );
    }
}

int Component::GetInputCount() const
{
    return p_->inputBuses_[0].GetSignalCount();
}

int Component::GetOutputCount() const
{
    return p_->outputBuses_[0].GetSignalCount();
}

std::string Component::GetInputName( int inputNo ) const
{
    if ( inputNo < (int)p_->inputNames_.size() )
    {
        return p_->inputNames_[inputNo];
    }
    return "";
}

std::string Component::GetOutputName( int outputNo ) const
{
    if ( outputNo < (int)p_->outputNames_.size() )
    {
        return p_->outputNames_[outputNo];
    }
    return "";
}

void Component::SetBufferCount(int bufferCount)
{
    // p_->bufferCount is the current thread count / bufferCount is new thread count

    if (bufferCount <= 0)
    {
        bufferCount = 1;  // there needs to be at least 1 buffer
    }

    // resize vectors
    p_->componentThreads_.resize( bufferCount );
    p_->feedbackWires_.resize( bufferCount );

    p_->tickStatuses_.resize( bufferCount );

    p_->inputBuses_.resize( bufferCount );
    p_->outputBuses_.resize( bufferCount );

    p_->gotReleases_.resize( bufferCount );
    p_->releaseMutexes_.resize( bufferCount );
    p_->releaseCondts_.resize( bufferCount );

    p_->refs_.resize( bufferCount );
    p_->refMutexes_.resize( bufferCount );

    // init new vector values
    for (int i = p_->bufferCount_; i < bufferCount; ++i)
    {
        p_->componentThreads_[i] = std::unique_ptr<internal::ComponentThread>( new internal::ComponentThread() );

        p_->tickStatuses_[i] = internal::Component::TickStatus::NotTicked;

        p_->inputBuses_[i].SetSignalCount(p_->inputBuses_[0].GetSignalCount());
        p_->outputBuses_[i].SetSignalCount(p_->outputBuses_[0].GetSignalCount());

        p_->gotReleases_[i] = false;
        p_->releaseMutexes_[i] = std::unique_ptr<std::mutex>( new std::mutex() );
        p_->releaseCondts_[i] = std::unique_ptr<std::condition_variable>( new std::condition_variable() );

        p_->refs_[i].resize(p_->refs_[0].size());
        for (size_t j = 0; j < p_->refs_[0].size(); ++j)
        {
            // sync output reference counts
            p_->refs_[i][j] = p_->refs_[0][j];
        }

        p_->refMutexes_[i].resize(p_->refMutexes_[0].size());
        for (size_t j = 0; j < p_->refs_[0].size(); ++j)
        {
            // construct new output reference mutexes
            p_->refMutexes_[i][j] = std::unique_ptr<std::mutex>( new std::mutex() );
        }
    }

    p_->gotReleases_[0] = true;

    p_->bufferCount_ = bufferCount;    
}

int Component::GetBufferCount() const
{
    return p_->inputBuses_.size();
}

bool Component::Tick( Component::TickMode mode, int bufferNo )
{
    if ( p_->tickStatuses_[bufferNo] == internal::Component::TickStatus::TickStarted )
    {
        // return false to indicate that we have already started a tick, and hence, are a feedback component.
        return false;
    }

    if ( p_->tickStatuses_[bufferNo] == internal::Component::TickStatus::Ticking )
    {
        // return true to indicate that we are now in "Ticking" state.
        return true;
    }

    // This component has not already been ticked

    // 1. set tickStatus -> TickStarted
    p_->tickStatuses_[bufferNo] = internal::Component::TickStatus::TickStarted;

    // 2. tick incoming components
    for ( auto& wire : p_->inputWires_ )
    {
        if ( mode == TickMode::Series )
        {
            wire.fromComponent_->Tick( mode, bufferNo );
        }
        else if ( mode == TickMode::Parallel )
        {
            if ( !wire.fromComponent_->Tick( mode, bufferNo ) )
            {
                p_->feedbackWires_[bufferNo].emplace( &wire );
            }
        }
    }

    // 3. set tickStatus -> Ticking
    p_->tickStatuses_[bufferNo] = internal::Component::TickStatus::Ticking;

    auto tick = [this, mode, bufferNo]() {
        // 4. get new inputs from incoming components
        for ( auto& wire : p_->inputWires_ )
        {
            if ( mode == TickMode::Parallel )
            {
                // wait for non-feedback incoming components to finish ticking
                auto wireIndex = p_->feedbackWires_[bufferNo].find( &wire );
                if ( wireIndex == p_->feedbackWires_[bufferNo].end() )
                {
                    wire.fromComponent_->p_->componentThreads_[bufferNo]->Sync();
                }
                else
                {
                    p_->feedbackWires_[bufferNo].erase( wireIndex );
                }
            }

            wire.fromComponent_->p_->GetOutput( bufferNo, wire.fromOutput_, wire.toInput_, p_->inputBuses_[bufferNo], mode );
        }

        // You might be thinking: Why not clear the outputs in Reset()?

        // This is because we need components to hold onto their outputs long enough for any
        // loopback wires to grab them during the next tick. The same applies to how we handle
        // output reference counting in internal::Component::GetOutput(), reseting the counter upon
        // the final request rather than in Reset().

        // 5. clear outputs
        p_->outputBuses_[bufferNo].ClearAllValues();

        if ( p_->processOrder_ == ProcessOrder::InOrder && p_->bufferCount_ > 1 )
        {
            // 6. wait for our turn to process
            p_->WaitForRelease( bufferNo );

            // 7. call Process() with newly aquired inputs
            Process( p_->inputBuses_[bufferNo], p_->outputBuses_[bufferNo] );

            // 8. signal that we're done processing
            p_->ReleaseThread( bufferNo );
        }
        else
        {
            // 6. call Process() with newly aquired inputs
            Process( p_->inputBuses_[bufferNo], p_->outputBuses_[bufferNo] );
        }
    };

    // do tick
    if ( mode == TickMode::Series )
    {
        tick();
    }
    else if ( mode == TickMode::Parallel )
    {
        p_->componentThreads_[bufferNo]->Resume( tick );
    }
    
    // return true to indicate that we are now in "Ticking" state.
    return true;
}

void Component::Reset( int bufferNo )
{
    // wait for ticking to complete
    p_->componentThreads_[bufferNo]->Sync();

    // clear inputs
    p_->inputBuses_[bufferNo].ClearAllValues();

    // reset tickStatus
    p_->tickStatuses_[bufferNo] = internal::Component::TickStatus::NotTicked;
}

void Component::SetInputCount(const int inputCount,   const std::vector<std::string>& inputNames)
{
    p_->inputNames_ = inputNames;

    for (auto& inputBus : p_->inputBuses_)
    {
        inputBus.SetSignalCount(inputCount);
    }
}

void Component::SetOutputCount(const int outputCount, const std::vector<std::string>& outputNames)
{
    p_->outputNames_ = outputNames;

    for (auto& outputBus : p_->outputBuses_)
    {
        outputBus.SetSignalCount(outputCount);
    }

    // add reference counters for our new outputs
    for (auto& ref : p_->refs_)
    {
        ref.resize( outputCount );
    }
    for (auto& refMutexes : p_->refMutexes_)
    {
        refMutexes.resize( outputCount );
        for (auto& refMutex : refMutexes)
        {
            // construct new output reference mutexes
            if (!refMutex)
            {
                refMutex = std::unique_ptr<std::mutex>( new std::mutex() );
            }
        }
    }
}

void internal::Component::WaitForRelease( int threadNo )
{
    std::unique_lock<std::mutex> lock( *releaseMutexes_[threadNo] );

    if ( !gotReleases_[threadNo] )
    {
        releaseCondts_[threadNo]->wait( lock );  // wait for resume
    }
    gotReleases_[threadNo] = false;  // reset the release flag
}

void internal::Component::ReleaseThread( int threadNo )
{
    threadNo = threadNo + 1 == bufferCount_ ? 0 : threadNo + 1;  // we're actually releasing the next available thread

    std::lock_guard<std::mutex> lock( *releaseMutexes_[threadNo] );

    gotReleases_[threadNo] = true;
    releaseCondts_[threadNo]->notify_all();
}

void internal::Component::GetOutput(
    int bufferNo, int fromOutput, int toInput, ::SignalBus& toBus, ::Component::TickMode mode )
{
    if ( !outputBuses_[bufferNo].HasValue( fromOutput ) )
    {
        return;
    }

    auto& signal = outputBuses_[bufferNo].GetSignal( fromOutput );

    if ( mode == ::Component::TickMode::Parallel && refs_[bufferNo][fromOutput].first > 1 )
    {
        refMutexes_[bufferNo][fromOutput]->lock();
    }

    if ( ++refs_[bufferNo][fromOutput].second == refs_[bufferNo][fromOutput].first )
    {
        // this is the final reference, reset the counter, move the signal
        refs_[bufferNo][fromOutput].second = 0;

        toBus.MoveSignal( toInput, signal );
    }
    else
    {
        // otherwise, copy the signal
        toBus.CopySignal( toInput, signal );
    }

    if ( mode == ::Component::TickMode::Parallel && refs_[bufferNo][fromOutput].first > 1 )
    {
        refMutexes_[bufferNo][fromOutput]->unlock();
    }
}

void internal::Component::IncRefs( int output )
{
    for ( auto& ref : refs_ )
    {
        ++ref[output].first;
    }
}

void internal::Component::DecRefs( int output )
{
    for ( auto& ref : refs_ )
    {
        --ref[output].first;
    }
}