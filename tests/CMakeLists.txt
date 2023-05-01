# Based on https://cliutils.gitlab.io/modern-cmake/chapters/testing/googletest.html
macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_include_directories(${TESTNAME}
    PRIVATE
        ${backup_tools_SOURCE_DIR}/include
    )
    target_link_libraries(${TESTNAME} PRIVATE gtest gmock gtest_main)
    gtest_discover_tests(${TESTNAME}
        WORKING_DIRECTORY ${PROJECT_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()


include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.13.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

#mark_as_advanced(
#    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
#    gmock_build_tests gtest_build_samples gtest_build_tests
#    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
#)

package_add_test(
    test1
    ${backup_tools_SOURCE_DIR}/src/ArgumentParser.cpp
    ${backup_tools_SOURCE_DIR}/src/FileHandler.cpp
    test1.cpp
)
