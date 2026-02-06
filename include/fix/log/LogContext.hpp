#pragma once

#include <fix/error/Category.hpp>
#include <fix/error/Layer.hpp>
#include <fix/error/Severity.hpp>

namespace Fix::Log
{
    struct Context {
        Fix::Error::Layer layer;
        Fix::Error::Category cat;
        Fix::Error::Severity sev;  
    };
} // namespace Fix::Log