#pragma once

#include <string>
#include <optional>

namespace System::Error {
    template <typename T>
    class ResultWrapper {
    private:
        std::optional<T> m_Value;
        std::string m_Error;

        ResultWrapper(std::optional<T> value, std::string error) : m_Value(std::move(value)), m_Error(std::move(error)) {}

        template <typename U>
        friend ResultWrapper<U> success(U);

        template <typename U>
        friend ResultWrapper<U> failure(std::string);

    public:
        [[nodiscard]] explicit operator bool() const noexcept {
            return m_Value.has_value();
        }

        [[nodiscard]] const T& value() const noexcept {
            return m_Value.value();
        }

        [[nodiscard]] const std::string& error() const noexcept {
            return m_Error;
        }
    };

    template <typename T>
    [[nodiscard]] inline ResultWrapper<T> success(T value) {
        return ResultWrapper<T>(std::move(value), "");
    }

    template <typename T>
    [[nodiscard]] inline ResultWrapper<T> failure(std::string error) {
        return ResultWrapper<T>(std::nullopt, std::move(error));
    }
}