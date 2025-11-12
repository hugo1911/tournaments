project(tournament_consumer_tests)

set(TEST_SOURCES
        MatchScoreUpdateListenerTest.cpp
)

set(SOURCES ${TEST_SOURCES})
include_directories(../include)
include_directories(../../tournament_common/include)

find_package(GTest CONFIG REQUIRED)

add_executable(${PROJECT_NAME}_runner
    ${TEST_SOURCES}
)

target_link_libraries(${PROJECT_NAME}_runner PRIVATE
        tournament_common
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
        nlohmann_json::nlohmann_json)

add_test(AllConsumerTestsInMain ${PROJECT_NAME}_runner)
