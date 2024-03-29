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

/*
@header
    fty_common_web_audit_log - Manage audit log
@discuss
@end
*/

#include "fty_common_rest_audit_log.h"

#define AUDIT_LOGGER_NAME "audit/rest"

Ftylog AuditLogManager::_auditlog = Ftylog(AUDIT_LOGGER_NAME, FTY_COMMON_LOGGING_DEFAULT_CFG);

// ISSUE: audit logs go silent due to MDC management
// WORKAROUND: always reload logger
void AuditLogManager::reloadAuditLogger()
{
    _auditlog.change(AUDIT_LOGGER_NAME, FTY_COMMON_LOGGING_DEFAULT_CFG);
}

//  return audit logger
Ftylog* AuditLogManager::getInstance()
{
    reloadAuditLogger();
    return &_auditlog;
}

void AuditLogManager::setAuditLogContext(
    const std::string token, const std::string username, const int userId, const std::string ip)
{
    std::hash<std::string> hasher;
    std::size_t hashedToken = hasher(token);

    // Prepare Mapped Diagnostic Context (MDC) for audit
    // Note: sessionId, see MDC equiv. code in 42ity:fty-rest.git my_profile.ecpp
    std::map<std::string, std::string> contextParam;
    contextParam.insert(std::make_pair("sessionid", std::to_string(hashedToken)));
    contextParam.insert(std::make_pair("username", username));
    contextParam.insert(std::make_pair("uid", std::to_string(userId)));
    contextParam.insert(std::make_pair("IP", ip));

    // Set the MDC context
    Ftylog::clearContext();
    Ftylog::setContext(contextParam);
}

void AuditLogManager::clearAuditLogContext()
{
    Ftylog::clearContext();
}
