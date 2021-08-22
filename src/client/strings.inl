#pragma once

#include "../base/config.h"

config_t c_language;

enum language
{
    LANGUAGE_EN,
    LANGUAGE_FR,

    LANGUAGE_COUNT
};

enum string
{
    STRING_QUIT,

    STRING_COUNT
};

const char* strings[LANGUAGE_COUNT][STRING_COUNT] =
    {
        {"Quit"},
        {"Quitter"},
    };

static inline const char* str(uint32_t id)
{
    return strings[c_language.int_value][id];
}