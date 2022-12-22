#include <unity.h>
#include "../tests/test_pressure_profiler.cpp"
#include "../tests/test_pump.cpp"
#include "../tests/test_esp_stm_comms.cpp"

int main(int argc, char **argv) {
    UNITY_BEGIN();
    runAllPressureProfilerTests();
    runAllPumpTests();
    r
    UNITY_END();
}
