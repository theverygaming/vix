#pragma once
#include <vix/panic.h>

// TODO: printable strings for status codes

namespace status {
    enum class StatusCode : int {
        OK = 0,
        UNKNOWN_ERR = -1,
        OOM_ERROR = -2,
    };

    template <typename StatusCodeT = StatusCode> class [[nodiscard]] Status {
    public:
        Status(const StatusCodeT &scode) {
            _code = scode;
        }

        Status(const Status &status) {
            _code = status._code;
        }

        [[nodiscard]] inline StatusCodeT code() const {
            return _code;
        }

        [[nodiscard]] inline bool ok() const {
            return _code == StatusCodeT::OK;
        }

        [[nodiscard]] const Status<StatusCodeT> &status() {
            return *this;
        }

    private:
        StatusCodeT _code;
    };

    template <typename T, typename StatusCodeT = StatusCode> class [[nodiscard]] StatusOr {
    public:
        StatusOr(const T &val) : _val(val), _status(StatusCodeT::OK) {}

        StatusOr(const StatusCodeT &scode) : _status(scode) {}

        StatusOr(const Status<StatusCodeT> &status) : _status(status) {}

        [[nodiscard]] const Status<StatusCodeT> &status() {
            return _status;
        }

        T &value() {
            return _val;
        }

    private:
        T _val;
        Status<StatusCodeT> _status;
    };
}

#define RETURN_IF_ERROR(expr)             \
    do {                                  \
        auto _tmp_status = (expr);        \
        if (!_tmp_status.status().ok()) { \
            return _tmp_status.status();  \
        }                                 \
    } while (0)

#define PANIC_IF_ERROR(expr)                                                             \
    do {                                                                                 \
        auto _tmp_status = (expr);                                                       \
        if (!_tmp_status.status().ok()) {                                                \
            KERNEL_PANIC("PANIC_IF_ERROR - Error code %d", _tmp_status.status().code()); \
        }                                                                                \
    } while (0)

#define ASSIGN_OR_RETURN(lhs, expr)      \
    do {                                 \
        auto _tmp_status = (expr);       \
        if (_tmp_status.status().ok()) { \
            lhs = _tmp_status.value();   \
        } else {                         \
            return _tmp_status.status(); \
        }                                \
    } while (0)

#define ASSIGN_OR_PANIC(lhs, expr)                                                        \
    do {                                                                                  \
        auto _tmp_status = (expr);                                                        \
        if (_tmp_status.status().ok()) {                                                  \
            lhs = _tmp_status.value();                                                    \
        } else {                                                                          \
            KERNEL_PANIC("ASSIGN_OR_PANIC - Error code %d", _tmp_status.status().code()); \
        }                                                                                 \
    } while (0)
