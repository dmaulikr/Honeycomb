// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Thread/Thread.h"
#include "Honey/Thread/Lock/Unique.h"

/** \cond */
namespace honey
{

namespace thread { namespace platform
{
    typedef honey::platform::Thread Thread;

    /// Thread local store.  Every thread has its own separate store, can be retrieved statically.
    struct LocalStore
    {
        /// Init for process
        static bool _init;
        static bool init()
        {
            static bool initOnce = false;
            if (initOnce) return true;
            initOnce = true;

            verify(!pthread_key_create(&_key, nullptr));
            return true;
        }

        /// Create thread local store
        static LocalStore& create(Thread& thread)
        {
            LocalStore& local = *new LocalStore();
            local.thread = &thread;
            verify(!pthread_setspecific(_key, &local));
            return local;
        }

        /// Destroy thread local store
        static void destroy()
        {
            delete_(&inst());
            pthread_setspecific(_key, nullptr);
        }

        /// Get thread local store
        static LocalStore& inst()
        {
            //Initializing here solves static order problem
            static bool _ = init();
            mt_unused(_);
            
            LocalStore* local = static_cast<LocalStore*>(pthread_getspecific(_key));
            if (!local)
            {
                //Externally created thread (ex. Main)
                create(*Thread::createExt());
                local = static_cast<LocalStore*>(pthread_getspecific(_key));
            }
            assert(local, "Thread local data not created");
            return *local;
        }

        Thread* thread;

        static pthread_key_t _key;
    };

    pthread_key_t LocalStore::_key;
    bool LocalStore::_init = init();
} }

namespace platform
{

const int Thread::priorityMin = sched_get_priority_min(SCHED_OTHER);
const int Thread::priorityMax = sched_get_priority_max(SCHED_OTHER);

Thread::Thread(bool external, int stackSize) :
    _id(threadIdInvalid),
    _stackSize(stackSize),
    _lock(new honey::Mutex)
{
    if (external)
    {
        _handle = pthread_self();
        _id = pthread_mach_thread_np(_handle);
    }
}

//Subclass calls move assign
Thread::Thread(Thread&&)                        : _id(threadIdInvalid) {}
Thread::~Thread()                               { finalize(); }

void Thread::finalize()
{
    if (_id != threadIdInvalid) pthread_detach(_handle);
}

Thread& Thread::operator=(Thread&& rhs)
{
    finalize();
    _handle = move(rhs._handle);
    _id = move(rhs._id); rhs._id = threadIdInvalid;
    _stackSize = move(rhs._stackSize);
    _lock = move(rhs._lock);
    return *this;
}

Thread& Thread::current()                       { return *LocalStore::inst().thread; }

void Thread::start()
{
    honey::Mutex::Scoped _(*_lock);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, _stackSize);
    verify(!pthread_create(&_handle, &attr, &entry, static_cast<honey::Thread*>(this)));
    _id = pthread_mach_thread_np(_handle);
    pthread_attr_destroy(&attr);
}

void Thread::join()                             { pthread_join(_handle, nullptr); }

void Thread::setPriority(int priority)
{
    sched_param param;
    param.sched_priority = priority;
    verify(!pthread_setschedparam(_handle, SCHED_OTHER, &param));
}
    
int Thread::getPriority() const
{
    int policy;
    sched_param param;
    verify(!pthread_getschedparam(_handle, &policy, &param));
    return param.sched_priority;
}

void* Thread::entry(void* arg)
{
    honey::Thread* thread = static_cast<honey::Thread*>(arg);
    assert(thread);
    {
        //Wait for start() to set thread id
        honey::Mutex::Scoped _(*thread->platform::Thread::_lock);
    }
    LocalStore::create(*thread);
    thread->entry();
    LocalStore::destroy();
    
    return nullptr;
}

int Thread::concurrency_priv()                  { return pthread_getconcurrency(); }

Thread* Thread::createExt()                     { return new honey::Thread(true, 0); }

} }
/** \endcond */



