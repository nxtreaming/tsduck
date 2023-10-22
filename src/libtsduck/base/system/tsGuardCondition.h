//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2023, Thierry Lelegard
// BSD-2-Clause license, see LICENSE.txt file or https://tsduck.io/license
//
//----------------------------------------------------------------------------
//!
//!  @file
//!  Automatic guard class for synchronization condition (ts::Condition).
//!
//----------------------------------------------------------------------------

#pragma once
#include "tsMutex.h"
#include "tsCondition.h"
#include "tsException.h"

namespace ts {
    //!
    //! Automatic guard class for synchronization condition (ts::Condition).
    //! @ingroup thread
    //!
    //! This class implements the @e guard design pattern for conditions.
    //! See ts::GuardMutex for other usages of the guard design pattern.
    //!
    //! Example (writer thread):
    //! @code
    //! Mutex mutex;
    //! Condition condition;
    //! ...
    //! {
    //!     GuardCondition guard(mutex, condition); // mutex acquired
    //!     ...
    //!     // do some modification on shared data
    //!     ...
    //!     guard.signal(); // notify other threads
    //!     ...
    //! } // mutex released
    //! @endcode
    //!
    //! Example (reader thread):
    //! @code
    //! // use same  mutex and condition
    //! ...
    //! {
    //!     GuardCondition guard(mutex, condition); // mutex acquired
    //!     ...
    //!     while (!expected_shared_data_state) {
    //!         guard.waitCondition();
    //!     }
    //!     ...
    //!     // use shared data
    //!     ...
    //! } // mutex released
    //! @endcode
    //!
    class TSDUCKDLL GuardCondition
    {
        TS_NOBUILD_NOCOPY(GuardCondition);
    public:
        //!
        //! Fatal low-level condition guard error.
        //!
        TS_DECLARE_EXCEPTION(GuardConditionError);

        //!
        //! Constructor, automatically acquire the mutex with a timeout.
        //!
        //! The user has to invoke isLocked() to check that the mutex was actually acquired
        //! before the timeout expired.
        //!
        //! @param [in,out] mutex A reference on the mutex object to acquire.
        //! @param [in,out] condition A reference on the condition to wait or signal.
        //! @param [in] timeout Maximum number of milliseconds to wait for the mutex.
        //! @exception ts::GuardCondition::GuardConditionError Thrown whenever an error occurs
        //! during the acquisition of the mutex. Exceeding the timeout is not
        //! error, the object is successfully constructed but isLocked() will
        //! return false.
        //!
        GuardCondition(Mutex& mutex, Condition& condition, MilliSecond timeout = Infinite);

        //!
        //! Destructor, automatically release the mutex.
        //!
        virtual ~GuardCondition();

        //!
        //! Check if the mutex was actually locked.
        //!
        //! This method is useful only with the object was constructed with a
        //! non-infinite timeout.
        //!
        //! @return True if the mutex was successfully acquired and false if
        //! the timeout expired.
        //!
        bool isLocked() const
        {
            return _is_locked;
        }

        //!
        //! Signal the condition.
        //!
        //! If more than one thread wait for the condition, at least one
        //! is awaken. It is then the responsibility of the awaken threads
        //! to check that the expected situation actually exists.
        //!
        //! @exception ts::GuardCondition::GuardConditionError Thrown whenever an error occurs
        //! or if the mutex was not locked (the constructor with timeout
        //! was used and the timeout expired before the mutex was acquired).
        //!
        void signal();

        //!
        //! Wait for the condition to be signaled with a timeout.
        //!
        //! The mutex is automatically released while waiting and then automatically
        //! re-acquired before returning.
        //!
        //! @param [in] timeout Maximum number of milliseconds to wait for the mutex.
        //! @return True when the condition was signaled, false if the timeout
        //! expired before the condition was signaled.
        //! @exception ts::GuardCondition::GuardConditionError Thrown whenever an error occurs
        //! or if the mutex was not locked (the constructor with timeout
        //! was used and the timeout expired before the mutex was acquired).
        //!
        bool waitCondition(MilliSecond timeout = Infinite);

    private:
        Mutex&     _mutex;
        Condition& _condition;
        bool       _is_locked = false;
    };
}
