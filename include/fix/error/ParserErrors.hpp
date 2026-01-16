#pragma once
#include <string>



namespace Fix {

    
    enum class ParseError {
        NoTag,
        MaxTagSize,
        MalformedTag,
        MissingEqualSign,
        MissingValue,
        Failed_checksum,
        Wrong_fix_version,
        Wrong_body_length,
        Duplicate_tag,
        Missing_soh,

    };

        
    


    inline std::string to_string(Fix::ParseError err) {
            switch (err)
            {
            case Fix::ParseError::MaxTagSize: return "MaxTagSize";
            case Fix::ParseError::NoTag: return "NoTag";
            case Fix::ParseError::MalformedTag: return "MalformedTag";
            case Fix::ParseError::MissingEqualSign: return "MissingEqualSign";
            case Fix::ParseError::MissingValue: return "MissingValue";
            default: return "Unknown Error";
                
            }
    }

}