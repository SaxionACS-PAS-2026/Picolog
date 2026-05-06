add_executable(demo_picolog
    demo_picolog.cpp
)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

target_compile_features(demo_picolog PRIVATE "cxx_std_${CMAKE_CXX_STANDARD}")

target_compile_options(demo_picolog PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(demo_picolog PRIVATE picolog::picolog)

target_compile_options(demo_picolog PRIVATE $<$<CONFIG:Debug>:-Og -ggdb3> $<$<CONFIG:Release>:-O2>)



if (DEFINED PICO_SDK_VERSION_MAJOR)

    target_link_libraries(demo_picolog PRIVATE pico_stdlib)
    pico_enable_stdio_usb(demo_picolog 1)
    pico_add_extra_outputs(demo_picolog)

endif()