#ifndef __EFD_TIMER_H__
#define __EFD_TIMER_H__

#include <chrono>
#include <cstdint>

namespace efd {
    /// \brief Tracks the elapsed time.
    /// One should use the methods \em start and \em stop in order to track the
    /// time of some event.
    ///
    /// Note that if you use the static members, you will be using a \em Timer shared
    /// between everyone (as it is using a static \em Timer).
    class Timer {
        private:
            typedef std::chrono::duration<uint64_t, std::nano> StdDurType;
            typedef std::chrono::steady_clock::time_point StdTimePointType;

            bool mTimed;
            StdTimePointType mStart;
            StdDurType mDuration;

            static Timer _Timer;

        public:
            Timer();

            /// \brief Starts the timer.
            void start();
            /// \brief Stops the timer.
            void stop();

            /// \brief Gets the number of nanoseconds stored in \p mDuration.
            uint64_t getNanoseconds();
            /// \brief Gets the number of microseconds stored in \p mDuration.
            uint64_t getMicroseconds();
            /// \brief Gets the number of milliseconds stored in \p mDuration.
            uint64_t getMilliseconds();
            /// \brief Gets the number of seconds stored in \p mDuration.
            uint64_t getSeconds();

            /// \brief Starts the static timer.
            static void Start();
            /// \brief Stops the timer and returns the elapsed time in the unit
            /// specified in \p T
            template <typename T>
            static uint64_t Stop();
    };
}

template <typename T>
uint64_t efd::Timer::Stop() {
    _Timer.stop();
    return std::chrono::duration_cast<T>(_Timer.mDuration).count();
}

#endif
