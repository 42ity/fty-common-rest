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
 * \file tokens.cc
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \author Jim Klimov <EvgenyKlimov@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \brief Maintain the OAuth2 access_tokens
 */
#include "fty_common_rest_tokens.h"
#include <fty_common_base64.h>
#include <fty_log.h>

#include <mutex>
#include <string>
#include <time.h>
#include <sys/random.h> //getrandom()

//! Round timestamps to this many seconds
#define ROUND 60

#define secret_NONCEBYTES 20
#define secret_KEYBYTES 20

struct Cipher
{
    long int      valid_until = 0;
    int           used = 0;
    unsigned char nonce[secret_NONCEBYTES + 1] = "";
    unsigned char key[secret_KEYBYTES + 1] = "";
};

//! Max time key is alive
#define MAX_LIVE 24 * 3600
//! Maximum tokens per key
#define MAX_USE 256

const uint32_t tokens::MESSAGE_LEN = (3 * sizeof(long int)) + sizeof(int) + 64;

static time_t mono_time(time_t* o_time)
{
#if defined(_POSIX_TIMERS) && defined(_POSIX_MONOTONIC_CLOCK)
    struct timespec monoTime;

    if (clock_gettime(CLOCK_MONOTONIC, &monoTime) == 0) {
        if (o_time != nullptr)
            *o_time = monoTime.tv_sec;
        return monoTime.tv_sec;
    } else
#endif
        return time(o_time);
}

static BiosProfile s_bios_profile(long int gid)
{
    long int foo = (gid - 8000);

    if (foo == static_cast<long int>(BiosProfile::Dashboard))
        return BiosProfile::Dashboard;
    else if (foo == static_cast<long int>(BiosProfile::Admin))
        return BiosProfile::Admin;
    else {
        log_warning("Cannot map gid %ld to BiosProfile", gid);
        return BiosProfile::Anonymous;
    }
}

static void s_fill_buf_with_random_bytes(unsigned char* buf, size_t len)
{
    if (buf) {
        if (len) {
            memset(buf, 0, len);
            getrandom(buf, len, 0);
            for (size_t i = 0; i < len; i++) { if (buf[i] < ' ') buf[i] += ' '; } //printable
            buf[len - 1] = 0; // 0 term str
        }
        else {
            *buf = 0;
        }
    }
}

// encrypt/decrypt a text with nonce & key privates
static std::string s_encrypt_message(const char* plaintext, const char* nonce, const char* key)
{
    //TODO real encryption

    std::string _nonce(nonce ? nonce : "<nonce>");
    std::string _key(key ? key : "<key>");
    std::string prefix = "(" + _nonce + "," + _key + ")";

    std::string _plaintext(plaintext ? plaintext : "<plaintext>");
    return prefix + _plaintext;
}

static std::string s_decrypt_message(const char* cryptedtext, const char* nonce, const char* key)
{
    //TODO decrypt as: text = decrypt(encrypt(text))

    std::string _nonce(nonce ? nonce : "<nonce>");
    std::string _key(key ? key : "<key>");
    std::string prefix = "(" + _nonce + "," + _key + ")";

    std::string _cryptedtext(cryptedtext ? cryptedtext : "<null>");
    if (_cryptedtext.find(prefix) != 0) {
        return ""; // error (empty)
    }

    return _cryptedtext.substr(prefix.size());
}

void tokens::regen_keys(long int expires_in)
{
    // drop all old keys
    auto now = mono_time(nullptr);
    while (!keys.empty() && (keys.front().valid_until < now)) {
        keys.pop_front();
    }

    if (keys.empty()
        || (keys.back().used > MAX_USE)
        || (keys.back().valid_until < (now + expires_in - MAX_LIVE))
    ) {
        Cipher cipher;
        s_fill_buf_with_random_bytes(cipher.nonce, sizeof(cipher.nonce));
        s_fill_buf_with_random_bytes(cipher.key, sizeof(cipher.key));
        cipher.valid_until = now + (2 * MAX_LIVE);
        cipher.used = 0;

        keys.push_back(cipher);
    }
}

tokens* tokens::get_instance()
{
    static tokens* inst = nullptr;

    static std::mutex mtx;
    mtx.lock();
    if (!inst) {
        inst = new tokens;
    }
    mtx.unlock();
    return inst;
}

BiosProfile tokens::gen_token(const char* user, std::string& token, long int* expires_in)
{
    token = "";

    BiosProfile profile = BiosProfile::Anonymous;
    long int uid = -1;
    long int gid = -1;

    if (user != nullptr) {
        static std::mutex pwnam_lock;
        pwnam_lock.lock();
        struct passwd* pwd = getpwnam(user);
        if (pwd != nullptr) {
            uid     = pwd->pw_uid;
            gid     = pwd->pw_gid;
            profile = s_bios_profile(gid);
        }
        pwnam_lock.unlock();
        if (!pwd) {
            log_error("Cannnot get uid for user %s: %s", user, strerror(errno));
            return BiosProfile::Anonymous;
        }
    }

    if (user && (profile == BiosProfile::Anonymous)) {
        log_warning("Cannot map gid %ld to BiosProfile", gid);
        return BiosProfile::Anonymous;
    }

    switch (profile) {
        case BiosProfile::Admin:
            *expires_in = 3600l;
            break;
        case BiosProfile::Dashboard:
            *expires_in = 3600l;
            break;
        case BiosProfile::Anonymous:
            return BiosProfile::Anonymous;
    }

    zconfig_t* root = zconfig_load(utils::config::get_path("FTY_SESSION_TIMEOUT_LEASE"));
    if (root) {
        const char* config_key = utils::config::get_mapping("FTY_SESSION_TIMEOUT_LEASE");
        zconfig_t*  item       = zconfig_locate(root, config_key);
        if (item) {
            std::string value = zconfig_value(item);
            try {
                *expires_in = std::stol(value);
            } catch (...) {
                // Nothing to do, just keep default values.
                log_error("Error on %s stol conversion", value.c_str());
            }
        }
        zconfig_destroy(&root);
    }

    static int number = int(random() % MAX_USE);

    long int tme = mono_time(nullptr) + *expires_in;
    tme /= ROUND;
    tme *= ROUND;

    static std::mutex mtx;
    mtx.lock();
    regen_keys(*expires_in);
    Cipher tmp = keys.back();
    log_debug("Cipher {key=%s, nonce=%s, valid_until=%ld}", tmp.key, tmp.nonce, tmp.valid_until);
    keys.back().used++;
    int my_number = number;
    number        = (number + 1) % MAX_USE;
    mtx.unlock();

    // username will be truncated to 32+nullptr byte by snprintf
    size_t len = strlen(user);
    if (len > 32) { len = 32; }

    char buff[MESSAGE_LEN + 1];
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "%ld %ld %ld %d %zu%.32s", tme, uid, gid, my_number, len, user);

    std::string buf2{buff};
    if (buf2.size() < MESSAGE_LEN) { buf2.resize(MESSAGE_LEN, '*'); } // '*' padding
    std::string cipheredtext = s_encrypt_message(buf2.c_str(), reinterpret_cast<char*>(tmp.nonce), reinterpret_cast<char*>(tmp.key));

    token = Base64::encode(cipheredtext.c_str(), cipheredtext.size());

    for (auto& c : token) {
        if (c == '+') c = '_';
        if (c == '/') c = '-';
    }

    return profile;
}

//assume buff size is MESSAGE_LEN+1
void tokens::decode_token(char* buff, std::string token)
{
    for (auto& c : token) {
        if (c == '_') c = '+';
        if (c == '-') c = '/';
    }

    std::string data;
    try {
        data = Base64::decode(token);
    }
    catch (...) {}

    for (const auto& it : keys) {
        std::string plaintext = s_decrypt_message(data.c_str(), reinterpret_cast<const char*>(it.nonce), reinterpret_cast<const char*>(it.key));
        if (!plaintext.empty()) {
            snprintf(buff, MESSAGE_LEN + 1, "%s", plaintext.c_str());
            return; //success
        }
    }

    //failed
    memset(buff, 0, MESSAGE_LEN + 1);
}

void tokens::clean_revoked()
{
    std::multimap<long int, std::string>::iterator it;
    while (!revoked_queue.empty() && (it = revoked_queue.begin())->first < mono_time(nullptr)) {
        revoked.erase(it->second);
        revoked_queue.erase(it);
    }
}

void tokens::revoke(const std::string token)
{
    char buff[MESSAGE_LEN + 1];
    long int tme = 0;

    memset(buff, 0, sizeof(buff));
    decode_token(buff, token);
    sscanf(buff, "%ld", &tme);
    if (tme <= mono_time(nullptr))
        return;

    revoked.insert(token);
    revoked_queue.insert(std::make_pair(tme, token));
}

BiosProfile tokens::verify_token(
    const std::string token, long int* expInSec, long int* uid, long int* gid, char** user_name)
{
    if (uid) { *uid = 0; }
    if (gid) { *gid = 0; }
    if (user_name) { *user_name = NULL; }

    clean_revoked();
    if (revoked.find(token) != revoked.end()) {
        log_info("verify_token: token is revoked, authentication failed!");
        return BiosProfile::Anonymous;
    }

    char buff[MESSAGE_LEN + 1];
    memset(buff, 0, sizeof(buff));
    decode_token(buff, token);

    long int tme = 0, l_uid = 0, l_gid = 0;
    int r = sscanf(buff, "%ld %ld %ld", &tme, &l_uid, &l_gid);
    if (r != 3) {
        log_debug("verify_token: sscanf read of tme, uid, gid, failed: %s", strerror(errno));
        return BiosProfile::Anonymous;
    }

    if (uid) { *uid = l_uid; }
    if (gid) { *gid = l_gid; }

    time_t now = mono_time(nullptr);
    if (now > tme) {
        log_info("verify_token: expired token for uid/gid %ld/%ld, authentication failed!", l_uid, l_gid);
        return BiosProfile::Anonymous;
    }
    *expInSec = tme - now;

    if (user_name) {
        char* foo = new char[MESSAGE_LEN + 1];
        // char *foo = nullptr;
        size_t foo_len;
        // find 4th space
        char* buff2 = buff;
        for (int i = 0; i != 4; i++) {
            buff2 = strchr(buff2, ' ');
            buff2++;
        }
        log_debug("buff=%s, buff=%s", buff, buff2);
        r = sscanf(buff2, " %zu%s", &foo_len, foo);
        if (r != 2) {
            log_debug("verify_token: read of username failed: %s", strerror(errno));
            if (foo)
                delete[] foo;
            return BiosProfile::Anonymous;
        }
        if (foo_len > strlen(foo)) {
            log_debug("verify_token: read username len %zu is bigger than actual string size %zu, data corruption",
                foo_len, strlen(foo));
            delete[] foo;
            return BiosProfile::Anonymous;
        }
        foo[foo_len] = '\0';
        *user_name = foo;
    }

    return s_bios_profile(l_gid);
}
