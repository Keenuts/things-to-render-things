#include <chrono>

typedef struct scoped_timer
{
    scoped_timer(float& seconds) : seconds(seconds) {
        t0 = std::chrono::steady_clock::now();
    }

    ~scoped_timer() {
        auto t1 = std::chrono::steady_clock::now();
        std::chrono::duration<float> diff = t1 - t0;
        seconds = diff.count();
    }

    float& seconds;
    std::chrono::steady_clock::time_point t0;
} scoped_timer_t;
