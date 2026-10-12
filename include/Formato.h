#pragma once
#include <cstdio>
#include <string>

// Formata um double de forma compacta (notacao cientifica quando precisa).
inline std::string fmtSci(double v, int sig = 3)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.*g", sig, v);
    return std::string(buf);
}
