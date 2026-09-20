#pragma once

#include <string>
#include <string_view>
#include <system_error>

namespace catchim::core {

enum class ErrorCode {
    Success = 0,
    InvalidParameter,
    NotFound,
    AlreadyExists,
    FileNotFound,
    FileCorrupted,
    UnsupportedFormat,
    DecoderInitFailed,
    EncoderInitFailed,
    OutOfMemory,
    OperationCancelled,
    TrackIncompatible,
    ClipOverlapForbidden,
    InternalError
};

class ErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "catchim";
    }

    std::string message(int ev) const override {
        switch (static_cast<ErrorCode>(ev)) {
            case ErrorCode::Success: return "Success";
            case ErrorCode::InvalidParameter: return "Invalid parameter";
            case ErrorCode::NotFound: return "Resource not found";
            case ErrorCode::AlreadyExists: return "Resource already exists";
            case ErrorCode::FileNotFound: return "File not found";
            case ErrorCode::FileCorrupted: return "File corrupted";
            case ErrorCode::UnsupportedFormat: return "Unsupported format";
            case ErrorCode::DecoderInitFailed: return "Failed to initialize decoder";
            case ErrorCode::EncoderInitFailed: return "Failed to initialize encoder";
            case ErrorCode::OutOfMemory: return "Out of memory";
            case ErrorCode::OperationCancelled: return "Operation cancelled";
            case ErrorCode::TrackIncompatible: return "Clip incompatible with track";
            case ErrorCode::ClipOverlapForbidden: return "Clip overlap forbidden";
            case ErrorCode::InternalError: return "Internal error";
            default: return "Unknown error";
        }
    }
};

inline const ErrorCategory& getCatchimCategory() {
    static ErrorCategory category;
    return category;
}

inline std::error_code make_error_code(ErrorCode e) {
    return {static_cast<int>(e), getCatchimCategory()};
}

template <typename T>
class Result {
public:
    Result(const T& val) : value_(val), error_(ErrorCode::Success) {}
    Result(T&& val) : value_(std::move(val)), error_(ErrorCode::Success) {}
    Result(ErrorCode err, std::string message = "") : error_(err), message_(std::move(message)) {}

    bool ok() const { return error_ == ErrorCode::Success; }
    explicit operator bool() const { return ok(); }

    const T& value() const { return value_; }
    T& value() { return value_; }
    T unwrap() { return std::move(value_); }

    ErrorCode error() const { return error_; }
    const std::string& message() const { return message_; }

private:
    T value_{};
    ErrorCode error_{ErrorCode::Success};
    std::string message_{};
};

template <>
class Result<void> {
public:
    Result() : error_(ErrorCode::Success) {}
    Result(ErrorCode err, std::string message = "") : error_(err), message_(std::move(message)) {}

    bool ok() const { return error_ == ErrorCode::Success; }
    explicit operator bool() const { return ok(); }

    ErrorCode error() const { return error_; }
    const std::string& message() const { return message_; }

    static Result<void> success() { return Result<void>(); }

private:
    ErrorCode error_{ErrorCode::Success};
    std::string message_{};
};

} // namespace catchim::core
