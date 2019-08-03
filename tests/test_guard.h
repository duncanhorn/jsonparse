#pragma once

#include <cstdio>

struct test_guard
{
    const char* name;
    ~test_guard()
    {
        if (name)
        {
            std::printf("ERROR: Failed test '%s'\n\n", name);
        }
    }

    int success() noexcept
    {
        name = nullptr;
        return 0;
    }
};
