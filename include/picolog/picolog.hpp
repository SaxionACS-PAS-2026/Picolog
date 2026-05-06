/**
 * @file picolog.hpp
 * @brief A lightweight logging library for C++ that provides macros for logging function entries/exits
 * @author Dawid Zalewski
 * @copyright Copyright (c) 2026 Dawid Zalewski
 * @version 1.0.0
 * @date 2026-04-30
 * 
 * MIT License
 * 
 * Copyright (c) 2026 Dawid Zalewski
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#ifndef PICOLOG_DISABLE_ALL

#ifndef INCLUDED_PICOLOG_PICOLOG_HPP
#define INCLUDED_PICOLOG_PICOLOG_HPP

// The line below disables clang-tidy checks for this file
// NOLINTBEGIN

// This line disables specific cppcheck warnings for this file
// cppcheck-suppress-begin [functionStatic, useStlAlgorithm, uninitMemberVar]

#include <string_view>
#include <cstdio>
#include <cstdint>
#include <cstdlib>


#if defined(PICO_ON_DEVICE) && PICO_ON_DEVICE
    #include "pico/time.h"
#else
    #include <chrono>
#endif

#define PICOLOG_CAT(a, b) PICOLOG_CAT_IMPL(a, b)
#define PICOLOG_CAT_IMPL(a, b) a##b

#define PICOLOG_NARG(...) PICOLOG_NARG_IMPL(__VA_ARGS__, PICOLOG_RSEQ_N())
#define PICOLOG_NARG_IMPL(...) PICOLOG_ARG_N(__VA_ARGS__)
#define PICOLOG_ARG_N( \
     _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8, \
     _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define PICOLOG_RSEQ_N() \
    16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0

#define PICOLOG_MDISPATCH(MACRO, ...) \
    PICOLOG_MDISPATCH_IMPL(MACRO, PICOLOG_NARG(__VA_ARGS__), __VA_ARGS__)

#define PICOLOG_MDISPATCH_IMPL(MACRO, COUNT, ...) \
    PICOLOG_CAT(MACRO, COUNT)(__VA_ARGS__)

namespace picolog
{
        namespace detail
        {
            enum  struct print_flag_t : unsigned char
            {
                none = 0u,
                ts = 1u << 0u,
                loc = 1u << 1u,
                fun = 1u << 2u,
                ts_loc = ts | loc,
                ts_fun = ts | fun,
                loc_fun = loc | fun,
                all = ts | loc | fun
            };

            constexpr auto to_underlying(print_flag_t flag) -> std::underlying_type_t<print_flag_t>
            {
                return static_cast<std::underlying_type_t<print_flag_t>>(flag);
            }

            constexpr print_flag_t operator|(print_flag_t a, print_flag_t b)
            {
                return static_cast<print_flag_t>(to_underlying(a) | to_underlying(b));
            }

            constexpr print_flag_t operator&(print_flag_t a, print_flag_t b)
            {
                return static_cast<print_flag_t>(to_underlying(a) & to_underlying(b));
            }

            inline print_flag_t& operator|=(print_flag_t& a, print_flag_t b)
            {
                a = a | b;
                return a;
            }

            constexpr bool has_flag(print_flag_t value, print_flag_t flag)
            {
                return to_underlying(value & flag) != 0;
            }


            inline struct config_t 
            {
            public:
                config_t() = default;

            void enable_short_source_paths(std::string_view source_root = __FILE__) 
            { 
                short_source_paths_ = true;

                std::string_view root_{};

                #if defined(PICOLOG_ROOT_SOURCE_DIR)
                    root_ = PICOLOG_ROOT_SOURCE_DIR;
                #endif
                    
                    if (root_.empty()) {
                        root_ = source_root;
                    }
                    // Try to guess the common root from the current file path
                    auto last_sep = root_.find_last_of("/\\");
                    if (last_sep != std::string_view::npos) {
                        root_ = root_.substr(0, last_sep);
                    }

                    source_root_ = root_;

                }
                void disable_short_source_paths() { short_source_paths_ = false; }

                void set_output_stream(std::FILE* stream) { output_stream_ = stream; }

                void set_max_array_elements(std::size_t max_elements) { max_array_elements_ = max_elements; }

                [[nodiscard]] 
                constexpr bool short_source_paths() const { return short_source_paths_; }

                [[nodiscard]] 
                constexpr std::FILE* output_stream() const { return output_stream_; }
                [[nodiscard]] 
                constexpr std::string_view source_root() const { return source_root_; }
                [[nodiscard]] 
                constexpr std::size_t max_array_elements() const { return max_array_elements_; }

                struct print_cfg_t
                {
                    bool print_timestamp = false;
                    bool print_location = true;
                    bool print_function = true;
                };

                [[nodiscard]]
                print_cfg_t print_cfg_func() const 
                { 
                    return 
                    {
                        .print_timestamp = has_flag(print_flags_func_, print_flag_t::ts),
                        .print_location = has_flag(print_flags_func_, print_flag_t::loc),
                        .print_function = has_flag(print_flags_func_, print_flag_t::fun)
                    };
                }
                
                [[nodiscard]]
                print_cfg_t print_cfg_vars() const 
                { 
                    return 
                    {
                        .print_timestamp = has_flag(print_flags_vars_, print_flag_t::ts),
                        .print_location = has_flag(print_flags_vars_, print_flag_t::loc),
                        .print_function = has_flag(print_flags_vars_, print_flag_t::fun)
                    }; 
                }

                void set_print_cfg_func(const print_cfg_t& cfg) 
                { 
                    print_flags_func_ = print_flag_t::none;
                    if (cfg.print_timestamp) { print_flags_func_ |= print_flag_t::ts; }
                    if (cfg.print_location)  { print_flags_func_ |= print_flag_t::loc; }
                    if (cfg.print_function)  { print_flags_func_ |= print_flag_t::fun; }
                }
                
                void set_print_cfg_vars(const print_cfg_t& cfg) 
                { 
                    print_flags_vars_ = print_flag_t::none;
                    if (cfg.print_timestamp) { print_flags_vars_ |= print_flag_t::ts; }
                    if (cfg.print_location)  { print_flags_vars_ |= print_flag_t::loc; }
                    if (cfg.print_function)  { print_flags_vars_ |= print_flag_t::fun; }
                }

                [[nodiscard]]
                print_flag_t print_flags_func() const { return print_flags_func_; }
                
                [[nodiscard]]
                print_flag_t print_flags_vars() const { return print_flags_vars_; }

            private:
                print_flag_t print_flags_func_ = print_flag_t::ts_fun;
                print_flag_t print_flags_vars_ = print_flag_t::loc_fun;

                bool short_source_paths_ = false;
                std::FILE* output_stream_ = stdout;
                std::string_view source_root_{};
                std::size_t max_array_elements_ = 10;
            } config;

        constexpr std::string_view remove_source_root(std::string_view file) 
        {
            if (config.short_source_paths()) 
            {
                auto const& root = config.source_root();
                if (!root.empty() && file.starts_with(root)) 
                {
                    file.remove_prefix(root.size());
                    if (file.starts_with("/") || file.starts_with("\\"))
                    {
                        file.remove_prefix(1);
                    }
                }
            }
            return file;
        }

        template <typename T, typename U = std::remove_cvref_t<T>>
        consteval const char * format_for_var()
        {
            if constexpr (std::is_same_v<U, bool>)                          { return "  %s: %s\n"; }
            else if constexpr (std::is_same_v<U, char>)                     { return "  %s: '%c'\n"; }
            else if constexpr (std::is_same_v<U, signed char>)              { return "  %s: %hhd\n"; }
            else if constexpr (std::is_same_v<U, unsigned char>)            { return "  %s: %hhu\n"; }
            else if constexpr (std::is_same_v<U, int>)                      { return "  %s: %d\n"; }
            else if constexpr (std::is_same_v<U, long>)                     { return "  %s: %ld\n"; }
            else if constexpr (std::is_same_v<U, long long>)                { return "  %s: %lld\n"; }
            else if constexpr (std::is_same_v<U, unsigned int>)             { return "  %s: %u\n"; }
            else if constexpr (std::is_same_v<U, unsigned long>)            { return "  %s: %lu\n"; }
            else if constexpr (std::is_same_v<U, unsigned long long>)       { return "  %s: %llu\n"; }
            else if constexpr (std::is_same_v<U, float>)                    { return "  %s: %f\n"; }
            else if constexpr (std::is_same_v<U, double>)                   { return "  %s: %lf\n"; }
            else if constexpr (std::is_same_v<U, const char*>)              { return "  %s: \"%s\"\n"; }
            else if constexpr (std::is_same_v<U, std::string_view>)         { return "  %s: \"%s\"\n"; }
            else                                                            { return "  %s: %p\n"; } // fallback for pointers and other types
        }

        template <typename T, typename U = std::remove_cvref_t<T>>
        consteval const char * format_for_type()
        {
            if constexpr (std::is_same_v<U, bool>)                      { return "%s"; }
            else if constexpr (std::is_same_v<U, char>)                 { return "'%c'"; }
            else if constexpr (std::is_same_v<U, signed char>)          { return "%hhd"; }
            else if constexpr (std::is_same_v<U, unsigned char>)        { return "%hhu"; }
            else if constexpr (std::is_same_v<U, int>)                  { return "%d"; }
            else if constexpr (std::is_same_v<U, long>)                 { return "%ld"; }
            else if constexpr (std::is_same_v<U, long long>)            { return "%lld"; }
            else if constexpr (std::is_same_v<U, unsigned int>)         { return "%u"; }
            else if constexpr (std::is_same_v<U, unsigned long>)        { return "%lu"; }
            else if constexpr (std::is_same_v<U, unsigned long long>)   { return "%llu"; }
            else if constexpr (std::is_same_v<U, float>)                { return "%f"; }
            else if constexpr (std::is_same_v<U, double>)               { return "%lf"; }
            else if constexpr (std::is_same_v<U, const char*>)          { return "\"%s\""; }
            else if constexpr (std::is_same_v<U, std::string_view>)     { return "\"%s\""; }
            else                                                        { return "%p"; }
        }

        template <class T>
        concept array_like = std::is_array_v<std::remove_cvref_t<T>> || std::is_pointer_v<std::remove_cvref_t<T>>;

        template <class T>
        requires array_like<T>
        using array_element_t =
                std::conditional_t<
                    std::is_array_v<std::remove_cvref_t<T>>,
                    std::remove_extent_t<std::remove_cvref_t<T>>,
                    std::remove_pointer_t<std::remove_cvref_t<T>>
                    >;


        template <typename T>
            requires array_like<T>
        void print_array(std::FILE* stream, const char* expr_str, const T& array, std::size_t n_elems, std::size_t focus_index)
        {

            std::size_t from_index = 0;
            std::size_t to_index = n_elems;

            if (n_elems > detail::config.max_array_elements())
            {
                const std::size_t half = detail::config.max_array_elements() / 2;
                if (focus_index < half) {
                    from_index = 0;
                    to_index = detail::config.max_array_elements();
                } else if (focus_index >= n_elems - half) {
                    from_index = n_elems - detail::config.max_array_elements();
                    to_index = n_elems;
                } else {
                    from_index = focus_index - half;
                    to_index = focus_index + half;
                }
            }

           (void)std::fprintf(stream, "  %s[%zu..%zu]: [", expr_str, from_index, to_index - 1);
            for (auto i = from_index; i < to_index; ++i)
            {
                if (i > from_index) 
                {
                   (void)std::fprintf(stream, ", ");
                }
                if constexpr (std::is_same_v<array_element_t<T>, bool>) 
                {
                   (void)std::fprintf(stream, "%s", array[i] ? "true" : "false");
                }
                else 
                {    
                   (void)std::fprintf(stream, format_for_type<array_element_t<T>>(), array[i]);
                }
            }
           (void)std::fprintf(stream, "]\n");
        }

        // Notice that the return types differ, depending on the host platform.
        #if defined(PICO_ON_DEVICE) && PICO_ON_DEVICE
            inline std::uint32_t current_timestamp_ms() {
                return to_ms_since_boot(get_absolute_time());
            }
        #else
            inline std::uint64_t current_timestamp_ms() {
                return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            }
        #endif


        /// Expression wrapper for logging a single variable.
        template <typename T>
        struct expr
        {
            const char* str;
            T const& value;
        };

        /// Expression wrapper for logging array-like variables.
        template <typename T>
            requires detail::array_like<T>
        struct expr_array
        {
            const char* str;
            T const& arr;
            std::size_t from;
            std::size_t length;
        };

        struct exp_message
        {
            const char* str;
        };

        template <typename T>
            requires detail::array_like<T>
        constexpr expr_array<T> make_expr_array(const char* str, const T& arr, std::size_t from, std::size_t length) {
            return expr_array<T>{str, arr, from, length};
        }

        template <typename T>
            requires detail::array_like<T>
        constexpr expr_array<T> make_expr_array(const char* str, const T& arr, std::size_t length) {
            return expr_array<T>{str, arr, 0, length};
        }

        
        template <typename T>
        void log_var(const expr_array<T>& value)
        {
            detail::print_array(detail::config.output_stream(), value.str, value.arr, value.length, value.from);}

        template <typename T>
        void log_var(const expr<T>& value)
        {
           (void)std::fprintf(detail::config.output_stream(), detail::format_for_var<decltype(value.value)>(), value.str, value.value);
        }

        inline void log_var(const exp_message& message)
        {
           (void)std::fprintf(detail::config.output_stream(), "  %s\n", message.str);
        }

        template <typename T>
        constexpr expr<T> make_expr(const char* str, const T& value) {
            return expr<T>{str, value};
        }

        template <typename T>
        constexpr expr_array<T> make_expr(const char*, expr_array<T> arr)
        {
            return arr;
        }
        constexpr exp_message make_expr(const char*, exp_message message)
        {
            return message;
        }
    } /* namespace detail */


    template <typename... Args>
    void log_enter(const char* file, int line, const char* function, const char* format, Args&&... args)
    {
        switch(detail::config.print_flags_func())
        {
            case detail::print_flag_t::ts:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms]", detail::current_timestamp_ms());
                break;
            case detail::print_flag_t::loc:
               (void)std::fprintf(detail::config.output_stream(), "(at %s:%d)", detail::remove_source_root(file).data(), line);
                break;
            case detail::print_flag_t::fun:
               (void)std::fprintf(detail::config.output_stream(), "ENTER @%s", function);
                break;
            case detail::print_flag_t::ts_loc:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms] (at %s:%d)", detail::current_timestamp_ms(), detail::remove_source_root(file).data(), line);
                break;
            case detail::print_flag_t::ts_fun:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms] ENTER @%s", detail::current_timestamp_ms(), function);
                break;
            case detail::print_flag_t::loc_fun:
               (void)std::fprintf(detail::config.output_stream(), "(at %s:%d) ENTER @%s", detail::remove_source_root(file).data(), line, function);
                break;
            case detail::print_flag_t::all:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms] (at %s:%d) ENTER @%s", detail::current_timestamp_ms(), detail::remove_source_root(file).data(), line, function);
                break;
            default:
                break;
        }


        if (format != nullptr)
        {
            (void)std::fputs(": ", detail::config.output_stream());
        
            #if defined(__GNUC__) || defined(__clang__)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-security"
            #endif

           (void)std::fprintf(detail::config.output_stream(), format, std::forward<Args>(args)...);
        
            #if defined(__GNUC__) || defined(__clang__)
            #pragma GCC diagnostic pop
            #endif
        }

        if (format == nullptr || std::string_view{format}.back() != '\n')
        {
           (void)std::fprintf(detail::config.output_stream(), "\n");
        }
    }

    inline void log_enter(const char* file, int line, const char* function)
    {
        log_enter(file, line, function, nullptr);
    }

    template <typename... Args>
    void log_exit(const char* file, int line, const char* function, const char* format, Args&&... args)
    {
        
        switch(detail::config.print_flags_func())
        {
            case detail::print_flag_t::ts:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms]", detail::current_timestamp_ms());
                break;
            case detail::print_flag_t::loc:
               (void)std::fprintf(detail::config.output_stream(), "(at %s:%d)", detail::remove_source_root(file).data(), line);
                break;
            case detail::print_flag_t::fun:
               (void)std::fprintf(detail::config.output_stream(), "EXIT @%s", function);
                break;
            case detail::print_flag_t::ts_loc:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms] (at %s:%d)", detail::current_timestamp_ms(), detail::remove_source_root(file).data(), line);
                break;
            case detail::print_flag_t::ts_fun:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms] EXIT @%s", detail::current_timestamp_ms(), function);
                break;
            case detail::print_flag_t::loc_fun:
               (void)std::fprintf(detail::config.output_stream(), "(at %s:%d) EXIT @%s", detail::remove_source_root(file).data(), line, function);
                break;
            case detail::print_flag_t::all:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms] (at %s:%d) EXIT @%s", detail::current_timestamp_ms(), detail::remove_source_root(file).data(), line, function);
                break;
            default:
                break;
        }


        if (format != nullptr)
        {
            (void)std::fputs(": ", detail::config.output_stream());
        
            #if defined(__GNUC__) || defined(__clang__)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-security"
            #endif

           (void)std::fprintf(detail::config.output_stream(), format, std::forward<Args>(args)...);
        
            #if defined(__GNUC__) || defined(__clang__)
            #pragma GCC diagnostic pop
            #endif
        
        }

        if (format == nullptr || std::string_view{format}.back() != '\n')
        {
           (void)std::fprintf(detail::config.output_stream(), "\n");
        }
    }

    inline void log_exit(const char* file, int line, const char* function)
    {
        log_exit(file, line, function, nullptr);
    }

    template <typename... Args>
    constexpr void log_vars(const char* file, int line, const char* function, Args&&... args)
    {
        switch(detail::config.print_flags_vars())
        {
            case detail::print_flag_t::ts:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms]", detail::current_timestamp_ms());
                break;
            case detail::print_flag_t::loc:
               (void)std::fprintf(detail::config.output_stream(), "(at %s:%d)", detail::remove_source_root(file).data(), line);
                break;
            case detail::print_flag_t::fun:
               (void)std::fprintf(detail::config.output_stream(), "IN @%s", function);
                break;
            case detail::print_flag_t::ts_loc:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms] (at %s:%d)", detail::current_timestamp_ms(), detail::remove_source_root(file).data(), line);
                break;
            case detail::print_flag_t::ts_fun:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms] IN @%s", detail::current_timestamp_ms(), function);
                break;
            case detail::print_flag_t::loc_fun:
               (void)std::fprintf(detail::config.output_stream(), "(at %s:%d) IN @%s", detail::remove_source_root(file).data(), line, function);
                break;
            case detail::print_flag_t::all:
               (void)std::fprintf(detail::config.output_stream(), "[%lu ms] (at %s:%d) IN @%s", detail::current_timestamp_ms(), detail::remove_source_root(file).data(), line, function);
                break;
            default:
                break;
        }
        if constexpr (sizeof...(args) > 0)
        {
           (void)std::fprintf(detail::config.output_stream(), detail::config.print_flags_vars() == detail::print_flag_t::none ? "{\n" : ":\n{\n");
            (detail::log_var(args), ...);
           (void)std::fprintf(detail::config.output_stream(), "}\n");
        }
    }

}

#define MK_EXPR(expr) picolog::detail::make_expr(#expr, expr)

/// @brief Logs the entry of a function, along with an optional message (which can be a printf-style format string with additional arguments).
/// @param ... Optional printf-style format string and arguments to log with the function entry.
#define LOG_ENTER(...) \
    picolog::log_enter(__FILE__, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__)

/// @brief Logs the exit of a function, along with an optional message (which can be a printf-style format string with additional arguments).
/// @param ... Optional printf-style format string and arguments to log with the function exit.
#define LOG_EXIT(...) \
    picolog::log_exit(__FILE__, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__)

/// @brief Logs the values of one or more variables. Each variable is logged with its name and value.
/// @param ... One or more variables to log.
/// @note To log array-like variables, use the ARRAY() macro. For custom messages, use the MESSAGE() macro.
#define LOG_VARS(...) \
    PICOLOG_MDISPATCH(LOG_VARS_, __VA_ARGS__)

/// @brief Used with LOG_VARS to log array-like variables. Logs a portion of the array starting from a specified index and for a specified length.
/// @param expr The array variable to log (can be a pointer or an array).
/// @param from The starting index of the array portion to log.
/// @param length The number of elements to log from the starting index.
#define ARRAY(expr, from, length) \
    picolog::detail::make_expr_array(#expr, (expr), from, length)

/// @brief Used with LOG_VARS to log custom messages that are not tied to a specific variable. Logs the provided message string.
/// @param str The message string to log.
#define MESSAGE(str) \
    picolog::detail::make_expr(str, picolog::detail::exp_message{str})

// Configuration things

namespace picolog
{
    using print_config = detail::config_t::print_cfg_t;
}

/// @brief Enables short source paths in log output. When enabled, the common root directory of the source files is removed from the file paths in log messages.
#define PICOLOG_ENABLE_SHORT_SOURCE_PATHS() picolog::detail::config.enable_short_source_paths()

/// @brief Disables short source paths in log output. When disabled, full file paths are shown in log messages.
#define PICOLOG_DISABLE_SHORT_SOURCE_PATHS() picolog::detail::config.disable_short_source_paths()

/// @brief Sets the output stream for log messages. By default, log messages are printed to stdout, but this macro allows redirecting them to a different stream (e.g., a file).
/// @param stream A pointer to a FILE stream (e.g., stdout, stderr, or a file stream) where log messages should be printed.
#define PICOLOG_SET_OUTPUT_STREAM(stream) picolog::detail::config.set_output_stream(stream)

/// @brief Sets the maximum number of elements to log for array-like variables. If an array has more elements than this limit, only a portion of the array will be logged.
/// @param n The maximum number of elements to log.
#define PICOLOG_SET_MAX_ARRAY_ELEMENTS(n) picolog::detail::config.set_max_array_elements(n)

/// @brief Sets the print configuration for function entry/exit logs. This configuration determines whether timestamps, source locations, and function names are included in the log messages for function entries and exits.
/// @param ... A print_config struct specifying which components to include in function entry/exit logs
/// Example usage: PICOLOG_SET_PRINT_CFG_FUNC({.print_timestamp = true, .print_location = false, .print_function = true});
#define PICOLOG_SET_PRINT_CFG_FUNC(...) picolog::detail::config.set_print_cfg_func(__VA_ARGS__)

/// @brief Sets the print configuration for variable logs. This configuration determines whether timestamps, source locations, and function names are included in the log messages for variable logs.
/// @param ... A print_config struct specifying which components to include in variable logs
/// Example usage: PICOLOG_SET_PRINT_CFG_VARS({.print_timestamp = false, .print_location = true, .print_function = false});
#define PICOLOG_SET_PRINT_CFG_VARS(...) picolog::detail::config.set_print_cfg_vars(__VA_ARGS__)


/* PRIVATE MACROS*/

#define LOG_VARS_1(exp1) \
    picolog::log_vars(__FILE__, __LINE__, __func__, MK_EXPR(exp1))

#define LOG_VARS_2(exp1, exp2) \
    picolog::log_vars(__FILE__, __LINE__, __func__, MK_EXPR(exp1), MK_EXPR(exp2))

#define LOG_VARS_3(exp1, exp2, exp3) \
    picolog::log_vars(__FILE__, __LINE__, __func__, MK_EXPR(exp1), MK_EXPR(exp2), MK_EXPR(exp3))

#define LOG_VARS_4(exp1, exp2, exp3, exp4) \
    picolog::log_vars(__FILE__, __LINE__, __func__, MK_EXPR(exp1), MK_EXPR(exp2), MK_EXPR(exp3), MK_EXPR(exp4)) 

#define LOG_VARS_5(exp1, exp2, exp3, exp4, exp5) \
    picolog::log_vars(__FILE__, __LINE__, __func__, MK_EXPR(exp1), MK_EXPR(exp2), MK_EXPR(exp3), MK_EXPR(exp4), MK_EXPR(exp5))

#define LOG_VARS_6(exp1, exp2, exp3, exp4, exp5, exp6) \
    picolog::log_vars(__FILE__, __LINE__, __func__, MK_EXPR(exp1), MK_EXPR(exp2), MK_EXPR(exp3), MK_EXPR(exp4), MK_EXPR(exp5), MK_EXPR(exp6))

#define LOG_VARS_7(exp1, exp2, exp3, exp4, exp5, exp6, exp7) \
    picolog::log_vars(__FILE__, __LINE__, __func__, MK_EXPR(exp1), MK_EXPR(exp2), MK_EXPR(exp3), MK_EXPR(exp4), MK_EXPR(exp5), MK_EXPR(exp6), MK_EXPR(exp7))

#define LOG_VARS_8(exp1, exp2, exp3, exp4, exp5, exp6, exp7, exp8) \
    picolog::log_vars(__FILE__, __LINE__, __func__, MK_EXPR(exp1), MK_EXPR(exp2), MK_EXPR(exp3), MK_EXPR(exp4), MK_EXPR(exp5), MK_EXPR(exp6), MK_EXPR(exp7), MK_EXPR(exp8))

#define LOG_VARS_9(exp1, exp2, exp3, exp4, exp5, exp6, exp7, exp8, exp9) \
    picolog::log_vars(__FILE__, __LINE__, __func__, MK_EXPR(exp1), MK_EXPR(exp2), MK_EXPR(exp3), MK_EXPR(exp4), MK_EXPR(exp5), MK_EXPR(exp6), MK_EXPR(exp7), MK_EXPR(exp8), MK_EXPR(exp9))

#define LOG_VARS_10(exp1, exp2, exp3, exp4, exp5, exp6, exp7, exp8, exp9, exp10) \
    picolog::log_vars(__FILE__, __LINE__, __func__, MK_EXPR(exp1), MK_EXPR(exp2), MK_EXPR(exp3), MK_EXPR(exp4), MK_EXPR(exp5), MK_EXPR(exp6), MK_EXPR(exp7), MK_EXPR(exp8), MK_EXPR(exp9), MK_EXPR(exp10))


// cppcheck-suppress-end [functionStatic, useStlAlgorithm, uninitMemberVar]
// NOLINTEND

#endif /* INCLUDED_PICOLOG_PICOLOG_HPP */

#endif /* PICOLOG_DISABLE_ALL */

// The DISABLE_ALL block starts here

#ifdef PICOLOG_DISABLE_ALL

#ifndef INCLUDED_PICOLOG_PICOLOG_HPP
#define INCLUDED_PICOLOG_PICOLOG_HPP

// The line below disables clang-tidy checks for this file
// NOLINTBEGIN

// This line disables specific cppcheck warnings for this file
// cppcheck-suppress-begin [functionStatic, useStlAlgorithm, uninitMemberVar]

namespace picolog
{
}

#undef PICOLOG_CAT
#undef PICOLOG_CAT_IMPL
#undef PICOLOG_NARG
#undef PICOLOG_NARG_IMPL
#undef PICOLOG_ARG_N
#undef PICOLOG_RSEQ_N
#undef PICOLOG_MDISPATCH
#undef PICOLOG_MDISPATCH_IMPL

#undef MK_EXPR

#undef LOG_ENTER
#undef LOG_EXIT
#undef LOG_VARS
#undef ARRAY
#undef MESSAGE
#undef LOG_VARS_1
#undef LOG_VARS_2
#undef LOG_VARS_3
#undef LOG_VARS_4
#undef LOG_VARS_5
#undef LOG_VARS_6
#undef LOG_VARS_7
#undef LOG_VARS_8
#undef LOG_VARS_9
#undef LOG_VARS_10

#undef PICOLOG_ENABLE_SHORT_SOURCE_PATHS
#undef PICOLOG_DISABLE_SHORT_SOURCE_PATHS
#undef PICOLOG_SET_OUTPUT_STREAM
#undef PICOLOG_SET_MAX_ARRAY_ELEMENTS
#undef PICOLOG_SET_PRINT_CFG_FUNC
#undef PICOLOG_SET_PRINT_CFG_VARS

#define LOG_ENTER(...) ((void)0)
#define LOG_EXIT(...) ((void)0)
#define LOG_VARS(...) ((void)0)
#define ARRAY(...) ((void)0)
#define MESSAGE(str) ((void)0)

#define PICOLOG_ENABLE_SHORT_SOURCE_PATHS() ((void)0)
#define PICOLOG_DISABLE_SHORT_SOURCE_PATHS() ((void)0)
#define PICOLOG_SET_OUTPUT_STREAM(stream) ((void)0)
#define PICOLOG_SET_MAX_ARRAY_ELEMENTS(n) ((void)0)
#define PICOLOG_SET_PRINT_CFG_FUNC(...) ((void)0)
#define PICOLOG_SET_PRINT_CFG_VARS(...) ((void)0)

// cppcheck-suppress-end [functionStatic, useStlAlgorithm, uninitMemberVar]
// NOLINTEND

#endif /* INCLUDED_PICOLOG_PICOLOG_HPP */

#endif /* PICOLOG_DISABLE_ALL */
