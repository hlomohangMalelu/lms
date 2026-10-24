
cmake_minimum_required(VERSION 3.20)

project(ModeratedLMS
    VERSION 1.0.0
    DESCRIPTION "Moderated Learning Management System"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LMS_BUILD_TESTS "Build LMS tests" ON)
option(LMS_ENABLE_SQLITECPP "Build and link vendored SQLiteCpp" ON)
option(LMS_ENABLE_BCRYPT "Build and link vendored bcrypt" OFF)

add_library(lms_options INTERFACE)

if(MSVC)
    target_compile_options(lms_options INTERFACE /W4 /permissive-)
else()
    target_compile_options(lms_options INTERFACE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    )
endif()

file(GLOB_RECURSE LMS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(lms ${LMS_SOURCES})

target_include_directories(lms PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(lms PRIVATE lms_options)

if(LMS_ENABLE_SQLITECPP)
    set(SQLITECPP_SOURCE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/SQLiteCpp"
        CACHE PATH "SQLiteCpp source directory"
    )

    if(NOT EXISTS "${SQLITECPP_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "SQLiteCpp CMakeLists.txt not found: ${SQLITECPP_SOURCE_DIR}"
        )
    endif()

    set(SQLITECPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SQLITECPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SQLITECPP_INTERNAL_SQLITE OFF CACHE BOOL "" FORCE)
    set(SQLITECPP_FIND_SQLITE ON CACHE BOOL "" FORCE)

    add_subdirectory(
        "${SQLITECPP_SOURCE_DIR}"
        "${CMAKE_BINARY_DIR}/third_party/SQLiteCpp"
        EXCLUDE_FROM_ALL
    )

    if(TARGET SQLiteCpp::SQLiteCpp)
        set(LMS_SQLITECPP_TARGET SQLiteCpp::SQLiteCpp)
    elseif(TARGET SQLiteCpp)
        set(LMS_SQLITECPP_TARGET SQLiteCpp)
    else()
        message(FATAL_ERROR "SQLiteCpp target was not found.")
    endif()

    target_link_libraries(lms PRIVATE ${LMS_SQLITECPP_TARGET})
    target_compile_definitions(lms PRIVATE LMS_ENABLE_SQLITECPP=1)
endif()

if(LMS_ENABLE_BCRYPT)
    set(BCRYPT_SOURCE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bcrypt"
        CACHE PATH "bcrypt source directory"
    )

    set(BCRYPT_TARGET "bcrypt" CACHE STRING
        "CMake target exported by the selected bcrypt implementation"
    )

    if(NOT EXISTS "${BCRYPT_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "bcrypt CMakeLists.txt not found: ${BCRYPT_SOURCE_DIR}"
        )
    endif()

    add_subdirectory(
        "${BCRYPT_SOURCE_DIR}"
        "${CMAKE_BINARY_DIR}/third_party/bcrypt"
        EXCLUDE_FROM_ALL
    )

    if(NOT TARGET "${BCRYPT_TARGET}")
        message(FATAL_ERROR
            "bcrypt target '${BCRYPT_TARGET}' not found."
        )
    endif()

    target_link_libraries(lms PRIVATE "${BCRYPT_TARGET}")
    target_compile_definitions(lms PRIVATE LMS_ENABLE_BCRYPT=1)
endif()

if(LMS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

