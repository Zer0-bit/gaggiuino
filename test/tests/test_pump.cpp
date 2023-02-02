#include "../src/peripherals/pump.cpp"
#include "mock.h"
#include "utils/test-utils.h"

void test_pump_clicks_for_flow_correct_binary_search(void) {
    TEST_ASSERT_EQUAL_FLOAT_ACCURACY(31, getClicksPerSecondForFlow(5, 2), 0);
    TEST_ASSERT_EQUAL_FLOAT_ACCURACY(50, getClicksPerSecondForFlow(10, 9), 0);
    TEST_ASSERT_EQUAL_FLOAT_ACCURACY(0, getClicksPerSecondForFlow(0, 0), 0);
}

void test_pump_get_flow(void) {
    TEST_ASSERT_EQUAL_FLOAT_ACCURACY(5.f, getPumpFlow(getClicksPerSecondForFlow(5, 2), 2), 1);
    TEST_ASSERT_EQUAL_FLOAT_ACCURACY(4.5f, getPumpFlow(getClicksPerSecondForFlow(4.5, 5), 5), 1);
}

void runAllPumpTests() {
    RUN_TEST(test_pump_clicks_for_flow_correct_binary_search);
    RUN_TEST(test_pump_get_flow);
}
