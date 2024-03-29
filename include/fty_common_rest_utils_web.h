/*
 *
 * Copyright (C) 2015 - 2020 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file   utils_web.h
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \brief  some helpers for web
 */
#pragma once

#ifdef __cplusplus

#include <array>
#include <climits>
#include <cmath>
#include <cxxtools/serializationinfo.h>
#include <czmq.h>
#include <fty_log.h>
#include <list>
#include <mutex>
#include <stdarg.h>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
//#include "utilspp.h"

#include <fty_common_macros.h>
#include <fty_common_utf8.h>
#include <tnt/http.h>

#define BIOS_SCRIPT_USER "_bios-script"

//  ###### THOSE DEFINITONS BELOW ARE PRIVATE TO http_die AND SHALL NOT BE ACCESSED DIRECTLY

/* \brief helper WebService Error struct to store all important items*/
typedef struct _wserror
{
    const char* key;       ///! short key for compile time dispatch
    int         http_code; ///! http_code is HTTP reply code, use HTTP defines
    int         err_code;  ///! sw internal error code
    const char* message;   ///! Message explaining the error, can contain printf like formatting chars
} _WSError;

// size of _errors array, keep this up to date otherwise code won't build
static constexpr size_t _WSErrorsCOUNT = 18;

// typedef for array of errors
typedef std::array<_WSError, _WSErrorsCOUNT> _WSErrors;

// WARNING!!! - don't use anything else than %s as format parameter for .message
//
// TL;DR;
// The .messages are supposed to be called with FEWER formatting arguments than defined.
// To avoid issues with going to unallocated memory, the _die_asprintf is called with
// **5** additional empty strings, which fill sefgaults for other formatting specifiers.

#define HTTP_TEAPOT 418 // see RFC2324
// clang-format off
static constexpr const _WSErrors _errors = {{
    {"undefined",                HTTP_TEAPOT,                   INT_MIN, TRANSLATE_ME_IGNORE_PARAMS ("I'm a teapot!") },
    {"internal-error",           HTTP_INTERNAL_SERVER_ERROR,    42,      TRANSLATE_ME_IGNORE_PARAMS ("Internal Server Error. %s") },
    {"not-authorized",           HTTP_UNAUTHORIZED,             43,      TRANSLATE_ME_IGNORE_PARAMS ("You are not authenticated or your rights are insufficient.") },
    {"element-not-found",        HTTP_NOT_FOUND,                44,      TRANSLATE_ME_IGNORE_PARAMS ("Element '%s' not found.") },
    {"method-not-allowed",       HTTP_METHOD_NOT_ALLOWED,       45,      TRANSLATE_ME_IGNORE_PARAMS ("Http method '%s' not allowed.") },
    {"request-param-required",   HTTP_BAD_REQUEST,              46,      TRANSLATE_ME_IGNORE_PARAMS ("Parameter '%s' is required.") },
    {"request-param-bad",        HTTP_BAD_REQUEST,              47,      TRANSLATE_ME_IGNORE_PARAMS ("Parameter '%s' has bad value. Received %s. Expected %s.") },
    {"bad-request-document",     HTTP_BAD_REQUEST,              48,      TRANSLATE_ME_IGNORE_PARAMS ("Request document has invalid syntax. %s") },
    {"data-conflict",            HTTP_CONFLICT,                 50,      TRANSLATE_ME_IGNORE_PARAMS ("Element '%s' cannot be processed because of conflict. %s") },
    {"action-forbidden",         HTTP_FORBIDDEN,                51,      TRANSLATE_ME_IGNORE_PARAMS ("%s is forbidden. %s") },
    {"parameter-conflict",       HTTP_BAD_REQUEST,              52,      TRANSLATE_ME_IGNORE_PARAMS ("Request cannot be processed because of conflict in parameters. %s") },
    {"content-too-big",          HTTP_REQUEST_ENTITY_TOO_LARGE, 53,      TRANSLATE_ME_IGNORE_PARAMS ("Content size is too big, maximum size is %s.") },
    {"not-found",                HTTP_NOT_FOUND,                54,      TRANSLATE_ME_IGNORE_PARAMS ("%s does not exist.") },
    {"precondition-failed",      HTTP_PRECONDITION_FAILED,      55,      TRANSLATE_ME_IGNORE_PARAMS ("Precondition failed - %s") },
    {"db-err",                   HTTP_INTERNAL_SERVER_ERROR,    56,      TRANSLATE_ME_IGNORE_PARAMS ("General DB error. %s") },
    {"bad-input",                HTTP_BAD_REQUEST,              57,      TRANSLATE_ME_IGNORE_PARAMS ("Incorrect input. %s") },
    {"licensing-err",            HTTP_FORBIDDEN,                58,      TRANSLATE_ME_IGNORE_PARAMS ("Action forbidden in current licensing state. %s") },
    {"upstream-err",             HTTP_BAD_GATEWAY,              59,      TRANSLATE_ME_IGNORE_PARAMS ("Server which was contacted to fulfill the request has returned an error. %s") }
}};
// clang-format on
#undef HTTP_TEAPOT

constexpr bool _strcmp(char const* a, char const* b)
{
    return (a && b) ? ((*a && *b) ? (*a == *b && _strcmp(a + 1, b + 1)) : (!*a && !*b)) : false;
}

template <ssize_t N>
constexpr ssize_t _die_idx(const char* key)
{
    static_assert(std::tuple_size<_WSErrors>::value > N, "_die_idx asked for too big N");
    return (_strcmp(_errors.at(N).key, key) || _strcmp(_errors.at(N).message, key)) ? N : _die_idx<N - 1>(key);
}

template <>
constexpr ssize_t _die_idx<1>(const char* key)
{
    static_assert(std::tuple_size<_WSErrors>::value > 1, "_die_idx asked for too big N");
    return (_strcmp(_errors.at(1).key, key) || _strcmp(_errors.at(1).message, key)) ? 1 : 0;
}

inline int _die_asprintf(char** buf, const char* format, ...)
{
    va_list args;

    va_start(args, format);
    std::string buf_str = UTF8::vajsonify_translation_string(format, args);
    va_end(args);
    size_t length = buf_str.length();
    *buf          = static_cast<char*>(zmalloc(length + 1));
    strcpy(*buf, buf_str.c_str());

    return int(length);
}

//  ###### THOSE DEFINITONS ABOVE ARE PRIVATE TO http_die AND SHALL NOT BE ACCESSED DIRECTLY

/*
 * \brief print valid json error and die with proper http code
 *
 * \param[in] key - the .key or .message from static list of errors
 * \param[in] ... - format arguments for .message template
 *
 * can be used as ``http_die("internal-error", e.what())`` or
 * ``http_die("Internal Server Error: %s", e.what())``
 *
 * Mistakes in key strings are compile time errors, so one can't
 * make the mistake even if he want to.
 *
 */

/* If there is a codepath that did not choose a particular Content-Type yet,
 * make sure it is JSON (e.g. die on bad permissions for getlog action) */
#define http_die_contenttype_graceful(_replyobj)                                                                       \
    do {                                                                                                               \
        if (_replyobj.getContentType() == std::string("")) {                                                           \
            _replyobj.setContentType("application/json;charset=UTF-8");                                                \
        }                                                                                                              \
    } while (0)

#define http_die_contenttype_brutal(_replyobj)                                                                         \
    do {                                                                                                               \
        _replyobj.setContentType("application/json;charset=UTF-8");                                                    \
    } while (0)

/* By default use this variant - our version of tntnet seems VERY BAD at
 * replacing headers in practice, so we should only add one if not present */
#define http_die_contenttype http_die_contenttype_graceful

#define http_die(key, ...)                                                                                             \
    do {                                                                                                               \
        constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT - 1>(reinterpret_cast<const char*>(key));     \
        static_assert(__http_die__key_idx__ != 0,                                                                      \
            "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key");          \
        char* __http_die__error_message__ = NULL;                                                                      \
        _die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__);         \
        if (::getenv("BIOS_LOG_LEVEL") && !strcmp(::getenv("BIOS_LOG_LEVEL"), "LOG_DEBUG")) {                          \
            std::string __http_die__debug__ = {__FILE__};                                                              \
            __http_die__debug__ += ": " + std::to_string(__LINE__);                                                    \
            reply.out() << utils::json::create_error_json(                                                             \
                __http_die__error_message__, _errors.at(__http_die__key_idx__).err_code, __http_die__debug__);         \
        } else                                                                                                         \
            reply.out() << utils::json::create_error_json(                                                             \
                __http_die__error_message__, _errors.at(__http_die__key_idx__).err_code);                              \
        free(__http_die__error_message__);                                                                             \
        http_die_contenttype(reply);                                                                                   \
        return _errors.at(__http_die__key_idx__).http_code;                                                            \
    } while (0)


/**
 *  \brief http die based on _error index number
 *
 *  \param[in] idx - index to _error array
 *  \param[msg] msg - message to be printed
 *
 * idx is normalized before is used - absolute value is used for indexing, so 1
 * equals to -1.
 * If it's bigger than size of array, it's changed to 0.
 * 0 means, that there is an error hidden somewhere.
 *
 * XXX: should not we change it to http_die_be to use instance of BiosError directly?
 */
#define http_die_idx(idx, msg)                                                                                         \
    do {                                                                                                               \
        int64_t _idx = idx;                                                                                            \
        if (_idx < 0)                                                                                                  \
            _idx = _idx * -1;                                                                                          \
        if (_idx >= int64_t(_WSErrorsCOUNT))                                                                           \
            _idx = 0;                                                                                                  \
        if (_idx == 0)                                                                                                 \
            log_error("TEAPOT");                                                                                       \
        if (::getenv("BIOS_LOG_LEVEL") && !strcmp(::getenv("BIOS_LOG_LEVEL"), "LOG_DEBUG")) {                          \
            std::string __http_die__debug__ = {__FILE__};                                                              \
            __http_die__debug__ += ": " + std::to_string(__LINE__);                                                    \
            reply.out() << utils::json::create_error_json(                                                             \
                msg, uint32_t(_errors.at(size_t(_idx)).err_code), __http_die__debug__);                                \
        } else                                                                                                         \
            reply.out() << utils::json::create_error_json(msg, uint32_t(_errors.at(size_t(_idx)).err_code));           \
        http_die_contenttype(reply);                                                                                   \
        return uint32_t(_errors.at(size_t(_idx)).http_code);                                                           \
    } while (0)

typedef struct _http_errors_t
{
    uint32_t                                                    http_code;
    std::vector<std::tuple<uint32_t, std::string, std::string>> errors;
} http_errors_t;

#define http_add_error(debug, errors, key, ...)                                                                        \
    do {                                                                                                               \
        static_assert(std::is_same<decltype(errors), http_errors_t&>::value,                                           \
            "'errors' argument in macro http_add_error must be a http_errors_t.");                                     \
        constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT - 1>(static_cast<const char*>(key));          \
        static_assert(__http_die__key_idx__ != 0,                                                                      \
            "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key");          \
        (errors).http_code                = _errors.at(__http_die__key_idx__).http_code;                               \
        char* __http_die__error_message__ = nullptr;                                                                   \
        _die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__);         \
        (errors).errors.push_back(                                                                                     \
            std::make_tuple(_errors.at(__http_die__key_idx__).err_code, __http_die__error_message__, (debug)));        \
        free(__http_die__error_message__);                                                                             \
    } while (0)

#define http_die_error(errors)                                                                                         \
    do {                                                                                                               \
        static_assert(std::is_same<decltype(errors), http_errors_t>::value,                                            \
            "'errors' argument in macro http_add_error must be a http_errors_t.");                                     \
        reply.out() << utils::json::create_error_json((errors).errors);                                                \
        http_die_contenttype(reply);                                                                                   \
        return (errors).http_code;                                                                                     \
    } while (0)

/**
 * \brief general bios error
 *
 * This exception is not supposed to be created manually as it's easy to
 * misstype it. The bios_throw macro below is supposed to be used.
 *
 * Only one good case is to throw an error from low level function, euther via
 * return + message string or via db_reply_t or reply_t types.
 */
struct BiosError : std::invalid_argument
{
    explicit BiosError(size_t _idx, const std::string& message)
        : std::invalid_argument(message)
        , idx(_idx)
    {
    }

    size_t idx;
};

/*
 * \brief get the index to error message and formatted error message
 *
 * \param[out] idx - variable name to store index
 * \param[out] str - variable name to store formatted error message
 * \param[in]  key - the .key or .message from static list of errors
 * \param[in]  ... - format arguments for .message template
 *
 * It is supposed for DB functions to return errors easily expressed in REST API
 *
 * // new low level DB API:
 * int db_foo_bar(conn, id, std::string &err) {
 *   ...
 *   if (!failed)
 *     return 0;
 *   ...
 *   int idx;
 *   bios_error_idx(idx, err, "internal-error");
 *   return -idx;
 * }
 *
 * // old low level DB API:
 * db_reply_t db_ham_spam(conn, id) {
 *   ...
 *   if (!failed)
 *     return ret;
 *   ...
 *   bios_error_idx(ret.rowid, ret.msg, "internal-error");
 *   return ret;
 * }
 *
 * // in REST API
 * int r = db_foo_bar(...);
 * if (r == 0)
 *   return HTTP_OK;
 *
 * http_die_idx(idx, message);
 *
 */

#define bios_error_idx(idx, str, key, ...)                                                                             \
    do {                                                                                                               \
        static_assert(                                                                                                 \
            std::is_same<decltype(str), std::string>::value || std::is_same<decltype(str), std::string&>::value,       \
            "'str' argument in macro bios_error_idx must be a std::string.");                                          \
        constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT - 1>(const_cast<const char*>(key));           \
        static_assert(__http_die__key_idx__ != 0,                                                                      \
            "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key");          \
        char* __http_die__error_message__ = NULL;                                                                      \
        _die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__);         \
        str = __http_die__error_message__;                                                                             \
        idx = __http_die__key_idx__;                                                                                   \
        free(__http_die__error_message__);                                                                             \
    } while (0)

/**
 * \brief throw specified bios error
 *
 * \param[in] key - the .key or .message from static list of errors
 * \param[in] ... - format arguments for .message template
 *
 * Similar to http_die, just throws BiosError, where it can be easily 'consumed'
 * by http_die_idx macro.
 *
 */

#define bios_throw(key, ...)                                                                                           \
    do {                                                                                                               \
        constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT - 1>(static_cast<const char*>(key));          \
        static_assert(__http_die__key_idx__ != 0,                                                                      \
            "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key");          \
        char* __http_die__error_message__ = NULL;                                                                      \
        _die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__);         \
        std::string str{__http_die__error_message__};                                                                  \
        free(__http_die__error_message__);                                                                             \
        log_warning("throw BiosError{%zu, \"%s\"}", __http_die__key_idx__, str.c_str());                               \
        throw BiosError{__http_die__key_idx__, str};                                                                   \
    } while (0);


// General template for whether type T (a standard container) is iterable
// We deliberatelly don't want to solve this for general case (we don't need it)
// If we ever need a general is_container - http://stackoverflow.com/a/9407521
template <typename T>
struct is_iterable
{
    static const bool value = false;
};

// Partial specialization for std::vector
template <typename T, typename Alloc>
struct is_iterable<std::vector<T, Alloc>>
{
    static const bool value = true;
};

// Partial specialization for std::list
template <typename T, typename Alloc>
struct is_iterable<std::list<T, Alloc>>
{
    static const bool value = true;
};


namespace utils {

/*!
 \brief Convert string to element identifier

 \throws std::out_of_range      When number represented by 'string' is out of <1, UINT_MAX> range
 \throws std::invalid_argument  When 'string' does not represent a number
 \return element identifier or throws
*/
uint32_t string_to_element_id(const std::string& string);

/*!
 \brief  Return an identifier for the MLM client, based on PID/pThreadID/LWP-id
 \return Provided prefix plus ID from definition above, or plus a random number.
*/
std::string generate_mlm_client_id(std::string client_name);


namespace json {

    std::string jsonify(double t);

    template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    std::string jsonify(T t)
    {
        try {
            return UTF8::escape(std::to_string(t));
        } catch (...) {
            return "";
        }
    }

    // TODO: doxy
    // basically, these are property "jsonifyrs"; you supply any json-ifiable type pair and it creates a valid, properly
    // escaped property (key:value) pair out of it. single arg version escapes and quotes were necessary (i.e. except
    // int types...)

    template <typename T, typename std::enable_if<std::is_convertible<T, std::string>::value>::type* = nullptr>
    std::string jsonify(const T& t)
    {
        try {
            // check if the arg is already JSON
            std::string t_str(t);
            size_t      length = t_str.length();
            if (length >= 2 && t[0] == '{' && t[1] != '{' && t[length - 2] != '}' && t[length - 1] == '}') {
                log_trace(t_str.c_str());
                return t;
            }
            return std::string("\"").append(UTF8::escape(t)).append("\"");
        } catch (...) {
            return "";
        }
    }

    template <typename T, typename std::enable_if<is_iterable<T>::value>::type* = nullptr>
    std::string jsonify(const T& t)
    {
        try {
            std::string result = "[ ";
            bool        first  = true;
            for (const auto& item : t) {
                if (first) {
                    result += jsonify(item);
                    first = false;
                } else {
                    result += ", " + jsonify(item);
                }
            }
            result += " ]";
            return result;
        } catch (...) {
            return "[]";
        }
    }

    template <typename S, typename std::enable_if<std::is_convertible<S, std::string>::value>::type* = nullptr,
        typename T, typename std::enable_if<std::is_convertible<T, std::string>::value>::type*       = nullptr>
    std::string jsonify(const S& key, const T& value)
    {
        return std::string(jsonify(key)).append(" : ").append(jsonify(value));
    }

    template <typename S, typename = typename std::enable_if<std::is_convertible<S, std::string>::value>::type,
        typename T, typename       = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    std::string jsonify(const S& key, T value)
    {
        return std::string(jsonify(key)).append(" : ").append(jsonify(value));
    }

    template <typename S, typename std::enable_if<std::is_convertible<S, std::string>::value>::type* = nullptr,
        typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type*                     = nullptr>
    std::string jsonify(T key, const S& value)
    {
        return std::string("\"").append(jsonify(key)).append("\" : ").append(jsonify(value));
    }

    template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    std::string jsonify(T key, T value)
    {
        return std::string("\"").append(jsonify(key)).append("\" : ").append(jsonify(value));
    }

    template <typename S, typename std::enable_if<std::is_convertible<S, std::string>::value>::type* = nullptr,
        typename T, typename std::enable_if<is_iterable<T>::value>::type*                            = nullptr>
    std::string jsonify(const S& key, const T& value)
    {
        return std::string(jsonify(key)).append(" : ").append(jsonify(value));
    }

    template <typename S, typename std::enable_if<is_iterable<S>::value>::type* = nullptr, typename T,
        typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
    std::string jsonify(T key, const S& value)
    {
        return std::string("\"").append(jsonify(key)).append("\" : ").append(jsonify(value));
    }

    std::string create_error_json(const std::string& message, uint32_t code);

    std::string create_error_json(const std::string& message, uint32_t code, const std::string& debug);

    std::string create_error_json(std::vector<std::tuple<uint32_t, std::string, std::string>> messages);

} // namespace json

namespace config {

    const char* get_mapping(const std::string& key);

    const char* get_path(const std::string& key);

    /*!
     \brief Convert json for config call to respective zpl structure

     \param [out] roots - map of file_path to zconfig with updated values
     \param [in]  si - parser JSON document
     \param [in]  lock - the guard to ensure this function runs only once
     \throws BiosError if input parameters are wrong
    */
    void json2zpl(std::map<std::string, zconfig_t*>& roots, const cxxtools::SerializationInfo& si,
        std::lock_guard<std::mutex>& lock);

    /*!
     \brief Free zpl structures allocated by json2zpl

     \param [in] roots - map of file_path to zconfig with updated values
    */
    void roots_destroy(const std::map<std::string, zconfig_t*>& roots);

} // namespace config

namespace email {
    /*!
     *  \brief Add various X-Eaton-IPC headers ho zhash_t
     *   \param [in]  headers - hashmap with headers
     *   */
    void x_headers(zhash_t* headers);
} // namespace email

} // namespace utils


#ifdef __cplusplus
extern "C" {
#endif

//  Self test of this class
void fty_common_rest_utils_web_test(bool verbose);

#ifdef __cplusplus
}
#endif

#endif // __cplus_plus
