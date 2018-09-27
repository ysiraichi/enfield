#include "enfield/Support/Timer.h"
#include "enfield/Support/Defs.h"


efd::Timer efd::Timer::_Timer = efd::Timer();

efd::Timer::Timer() : mTimed(false) {
}

void efd::Timer::start() {
    mTimed = false;
    mStart = std::chrono::steady_clock::now();
}

void efd::Timer::stop() {
    auto end = std::chrono::steady_clock::now();
    mDuration = std::chrono::duration_cast<StdDurType>(end - mStart);
    mTimed = true;
}

uint64_t efd::Timer::getNanoseconds() {
    EfdAbortIf(!mTimed, "Trying to get elapsed time from dataless timer.");
    return mDuration.count();
}

uint64_t efd::Timer::getMicroseconds() {
    EfdAbortIf(!mTimed, "Trying to get elapsed time from dataless timer.");
    return std::chrono::duration_cast<std::chrono::microseconds>(mDuration).count();
}

uint64_t efd::Timer::getMilliseconds() {
    EfdAbortIf(!mTimed, "Trying to get elapsed time from dataless timer.");
    return std::chrono::duration_cast<std::chrono::milliseconds>(mDuration).count();
}

uint64_t efd::Timer::getSeconds() {
    EfdAbortIf(!mTimed, "Trying to get elapsed time from dataless timer.");
    return std::chrono::duration_cast<std::chrono::seconds>(mDuration).count();
}

void efd::Timer::Start() {
    _Timer.start();
}
