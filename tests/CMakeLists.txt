
include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE LMS_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp"
)

file(GLOB_RECURSE LMS_DOMAIN_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/domain/*.cpp"
)

list(APPEND LMS_TEST_SOURCES
    "${PROJECT_SOURCE_DIR}/src/repository/submission/SQLiteSubmissionRepository.cpp"
    "${PROJECT_SOURCE_DIR}/src/repository/submission/SQLiteSubmissionRepository.cpp"
    "${PROJECT_SOURCE_DIR}/src/repository/audit_log/SQLiteAuditLogRepository.cpp"
)

add_executable(lms_tests
    ${LMS_TEST_SOURCES}
    ${LMS_DOMAIN_SOURCES}
)

target_include_directories(lms_tests PRIVATE
    "${PROJECT_SOURCE_DIR}/src"
)


target_link_libraries(lms_tests
    PRIVATE
        GTest::gtest_main
        bcrypt
        lms_options
        SQLiteCpp
)


include(GoogleTest)
gtest_discover_tests(lms_tests)
