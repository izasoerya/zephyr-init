cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky)

target_sources(app PRIVATE
    src/main.cpp
    src/components/led_component.cpp
)

# Add subdirectories as include paths if you use relative header paths
target_include_directories(app PRIVATE
    src/
    src/components
    src/services
)