#pragma once
#include <string>


namespace Fix::Error {

    enum class Category {
        Debug,
        Info,
        Warn,
        Error,
    };


    std::string to_string(Error::Category cat) {
  
        switch (cat)
        {
        case Category::Debug:
            return "debug";
        
        case Category::Info:
            return "info";      

        case Category::Warn:
            return "warn";

        case Category::Error:
            return "error";


        default:
            return "unknown error category";
        }
    }


}