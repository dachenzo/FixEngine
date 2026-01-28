#pragma once
#include <fix/error/Severity.hpp>
#include <fix/error/Category.hpp>
#include <fix/error/Layer.hpp>
#include <fix/error/ValidatorErrors.hpp>
#include <string>

namespace Fix::Error {

    inline std::string to_string(Severity sev) {
        switch (sev) {
            case Severity::NA: return "NA";
            case Severity::Low: return "Low";
            case Severity::Moderate: return "Moderate";
            case Severity::High: return "High";
            case Severity::Fatal: return "Fatal";
            default: return "Unknown";
        }
    }

    inline std::string to_string(Category cat) {
        switch (cat) {
            case Category::Debug: return "Debug";
            case Category::Info: return "Info";
            case Category::Warn: return "Warn";
            case Category::Error: return "Error";
            default: return "Unknown";
        }
    }

    inline std::string to_string(Layer layer) {
        switch (layer) {
            case Layer::Transport: return "Transport";
            case Layer::Fix: return "Fix";
            case Layer::App: return "App";
            case Layer::Peripheral: return "Peripheral";
            default: return "Unknown";
        }
    }

    inline std::string to_string(Validator err) {
        switch (err) {
            case Validator::InvalidTagNumber: return "InvalidTagNumber";
            case Validator::RequiredTagMissing: return "RequiredTagMissing";
            case Validator::TagNotDefinedForMessage: return "TagNotDefinedForMessage";
            case Validator::UndefinedTag: return "UndefinedTag";
            case Validator::TagSpecifiedWithoutAValue: return "TagSpecifiedWithoutAValue";
            case Validator::ValueIsIncorrect: return "ValueIsIncorrect";
            case Validator::IncorrectDataFormatForValue: return "IncorrectDataFormatForValue";
            case Validator::DecryptionProblem: return "DecryptionProblem";
            case Validator::SignatureProblem: return "SignatureProblem";
            case Validator::CompIDProblem: return "CompIDProblem";
            case Validator::SendingTimeAccuracyProblem: return "SendingTimeAccuracyProblem";
            case Validator::InvalidMsgType: return "InvalidMsgType";
            case Validator::XMLValidationError: return "XMLValidationError";
            case Validator::TagAppearsMoreThanOnce: return "TagAppearsMoreThanOnce";
            case Validator::TagSpecifiedOutOfRequiredOrder: return "TagSpecifiedOutOfRequiredOrder";
            case Validator::RepeatingGroupFieldsOutOfOrder: return "RepeatingGroupFieldsOutOfOrder";
            case Validator::IncorrectNumInGroupCount: return "IncorrectNumInGroupCount";
            case Validator::NonDataFieldIncludesFieldDelimiter: return "NonDataFieldIncludesFieldDelimiter";
            case Validator::Other: return "Other";
            default: return "Unknown";
        }
    }

    


} // namespace Fix::Error