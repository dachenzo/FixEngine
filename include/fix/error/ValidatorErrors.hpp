#pragma once


namespace Fix::Error {
    enum class Validator {
        InvalidTagNumber,
        RequiredTagMissing,
        TagNotDefinedForMessage,
        UndefinedTag,
        TagSpecifiedWithoutAValue,
        ValueIsIncorrect,
        IncorrectDataFormatForValue,
        DecryptionProblem,
        SignatureProblem,
        CompIDProblem,
        SendingTimeAccuracyProblem,
        InvalidMsgType,
        XMLValidationError,
        TagAppearsMoreThanOnce,
        TagSpecifiedOutOfRequiredOrder,
        RepeatingGroupFieldsOutOfOrder,
        IncorrectNumInGroupCount,
        NonDataFieldIncludesFieldDelimiter,
        Other
    };
}