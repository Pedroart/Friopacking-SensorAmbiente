#pragma once
#include <Arduino.h>
enum class UserRole {
    VIEWER = 0,
    OPERATOR = 1,
    ADMIN = 2,
};

struct UserEntry {
    UserRole role;
    String username;
    String password_hash;
    bool enabled;
};

struct UserConfig
{
    UserEntry admin;
    UserEntry operador;
    UserEntry viewer;
};