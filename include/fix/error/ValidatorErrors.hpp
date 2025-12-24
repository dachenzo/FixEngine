#pragma once


namespace Fix::Error {
    enum class Validator {
        WrongFixVersion,
        WrongFieldType,
        MissingField,
        MissingGroupEntry,
        MissingGroupEntryOrWrongOrder,
        MissingGroupSchemaEntry,
        UnsupportedGroupSize,
        UnknownMessageType
    };
}