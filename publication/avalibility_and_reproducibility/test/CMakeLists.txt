# Test Patas: ----------------------------------------------------------------------------------------------------------
add_executable(test_patas test_patas.cpp)
target_link_libraries(test_patas PRIVATE gtest_main)
gtest_discover_tests(test_patas)

# Test CHIMP128: -------------------------------------------------------------------------------------------------------
add_executable(test_chimp128 test_chimp128.cpp)
target_link_libraries(test_chimp128 PRIVATE gtest_main)
gtest_discover_tests(test_chimp128)

# Test CHIMP: ----------------------------------------------------------------------------------------------------------
add_executable(test_chimp test_chimp.cpp)
target_link_libraries(test_chimp PRIVATE gtest_main)
gtest_discover_tests(test_chimp)

# Test GORILLAS: ----------------------------------------------------------------------------------------------------------
add_executable(test_gorillas test_gorillas.cpp)
target_link_libraries(test_gorillas PRIVATE gtest_main)
gtest_discover_tests(test_gorillas)

