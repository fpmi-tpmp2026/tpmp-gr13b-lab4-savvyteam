#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <vector>
#include <map>

class Database;

class Auth {
private:
    int current_user_id;
    std::string current_user_name;
    bool logged_in;
    bool is_admin;

public:
    Auth();

    bool login(const std::string& email, const std::string& phone, Database& db);
    void logout();

    bool isLoggedIn() const;
    bool isAdmin() const;
    int getUserId() const;
    std::string getUserName() const;

    bool registerUser(const std::string& last_name, const std::string& first_name,
        const std::string& middle_name, const std::string& phone,
        const std::string& email, Database& db);
};

#endif