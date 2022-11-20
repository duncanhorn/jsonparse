

add_executable(tests)

set_property(TARGET tests PROPERTY CXX_STANDARD 20)

# strcat, etc. are not deprecated...
target_compile_definitions(tests PRIVATE -D_CRT_SECURE_NO_WARNINGS)

target_sources(tests PRIVATE
    lexer_tests.cpp
    main.cpp
    parser_tests.cpp
    unicode_tests.cpp
    value_tests.cpp)
