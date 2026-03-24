#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <map>

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)
#endif

namespace Utils {
    std::string getCurrentDate();
    int daysDifference(const std::string& date1, const std::string& date2);
    void clearScreen();
    void pressEnter();
    void printTable(const std::vector<std::map<std::string, std::string>>& data);
    double stringToDouble(const std::string& str);
    int stringToInt(const std::string& str);
}

#endif