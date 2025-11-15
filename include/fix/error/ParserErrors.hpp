#pragma once
#include <string>



namespace Fix::Error {

    
    enum class Parse {
        NoTag,
        MaxTagSize,
        MalformedTag,
        MissingEqualSign,
        MissingValue,
        Failed_checksum,
        Wrong_fix_version,
        Wrong_body_length,
        Duplicate_tag
    };

        
    


    inline std::string to_string(Fix::Error::Parse err) {
            switch (err)
            {
            case Fix::Error::Parse::MaxTagSize: return "MaxTagSize";
            case Fix::Error::Parse::NoTag: return "NoTag";
            case Fix::Error::Parse::MalformedTag: return "MalformedTag";
            case Fix::Error::Parse::MissingEqualSign: return "MissingEqualSign";
            case Fix::Error::Parse::MissingValue: return "MissingValue";
            default: return "Unknown Error";
                
            }
    }

}