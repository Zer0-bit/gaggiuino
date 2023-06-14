#include <unity.h>
#include "./tests/test_pressure_profiler.cpp"
#include "./tests/test_pump.cpp"
#include "./tests/test_profile_serializer.cpp"
#include "./tests/test_settings_serializer.cpp"
#include "./tests/test_nextion_profile_mapping.cpp"
#include "./tests/test_migrations.cpp"

int main(int argc, char **argv) {
    UNITY_BEGIN();
    runAllMigrationsTests();
    runAllPressureProfilerTests();
    runAllPumpTests();
    runAllProfileSerializerTests();
    runAllSettingsSerializerTests();
    runAllNextionProfileMappingTests();
    UNITY_END();
}
