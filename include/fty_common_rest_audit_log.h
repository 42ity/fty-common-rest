/*  =========================================================================
    fty_common_web_audit_log - Manage audit log

    Copyright (C) 2014 - 2020 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

#pragma once

#ifndef __cplusplus
#define __cplusplus
#endif

#include <fty_log.h>
#include <log4cplus/mdc.h>
#include <log4cplus/ndc.h>


/* Prints message in Audit Log with DEBUG level. */
#define log_debug_audit(...) log_debug_log(AuditLogManager::getInstance(), __VA_ARGS__);

/* Prints message in Audit Log with INFO level. */
#define log_info_audit(...) log_info_log(AuditLogManager::getInstance(), __VA_ARGS__);

/* Prints message in Audit Log with WARNING level*/
#define log_warning_audit(...) log_warning_log(AuditLogManager::getInstance(), __VA_ARGS__);

/* Prints message in Audit Log with ERROR level*/
#define log_error_audit(...) log_error_log(AuditLogManager::getInstance(), __VA_ARGS__);

/* Prints message in Audit Log with FATAL level. */
#define log_fatal_audit(...) log_fatal_log(AuditLogManager::getInstance(), __VA_ARGS__);

// singleton for logger management
class AuditLogManager
{
private:
    AuditLogManager() = default;
    static Ftylog _auditlog;

    static void reloadAuditLogger();

public:
    // Return singleton Audit Ftylog instance
    static Ftylog* getInstance();

    /**
     * Set audit log context.
     * @param token The token
     * @param username The user name
     * @param userId The user id
     * @param ip The ip address
     */
    static void setAuditLogContext(
        const std::string token, const std::string username, const int userId, const std::string ip);

    /**
     * Clear audit log context.
     */
    static void clearAuditLogContext();
};
