# Testing Ledger

This document describes how to build and run the comprehensive unit tests for Ledger using the doctest framework.

## Test Framework

Ledger uses [doctest](https://github.com/doctest/doctest) as its testing framework. Doctest is a fast, modern, and feature-rich testing framework for C++17 and later.

**Test coverage includes:**
- LogLevel enumerations and string conversions
- StreamSink operations and output
- RotatingFileLogger creation and rotation
- AsyncLogSink queueing and dropping policies
- LogContext storage and RAII behavior
- Default and JSON log formatters
- Logger basic functionality (info, warning, error, fatal)
- Structured logging with key-value pairs
- Thread-safe concurrent logging
- File logging configuration
- Custom formatters and configurations
- Category loggers and registries
- Edge cases and special characters handling
- Convenience logging macros

## Prerequisites

- **C++17 or later** compiler
- **CMake 3.15** or later
- **doctest** (automatically downloaded during build)

## Building Tests

### Using CMake (Recommended)

```bash
# Create build directory
mkdir build
cd build

# Configure the project
cmake ..

# Build tests
cmake --build .

# Or on Unix-like systems
make
```

### Manual Compilation

If you prefer to compile manually without CMake:

```bash
# Download doctest header
curl -o doctest.h https://github.com/doctest/doctest/releases/download/v2.4.11/doctest.h

# Compile tests
g++ -std=c++17 -I. tests.cpp -o ledger_tests

# Or with Clang
clang++ -std=c++17 -I. tests.cpp -o ledger_tests
```

## Running Tests

### Using CMake

```bash
# From build directory
ctest

# For verbose output
ctest --verbose

# Or run tests directly
./ledger_tests
```

### Using CMake with verbose test output

```bash
cmake --build . --target test -- VERBOSE=1
```

### Running specific test suites

```bash
# Run only LogLevel tests
./ledger_tests -ts LogLevel

# Run only StreamSink tests
./ledger_tests -ts StreamSink

# Run only thread safety tests
./ledger_tests -ts "Logger - Thread Safety"
```

### Filtering test cases

```bash
# Run tests matching a pattern
./ledger_tests "*LogContext*"

# Run tests by index range
./ledger_tests -i "<test_index_range>"

# List all test cases without running
./ledger_tests -lc
```

## Test Suite Organization

### Core Components
- **LogLevel**: Log level enumeration and string conversion
- **StreamSink**: Stream output operations
- **RotatingFileLogger**: File logging with rotation
- **AsyncLogSink**: Asynchronous logging with drop policies

### Formatting
- **DefaultLogFormatter**: Standard text formatting with configurable timestamps
- **JSONLogFormatter**: Machine-parseable JSON formatting

### Logger Functionality
- **Logger - Basic Functionality**: Info, warning, error, and fatal logging
- **Logger - Structured Logging**: Key-value pairs and context inclusion
- **Logger - Thread Safety**: Concurrent logging from multiple threads
- **Logger - File Logging**: File output and rotation
- **Logger - Formatters**: Custom formatter integration
- **Logger - Configuration**: Fluent configuration API and builders
- **Logger - Category Loggers**: Named loggers with individual configurations

### Utilities
- **LogContext**: Thread-local context storage and RAII semantics
- **Macros**: Convenience logging macros (LOG_INFO, LOG_WARNING, etc.)
- **Edge Cases**: Empty messages, long messages, special characters

## Doctest Command-Line Options

```bash
./ledger_tests [options]

Common options:
  -h, --help              Show help message
  -l, --list-tests        List all test cases
  -lc                     List all test cases (case names)
  -ls                     List all test suites
  -ts <suite_name>        Run only tests from suite
  -tc <test_name>         Run only test case with given name
  -v, --verbose           Verbose output
  -s, --silent            Silent output
  -a, --abort-after <num> Abort after N failures
  -e, --exit              Exit after first failure
  --timeout <seconds>     Timeout for each test
  -nc, --no-colors        Disable colored output
```

## Test Development Guidelines

When adding new tests:

1. **Use descriptive test names**: Tests should clearly describe what they're testing
2. **Group by functionality**: Use TEST_SUITE for logical grouping
3. **Use CHECK macros**: For simple assertions
4. **Clean up resources**: Use RAII or explicit cleanup for file handles
5. **Avoid timing dependencies**: Tests should not rely on timing unless specifically testing time-based features
6. **Test thread safety**: Include multi-threaded scenarios where appropriate
7. **Handle edge cases**: Test empty inputs, very large inputs, special characters, etc.

## Continuous Integration

For CI/CD pipelines, use:

```bash
# Run tests with JUnit-style output
./ledger_tests --reporters=xml --out=test-results.xml

# Run with timeout
./ledger_tests --timeout=10
```

## Performance Considerations

- Tests use multiple threads to verify thread safety
- File I/O tests use temporary files (automatically cleaned up)
- Async tests include flush() calls to ensure deterministic behavior
- AsyncLogSink tests verify drop policies with full queues

## Troubleshooting

### Tests Won't Build
- Ensure C++17 support: `g++ --version` or `clang++ --version`
- Verify doctest is downloaded: Check for `doctest.h` in build directory
- Clear and rebuild: `rm -rf build && mkdir build && cd build && cmake ..`

### Tests Crash or Hang
- Check for circular dependencies in logging configuration
- Ensure proper thread cleanup (join all threads before exit)
- Verify file permissions for temporary test files

### Flaky Tests
- Thread safety tests may occasionally fail if timing is too aggressive
- Increase timeout values in CMakeLists.txt if running on slow systems
- Run problematic tests multiple times to verify they're not intermittent

## Integration with Ledger

To use Ledger in your project with these tests as reference:

1. Copy `ledger.h` to your project
2. Include in your source: `#include "ledger.h"`
3. Link against any required platform libraries (standard C++ library is sufficient)
4. Compile with C++17 or later

## Resources

- **Doctest Documentation**: https://github.com/doctest/doctest/blob/master/doc/markdown/readme.md
- **Google C++ Testing Best Practices**: https://google.github.io/styleguide/cppguide.html#Comments_and_Documentation
- **Ledger Repository**: https://github.com/Interlaced-Pixel/Ledger
