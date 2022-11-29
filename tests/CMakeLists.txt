

add_executable(tests)

set_property(TARGET tests PROPERTY CXX_STANDARD 20)

if (MSVC)
    # strcat, etc. are not deprecated...
    target_compile_definitions(tests PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    target_compile_options(tests PRIVATE /W4 /WX)
else()
    target_compile_options(tests PRIVATE -Wall -Werror)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(tests PRIVATE
        -fno-delayed-template-parsing
        -fno-ms-compatibility
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(tests PRIVATE
        /permissive-
    )
else()
    message(FATAL_ERROR "Unknown compiler")
endif()

target_sources(tests PRIVATE
    lexer_tests.cpp
    main.cpp
    parser_tests.cpp
    unicode_tests.cpp
    value_tests.cpp)
