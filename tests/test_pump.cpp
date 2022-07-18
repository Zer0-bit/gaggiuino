#include <unity.h>
#include "peripherals/pump.h"

void test_pump_clicks_for_flow_correct_binary_search(void) {
    TEST_ASSERT_EQUAL(30, getClicksPerSecondForFlow(5, 2));
    TEST_ASSERT_EQUAL(50, getClicksPerSecondForFlow(10, 9));
    TEST_ASSERT_EQUAL(0, getClicksPerSecondForFlow(0, 0));
}

void runAllPumpTests() {
    RUN_TEST(test_pump_clicks_for_flow_correct_binary_search);
}
