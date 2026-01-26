#pragma once
#include <cstdint>


namespace Fix::Error {
    enum class Validator: uint8_t {
        InvalidTagNumber = 0,
        RequiredTagMissing = 1,
        TagNotDefinedForMessage = 2,
        UndefinedTag = 3,
        TagSpecifiedWithoutAValue = 4,
        ValueIsIncorrect = 5,
        IncorrectDataFormatForValue = 6,
        DecryptionProblem = 7,
        SignatureProblem = 8,
        CompIDProblem = 9,
        SendingTimeAccuracyProblem = 10,
        InvalidMsgType = 11,
        XMLValidationError = 12,
        TagAppearsMoreThanOnce = 13,
        TagSpecifiedOutOfRequiredOrder = 14,
        RepeatingGroupFieldsOutOfOrder = 15,
        IncorrectNumInGroupCount = 16,
        NonDataFieldIncludesFieldDelimiter = 17,
        Other = 99
    };
}