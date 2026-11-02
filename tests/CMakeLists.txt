
# Use the GoogleTest copy already included with SQLiteCpp so builds do not
# require network access. This is particularly useful for offline school labs.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_build_tests OFF CACHE BOOL "" FORCE)
set(gtest_build_samples OFF CACHE BOOL "" FORCE)
set(LMS_GOOGLETEST_DIR "${PROJECT_SOURCE_DIR}/third_party/SQLiteCpp/googletest" CACHE PATH
    "Path to bundled GoogleTest sources")
if(NOT EXISTS "${LMS_GOOGLETEST_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Bundled GoogleTest not found: ${LMS_GOOGLETEST_DIR}")
endif()
add_subdirectory("${LMS_GOOGLETEST_DIR}" "${CMAKE_BINARY_DIR}/third_party/googletest" EXCLUDE_FROM_ALL)

file(GLOB_RECURSE LMS_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/integration/*.cpp"
)

file(GLOB_RECURSE LMS_DOMAIN_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/domain/*.cpp"
)

list(APPEND LMS_TEST_SOURCES
    "${PROJECT_SOURCE_DIR}/src/repository/submission/SQLiteSubmissionRepository.cpp"
    "${PROJECT_SOURCE_DIR}/src/repository/audit_log/SQLiteAuditLogRepository.cpp"
    "${PROJECT_SOURCE_DIR}/src/repository/mark_record/SQLiteMarkRecordRepository.cpp"
    "${PROJECT_SOURCE_DIR}/src/service/MarkingService.cpp"
    "${PROJECT_SOURCE_DIR}/src/service/ModerationService.cpp"
    "${PROJECT_SOURCE_DIR}/src/service/AuthService.cpp"
    "${PROJECT_SOURCE_DIR}/src/service/AdminService.cpp"
    "${PROJECT_SOURCE_DIR}/src/service/SubmissionService.cpp"
    "${PROJECT_SOURCE_DIR}/src/security/SQLiteAuthorizationPolicy.cpp"
    "${PROJECT_SOURCE_DIR}/src/security/BcryptPasswordHasher.cpp"
)

add_executable(lms_tests
    ${LMS_TEST_SOURCES}
    ${LMS_DOMAIN_SOURCES}
)

target_compile_definitions(lms_tests PRIVATE
    LMS_PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

target_include_directories(lms_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
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

# Black-box console/system scenarios run against disposable SQLite databases.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(
    NAME ConsoleWorkflowSystemTest
    COMMAND ${Python3_EXECUTABLE}
        "${CMAKE_CURRENT_SOURCE_DIR}/system/ConsoleWorkflowTest.py"
        "$<TARGET_FILE:lms>"
        "${PROJECT_SOURCE_DIR}"
)
