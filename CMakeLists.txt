cmake_minimum_required(VERSION 3.10)
project(trucksim-gps-telemetry CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(trucksim-gps-telemetry SHARED
    scs-telemetry/src/scs_telemetry.cpp
    scs-telemetry/src/sharedmemory.cpp
    scs-telemetry/src/scs_config_handlers.cpp
    scs-telemetry/src/scs_gameplay_event_handlers.cpp
    scs-telemetry/src/log.cpp
)

target_include_directories(trucksim-gps-telemetry PRIVATE
    scs-telemetry/inc
    scs_sdk/include
)

set_target_properties(trucksim-gps-telemetry PROPERTIES
    PREFIX ""
    POSITION_INDEPENDENT_CODE ON
)

if(UNIX)
    target_link_libraries(trucksim-gps-telemetry PRIVATE rt)
endif()
