// #define PICOLOG_DISABLE_ALL
#include "picolog/picolog.hpp"


void func(int k)
{
    LOG_ENTER("Hello from func!\n");

    auto a = "some random string";

    int arr[] = {1, 2, 3, 4, 5, 6, 7};

    LOG_VARS(k, a, ARRAY(arr, 0, 7));

    LOG_VARS(a, MESSAGE("This is a custom message without variables."), ARRAY(arr, 2, 3));

    LOG_EXIT();
}

int main()
{
    PICOLOG_ENABLE_SHORT_SOURCE_PATHS();
    PICOLOG_SET_MAX_ARRAY_ELEMENTS(20);
    PICOLOG_SET_PRINT_CFG_FUNC( {.print_timestamp = true, .print_location = true, .print_function = true});
    PICOLOG_SET_PRINT_CFG_VARS( {.print_timestamp = true, .print_location = true, .print_function = true});
    PICOLOG_SET_OUTPUT_STREAM(stderr);

    func(42);

    return 0;
}