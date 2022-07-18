#include <unity.h>
#include "../tests/test_pressure_profiler.cpp"
#include "../tests/test_pump.cpp"

int main(int argc, char **argv) {
    UNITY_BEGIN();
    runAllPressureProfilerTests();
    runAllPumpTests();
    UNITY_END();
}
