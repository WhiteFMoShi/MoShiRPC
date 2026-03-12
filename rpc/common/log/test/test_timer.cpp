#include "../src/timer.hpp"
#include <cmath>
#include <iostream>
#include <ctime>
#include <unistd.h>

int main() {
    AdvancedConditionalTimer timer;
    timer.start_min(1, []() {
        std::cout << "Timer callback executed!" << std::endl;
    });

    sleep(5);
}