#pragma once
#include "boost/asio.hpp"


namespace Fix::Error {


    enum class RetryClass {
        Permanent,
        Transient,
        Canceled
    };

    inline RetryClass classify_resolve_error(const boost::system::error_code& ec) {
        
        if (ec == boost::asio::error::operation_aborted) return RetryClass::Canceled;
        if (ec == boost::asio::error::host_not_found_try_again) return RetryClass::Transient;

        if (ec == boost::asio::error::host_not_found ||
            ec == boost::asio::error::service_not_found ||
            ec == boost::asio::error::no_recovery ||
            ec == boost::asio::error::invalid_argument) return RetryClass::Permanent;

        // default: treat unknown resolve errors as transient but log them
        return RetryClass::Transient;
    }

    inline RetryClass classify_connect_error(const boost::system::error_code ec) {
        if (ec == boost::asio::error::operation_aborted) return RetryClass::Canceled;

        if (ec == boost::asio::error::connection_refused ||
            ec == boost::asio::error::timed_out ||
            ec == boost::asio::error::network_unreachable ||
            ec == boost::asio::error::host_unreachable ||
            ec == boost::asio::error::connection_reset) return RetryClass::Transient;


        if (ec == boost::asio::error::address_family_not_supported ||
            ec == boost::asio::error::already_connected ||
            ec == boost::asio::error::invalid_argument) return RetryClass::Permanent;

        return RetryClass::Transient;
    }

    inline RetryClass classify_readwrite_error(const boost::system::error_code ec) {
        if (ec == boost::asio::error::operation_aborted ||
            ec == boost::asio::error::eof
        ) return RetryClass::Canceled;

        if (ec == boost::asio::error::would_block ||
            ec == boost::asio::error::try_again||
            ec == boost::asio::error::timed_out) return RetryClass::Transient;

        if (ec == boost::asio::error::not_connected ||
            ec == boost::asio::error::connection_reset ||
            ec == boost::asio::error::connection_aborted ||
            ec == boost::asio::error::broken_pipe ||
            ec == boost::asio::error::network_down ||
            ec == boost::asio::error::network_unreachable ||
            ec == boost::asio::error::host_unreachable ||
            ec == boost::asio::error::bad_descriptor) return RetryClass::Permanent;

        return RetryClass::Transient;
    }
}