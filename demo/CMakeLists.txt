add_executable(demo_picolog
    demo_picolog.cpp
)

target_compile_features(demo_picolog PRIVATE "cxx_std_${CMAKE_CXX_STANDARD}")

target_compile_options(demo_picolog PRIVATE -Wall -Wextra -pedantic)
target_link_libraries(demo_picolog PRIVATE picolog::picolog)

target_compile_options(demo_picolog PRIVATE $<$<CONFIG:Debug>:-Og -ggdb3> $<$<CONFIG:Release>:-O2>)


target_link_libraries(demo_picolog PRIVATE pico_stdlib pico_float)

pico_enable_stdio_usb(demo_picolog 1)
# pico_enable_stdio_uart(demo_picolog 0)
pico_set_float_implementation(demo_picolog pico)

pico_add_extra_outputs(demo_picolog)