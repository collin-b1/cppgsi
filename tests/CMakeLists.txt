include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.1
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(Catch2)
include(Catch)

add_executable(cppgsi_tests
        test_models.cpp
        test_dispatcher.cpp
)
target_link_libraries(cppgsi_tests PRIVATE cppgsi Catch2::Catch2WithMain)
catch_discover_tests(cppgsi_tests)
