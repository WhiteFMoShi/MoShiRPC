#include <iostream>

#include <gtest/gtest.h>

#include "common/timer.hpp"

using namespace std;
using moshi::Timer;

TEST(TimerNormalTest, TimerRunTest) {
    Timer timer;
    
    timer.Start();

    timer.AddMsTask(1000, []() {
        cout << "Hello world" << endl;
    }, true);

    timer.AddMsTask(500, []() {
        std::cout << "This is a Time-Clock!" << std::endl;
    }, true);

    sleep(5);
    timer.Stop();
}

