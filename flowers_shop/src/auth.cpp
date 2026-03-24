#include "../includes/auth.h"
#include "../includes/database.h"
#include <iostream>
#include <sstream>

Auth::Auth() : current_user_id(-1), current_user_name(""), logged_in(false), is_admin(false) {}

bool Auth::login(const std::string& email, const std::string& phone, Database& db) {
    std::stringstream ss;
    ss << "SELECT user_id, last_name, first_name, is_active FROM users WHERE email='"
        << email << "' AND phone='" << phone << "'";
    auto res = db.query(ss.str());
    if (res.empty()) {
        std::cout << "Invalid credentials!\n";
        return false;
    }
    if (res[0]["is_active"] == "0") {
        std::cout << "Account disabled!\n";
        return false;
    }
    current_user_id = std::stoi(res[0]["user_id"]);
    current_user_name = res[0]["last_name"] + " " + res[0]["first_name"];
    logged_in = true;
    is_admin = (email == "admin@flower.com");
    std::cout << "Welcome, " << current_user_name << "!\n";
    return true;
}

void Auth::logout() {
    logged_in = false;
    is_admin = false;
    current_user_id = -1;
    current_user_name = "";
    std::cout << "Logged out.\n";
}

bool Auth::isLoggedIn() const { return logged_in; }
bool Auth::isAdmin() const { return is_admin; }
int Auth::getUserId() const { return current_user_id; }
std::string Auth::getUserName() const { return current_user_name; }

bool Auth::registerUser(const std::string& last_name, const std::string& first_name,
    const std::string& middle_name, const std::string& phone,
    const std::string& email, Database& db) {
    std::stringstream ss;
    ss << "INSERT INTO users (last_name, first_name, middle_name, phone, email, registration_date, is_active) VALUES ('"
        << last_name << "','" << first_name << "',";
    if (middle_name.empty()) ss << "NULL,";
    else ss << "'" << middle_name << "',";
    ss << "'" << phone << "','" << email << "','2024-03-24',1)";
    return db.exec(ss.str());
}