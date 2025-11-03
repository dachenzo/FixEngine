#pragma once
#include <string>


namespace Fix::Error {

    enum class Layer {
        Transport,
        Fix,
        App, 
        Peripheral
    };

}