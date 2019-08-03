
project(tests)
add_executable(tests)

# strcat, etc. are not deprecated...
target_compile_definitions(tests PRIVATE -D_CRT_SECURE_NO_WARNINGS)

target_sources(tests PRIVATE
    lexer_tests.cpp
    main.cpp
    parser_tests.cpp
    unicode_tests.cpp)
