/**
 * Copyright (c) 2014-present, The scottcpu authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include "Circuit.h"
#include "internal/AutoTickThread.h"
#include "internal/CircuitThread.h"

namespace internal
{

class Circuit
{
public:
    bool FindComponent(const std::shared_ptr<::Component const>& component, int& returnIndex) const;

    int pauseCount_ = 0;
    int currentThreadNo_ = 0;

    AutoTickThread autoTickThread_;

    std::vector<std::shared_ptr<::Component>> components_;
    std::vector<std::unique_ptr<CircuitThread>> circuitThreads_;
};

}  // namespace internal

Circuit::Circuit()
{
    p_ = std::make_unique<internal::Circuit>();     
}

Circuit::~Circuit()
{
    StopAutoTick();
    SetBufferCount( 0 );
    RemoveAllComponents();
}

int Circuit::AddComponent(const std::shared_ptr<Component>& component)
{
    if (component != nullptr)
    {
        int componentIndex;

        if ( p_->FindComponent( component, componentIndex ) )
        {
            return componentIndex;  // if the component is already in the array
        }

        // components within the circuit need to have as many buffers as there are threads in the circuit
        component->SetBufferCount( p_->circuitThreads_.size() );

        PauseAutoTick();
        p_->components_.emplace_back( component );
        ResumeAutoTick();

        return p_->components_.size() - 1;
    }

    return -1;
}

void Circuit::RemoveComponent(const std::shared_ptr<::Component const>& component)
{
    int componentIndex;

    if (p_->FindComponent(component, componentIndex))
    {
        RemoveComponent(componentIndex);
    }
}

void Circuit::RemoveComponent(int componentIndex)
{
    PauseAutoTick();

    DisconnectComponent(componentIndex);

    if (!p_->components_.empty())
    {
        p_->components_.erase(p_->components_.begin() + componentIndex);
    }

    ResumeAutoTick();
}

void Circuit::RemoveAllComponents()
{
    for (size_t i = 0; i < p_->components_.size(); ++i)
    {
        RemoveComponent( i-- );  // size drops as one is removed
    }
}

int Circuit::GetComponentCount() const
{
    return p_->components_.size();
}

bool Circuit::ConnectOutToIn(const std::shared_ptr<Component const>& fromComponent, int fromOutput, const std::shared_ptr<Component const>& toComponent, int toInput)
{
    int toComponentIndex;
    int fromComponentIndex;
    if ( p_->FindComponent( fromComponent, fromComponentIndex ) && p_->FindComponent( toComponent, toComponentIndex ) )
    {
        return ConnectOutToIn( fromComponentIndex, fromOutput, toComponentIndex, toInput );
    }

    return false;
}

bool Circuit::ConnectOutToIn(const std::shared_ptr<Component const>& fromComponent, int fromOutput, int toComponent, int toInput)
{
    int fromComponentIndex;
    if ( p_->FindComponent( fromComponent, fromComponentIndex ) )
    {
        return ConnectOutToIn( fromComponentIndex, fromOutput, toComponent, toInput );
    }

    return false;
}

bool Circuit::ConnectOutToIn(int fromComponent, int fromOutput, const std::shared_ptr<Component const>& toComponent, int toInput)
{
    int toComponentIndex;
    if ( p_->FindComponent( toComponent, toComponentIndex ) )
    {
        return ConnectOutToIn( fromComponent, fromOutput, toComponentIndex, toInput );
    }

    return false;
}

bool Circuit::ConnectOutToIn(int fromComponent, int fromOutput, int toComponent, int toInput)
{
    if ( (size_t)fromComponent >= p_->components_.size() || (size_t)toComponent >= p_->components_.size() )
    {
        return false;
    }

    PauseAutoTick();
    bool result = p_->components_[toComponent]->ConnectInput( p_->components_[fromComponent], fromOutput, toInput );
    ResumeAutoTick();

    return result;
}

void Circuit::DisconnectComponent(const std::shared_ptr<Component const>& component)
{
    int componentIndex;

    if ( p_->FindComponent( component, componentIndex ) )
    {
        DisconnectComponent( componentIndex );
    }
}

void Circuit::DisconnectComponent( int componentIndex )
{
    PauseAutoTick();

    // remove component from _inputComponents and _inputWires
    p_->components_[componentIndex]->DisconnectAllInputs();

    // remove any connections this component has to other components
    for ( auto& component : p_->components_ )
    {
        component->DisconnectInput( p_->components_[componentIndex] );
    }

    ResumeAutoTick();
}

void Circuit::SetBufferCount( int bufferCount )
{
    if ( (size_t)bufferCount != p_->circuitThreads_.size() )
    {
        PauseAutoTick();

        // stop all threads
        for ( auto& circuitThread : p_->circuitThreads_ )
        {
            circuitThread->Stop();
        }

        // resize thread array
        p_->circuitThreads_.resize( bufferCount );

        // initialise and start all threads
        for ( size_t i = 0; i < p_->circuitThreads_.size(); ++i )
        {
            if ( !p_->circuitThreads_[i] )
            {
                p_->circuitThreads_[i] = std::unique_ptr<internal::CircuitThread>( new internal::CircuitThread() );
            }
            p_->circuitThreads_[i]->Start( &p_->components_, i );
        }

        // set all components to the new buffer count
        for ( auto& component : p_->components_ )
        {
            component->SetBufferCount( bufferCount );
        }

        ResumeAutoTick();
    }
}

int Circuit::GetBufferCount() const
{
    return p_->circuitThreads_.size();
}

void Circuit::Tick( Component::TickMode mode )
{
    // process in a single thread if this circuit has no threads
    // =========================================================
    if (p_->circuitThreads_.empty())
    {
        // tick all internal components
        for (auto& component : p_->components_)
        {
            component->Tick( mode );
        }

        // reset all internal components
        for (auto& component : p_->components_)
        {
            component->Reset();
        }
    }
    // process in multiple threads if this circuit has threads
    // =======================================================
    else
    {
        p_->circuitThreads_[p_->currentThreadNo_]->SyncAndResume( mode );  // sync and resume thread x

        p_->currentThreadNo_ = p_->currentThreadNo_ + 1 == (int)p_->circuitThreads_.size() ? 0 : p_->currentThreadNo_ + 1;
    }
}

void Circuit::StartAutoTick( Component::TickMode mode )
{
    if (p_->autoTickThread_.IsStopped())
    {
        p_->autoTickThread_.Start(this, mode);
    }
    else
    {
        ResumeAutoTick();
    }
}

void Circuit::StopAutoTick()
{
    if (!p_->autoTickThread_.IsStopped())
    {
        p_->autoTickThread_.Stop();

        // manually tick until 0
        while ( p_->currentThreadNo_ != 0 )
        {
            Tick( p_->autoTickThread_.Mode() );
        }

        // sync all threads
        for ( auto& circuitThread : p_->circuitThreads_ )
        {
            circuitThread->Sync();
        }
    }
}

void Circuit::PauseAutoTick()
{
    if (p_->autoTickThread_.IsStopped())
    {
        return;
    }

    if (++p_->pauseCount_ == 1 && !p_->autoTickThread_.IsPaused())
    {
        p_->autoTickThread_.Pause();

        // manually tick until 0
        while (p_->currentThreadNo_ != 0)
        {
            Tick(p_->autoTickThread_.Mode());
        }

        // sync all threads
        for (auto& circuitThread : p_->circuitThreads_)
        {
            circuitThread->Sync();
        }
    }
}

void Circuit::ResumeAutoTick()
{
    if (p_->autoTickThread_.IsPaused() && --p_->pauseCount_ == 0)
    {
        p_->autoTickThread_.Resume();
    }
}


bool internal::Circuit::FindComponent(const std::shared_ptr<::Component const>& component, int& returnIndex) const
{
    for (size_t i = 0; i < components_.size(); ++i)
    {
        if (components_[i] == component)
        {
            returnIndex = i;
            return true;
        }
    }

    return false;
}