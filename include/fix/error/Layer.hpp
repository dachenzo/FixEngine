#pragma once
#include <string>


namespace Fix::Error {

    enum class Layer {
        Transport,
        Fix,
        App, 
        Peripheral
    };


    std::string to_string(Error::Layer layer) {
  
        switch (layer)
        {
        case Layer::Transport:
            return "transport";
        
        case Layer::Fix:
            return "fix";      

        case Layer::App:
            return "app";

        case Layer::Peripheral:
            return "peripheral";

        default:
            return "unknown error layer";
        }
    }
}