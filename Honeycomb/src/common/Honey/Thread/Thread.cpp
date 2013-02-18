// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Thread/Thread.h"
#include "Honey/Thread/Condition/Lock.h"
#include "Honey/Thread/Lock/Spin.h"
#include "Honey/Misc/Range.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

namespace thread
{
    /** \cond */
    namespace priv
    {
        InterruptWait::InterruptWait(Condition& cond, Mutex& mutex) :
            thread(Thread::current()),
            enable(thread._interruptEnable)
        {
            if (!enable) return;
            SpinLock::Scoped _(*thread._lock);
            test();
            thread._interruptCond = &cond;
            thread._interruptMutex = &mutex;
        }

        InterruptWait::~InterruptWait()
        {
            if (!enable) return;
            SpinLock::Scoped _(*thread._lock);
            thread._interruptCond = nullptr;
            thread._interruptMutex = nullptr;
            test();
        }

        void InterruptWait::test()
        {
            if (!thread._interruptEx) return;
            current::interruptPoint();
        }
    }
    /** \endcond */

    namespace current
    {
        void sleep(MonoClock::Duration time)        { sleep(MonoClock::now() + time); }

        void sleep(MonoClock::TimePoint time)
        {
            Thread& thread = Thread::current();
            ConditionLock::Scoped _(*thread._sleepCond);
            thread._sleepCond->wait(time);
        }

        bool interruptEnabled()                     { return Thread::current()._interruptEnable; }

        void interruptPoint()
        {
            Thread& thread = Thread::current();
            if (!thread._interruptEnable) return;
            SpinLock::Scoped _(*thread._lock);
            if (!thread._interruptEx) return;
            auto e = thread._interruptEx;
            thread._interruptEx = nullptr;
            e->raise();
        }
    }

    InterruptEnable::InterruptEnable(bool enable) :
        thread(Thread::current()),
        saveState(thread._interruptEnable)
                                                    { thread._interruptEnable = enable; }

    InterruptEnable::~InterruptEnable()             { thread._interruptEnable = saveState; }
}

Thread::Static::Static() :
    storeCount(0),
    storeLock(new SpinLock)
{
}

Thread::Static::~Static()
{
}

Thread::Thread(bool external, int stackSize) :
    Super(external, stackSize),
    _lock(new SpinLock),
    _started(external),
    _done(false),
    _doneCond(new ConditionLock),
    _sleepCond(new ConditionLock),
    _interruptEnable(true),
    _interruptCond(nullptr),
    _interruptMutex(nullptr) {}

Thread::Thread(const Entry& entry, int stackSize) :
    Thread(false, stackSize)
{
    _entry = entry;
}

Thread::Thread(Thread&& rhs) :
    Super(move(rhs)),
    _started(false)
{
    operator=(move(rhs));
}

Thread::~Thread()                                   { finalize(); }

void Thread::finalize()
{
    assert(!_started || _done, "Thread must be joined");
    for (auto& e : _stores) e.fin();
}

Thread& Thread::operator=(Thread&& rhs)
{
    finalize();
    Super::operator=(move(rhs));
    _entry = move(rhs._entry);
    _lock = move(rhs._lock);
    _started = move(rhs._started);
    _done = move(rhs._done);
    _doneCond = move(rhs._doneCond);
    _sleepCond = move(rhs._sleepCond);
    _interruptEnable = move(rhs._interruptEnable);
    _interruptEx = move(rhs._interruptEx);
    _interruptCond = move(rhs._interruptCond);
    _interruptMutex = move(rhs._interruptMutex);
    _stores = move(rhs._stores);
    return *this;
}

void Thread::start()
{
    assert(!_started, "Thread already started");
    _started = true;
    Super::start();
}

void Thread::entry()
{
    _entry();

    _doneCond->lock();
    _done = true;
    _doneCond->broadcast();
    _doneCond->unlock();
}

bool Thread::join(MonoClock::TimePoint time)
{
    //Wait for execution to complete
    ConditionLock::Scoped _(*_doneCond);
    while (!_done && _doneCond->wait(time));

    //Wait for system thread to complete
    if (_done) Super::join();
    return _done;
}

void Thread::interrupt(const Exception& e)
{
    SpinLock::Scoped _(*_lock);
    _interruptEx = &e;
    if (_interruptEnable && _interruptCond)
    {
        _interruptMutex->lock();
        _interruptCond->broadcast();
        _interruptMutex->unlock();
    }
}

bool Thread::interruptRequested() const             { SpinLock::Scoped _(*const_cast<Thread*>(this)->_lock); return _interruptEx; }

Thread::StoreId Thread::allocStore()
{
    Static& ms = getStatic();
    SpinLock::Scoped _(*ms.storeLock);

    if (ms.storeIds.size() == 0)
    {
        //Allocate more ids
        int nextCount = ms.storeCount*2 + 1;
        int newCount = nextCount - ms.storeCount;
        for (int i = 0; i < newCount; ++i)
            ms.storeIds.push_back(ms.storeCount + i);
        ms.storeCount = nextCount;
    }

    StoreId id = ms.storeIds.back();
    ms.storeIds.pop_back();
    return id;
}

void Thread::freeStore(StoreId id)
{
    Static& ms = getStatic();
    if (!ms.storeLock) return; //Static may be destructed at app exit
    SpinLock::Scoped _(*ms.storeLock);
    ++id.reclaim;
    ms.storeIds.push_back(id);
}

}




