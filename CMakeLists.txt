cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

########################################################################################################################

project(fty_common_rest
    VERSION 1.0.0
    DESCRIPTION "Provides common RestAPI tools for agents"
)

########################################################################################################################
find_package(fty-cmake PATHS ${CMAKE_BINARY_DIR}/fty-cmake)
########################################################################################################################

etn_target(shared ${PROJECT_NAME}
    PUBLIC_INCLUDE_DIR include
    PUBLIC
        fty_common_rest_audit_log.h
        fty_common_rest.h
        fty_common_rest_helpers.h
        fty_common_rest_sasl.h
        fty_common_rest_tokens.h
        fty_common_rest_utils_web.h
    SOURCES
        src/fty_common_rest_audit_log.cc
        src/fty_common_rest_helpers.cc
        src/fty_common_rest_sasl.cc
        src/fty_common_rest_tokens.cc
        src/fty_common_rest_utils_web.cc
    FLAGS
        -Wno-gnu-zero-variadic-macro-arguments
    USES
        czmq
        cxxtools
        fty_common
        fty_common_db
        fty_common_logging
        fty-utils
        tntnet
        tntdb
        sodium
)

set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

########################################################################################################################

etn_test_target(${PROJECT_NAME}
    SOURCES
        fty_common_rest_utils_web.cc
        main.cpp
    SUBDIR
        test
)

########################################################################################################################

install(FILES fty-session.cfg DESTINATION "/etc/fty")

########################################################################################################################
