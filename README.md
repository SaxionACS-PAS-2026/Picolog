# Picolog - a simple C++ logging library (for RPi Pico)

This is a simple logging library for C++ that is designed to be used on the Raspberry Pi Pico microcontroller. It provides a set of macros for logging messages, variables, and function entries/exits, with support for printf-style formatting and array logging.

The library is header-only and can be easily included in your C++ projects. It is designed to be somewhat lightweight and efficient, making it suitable for use in embedded systems like the Raspberry Pi Pico.

The library supports the following hardware:

- Raspberry Pi Pico with RPi Pico SDK (bare-metal)
- Any C++20 compatible platform with a suitable output stream (e.g., std::cout, file streams, etc.) and `<chrono>` support.

## Features

- Logging function entries and exits with optional messages.
- Logging variable values with their names.
- Logging array contents with specified ranges.
- Custom messages that are not tied to specific variables.
- Configurable output stream and timestamping.

## Usage

To use the library, simply include the `picolog.hpp` header in your C++ source files and use the provided macros to log messages and variables. For example:

```cpp
#include "picolog/picolog.hpp"

void myFunction(int x) {
    LOG_ENTER("Entering myFunction with x=%d", x);
    
    int y = x * 2;
    LOG_VARS(x, y);
    
    LOG_EXIT();
}
```

## Logging macros

The following logging macros are available:

- `LOG_ENTER(...)`: Logs the entry of a function, along with an optional message (which can be a printf-style format string with additional arguments).
- `LOG_EXIT(...)`: Logs the exit of a function, along with an optional message (which can be a printf-style format string with additional arguments).
- `LOG_VARS(...)`: Logs the values of one or more variables. Each variable is logged with its name and value. To log array-like variables, use the `ARRAY()` macro. For custom messages, use the `MESSAGE()` macro.
- `ARRAY(expr, from, length)`: Used with `LOG_VARS` to log array-like variables. Logs a portion of the array starting from a specified index and for a specified length.
- `MESSAGE(str)`: Used with `LOG_VARS` to log custom messages that are not tied to a specific variable. Logs the provided message string.

### Example usage of logging macros:

```cpp
#include "picolog/picolog.hpp"

void exampleFunction() {
    LOG_ENTER("Starting exampleFunction");

    int a = 5;
    int b = 10;
    int arr[5] = {1, 2, 3, 4, 5};
    
    LOG_VARS(a, b, ARRAY(arr, 0, 5));

    LOG_VARS(MESSAGE("This is a custom message"), a + b);

    LOG_EXIT("Exiting exampleFunction");
}
```

This will produce a log similar to the following (the actual format may vary based on the configuration and the runtime platform):

```
[2026-05-05 11:34:56] (at example.cpp:4): ENTER @exampleFunction(): Starting exampleFunction
[2026-05-05 11:34:56] (at example.cpp:10): IN @exampleFunction(): a=5, b=10, arr[0..4]={1, 2, 3, 4, 5}
[2026-05-05 11:34:56] (at example.cpp:12): IN @exampleFunction(): This is a custom message, a + b=15
[2026-05-05 11:34:56] (at example.cpp:14): EXIT @exampleFunction(): Exiting exampleFunction
```

## Configuration macros

The following macros can be used to configure the behavior of the logging library:

- `PICOLOG_ENABLE_SHORT_SOURCE_PATHS()`: Enables short source paths in log messages, which can make the logs more concise by showing only the filename instead of the full path. By default, it is set to `false`.

- `PICOLOG_DISABLE_SHORT_SOURCE_PATHS()`: Disables short source paths in log messages, showing the full path to the source file. By default, it is set to `false` (disabled).
  
- `PICOLOG_SET_OUTPUT_STREAM(FILE* stream)`: Defines the output stream to which log messages will be written. By default, it is set to `stdout`.

- `PICOLOG_SET_MAX_ARRAY_ELEMENTS(n_elems)`: Defines the maximum number of elements to display when logging array contents. By default, it is set to 10.

- `PICOLOG_SET_PRINT_CFG_FUNC(cfg)`: Sets the print configuration for function entry/exit logs. This can be used to customize the format of these logs.
- 
- `PICOLOG_SET_PRINT_CFG_VARS(cfg)`: Sets the print configuration for variable logs. This can be used to customize the format of variable logs.

The print configuration (`cfg`) is a structure with three members:

- `print_timestamp`: A boolean that indicates whether to include a timestamp in the log messages.
- `print_location`: A boolean that indicates whether to include the source file and line number in the log messages.
- `print_function`: A boolean that indicates whether to include the function name in the log messages.

For instance, to disable timestamps and enable source location for function entry/exit logs, you can use:

```c++
PICOLOG_SET_PRINT_CFG_FUNC({.print_timestamp = false, .print_location = true, .print_function = true});
```

## Disabling logging

To disable all logging at compile time, you can define the `PICOLOG_DISABLE_ALL` macro before including the `picolog.hpp` header. This will effectively remove all logging calls from the compiled code for the current source file, resulting in no runtime overhead for logging.

```cpp
#define PICOLOG_DISABLE_ALL
#include "picolog.hpp"

// Rest of your code here
```

## License

This library is licensed under the MIT License. See the LICENSE file for more details.