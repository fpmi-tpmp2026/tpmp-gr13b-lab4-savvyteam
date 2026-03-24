#include "../includes/utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <algorithm>

namespace Utils {

    std::string getCurrentDate() {
        time_t now = time(nullptr);
        tm* ltm = localtime(&now);
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(4) << (1900 + ltm->tm_year) << "-"
            << std::setw(2) << (1 + ltm->tm_mon) << "-" << std::setw(2) << ltm->tm_mday;
        return ss.str();
    }

    int daysDifference(const std::string& date1, const std::string& date2) {
        if (date1.empty() || date2.empty()) return 0;
        int y1, m1, d1, y2, m2, d2;
        sscanf(date1.c_str(), "%d-%d-%d", &y1, &m1, &d1);
        sscanf(date2.c_str(), "%d-%d-%d", &y2, &m2, &d2);
        struct tm tm1 = { 0 }, tm2 = { 0 };
        tm1.tm_year = y1 - 1900;
        tm1.tm_mon = m1 - 1;
        tm1.tm_mday = d1;
        tm2.tm_year = y2 - 1900;
        tm2.tm_mon = m2 - 1;
        tm2.tm_mday = d2;
        time_t t1 = mktime(&tm1);
        time_t t2 = mktime(&tm2);
        return (int)difftime(t2, t1) / (60 * 60 * 24);
    }

    void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void pressEnter() {
        std::cout << "\nPress Enter...";
        std::cin.ignore();
        std::cin.get();
    }

    void printTable(const std::vector<std::map<std::string, std::string>>& data) {
        if (data.empty()) {
            std::cout << "No data.\n";
            return;
        }
        std::vector<std::string> cols;
        for (auto& row : data) {
            for (auto& col : row) {
                if (std::find(cols.begin(), cols.end(), col.first) == cols.end())
                    cols.push_back(col.first);
            }
        }
        std::vector<int> widths(cols.size(), 0);
        for (size_t i = 0; i < cols.size(); i++) {
            widths[i] = cols[i].length();
            for (auto& row : data) {
                auto it = row.find(cols[i]);
                if (it != row.end()) {
                    int len = it->second.length();
                    if (len > widths[i]) widths[i] = len > 25 ? 25 : len;
                }
            }
            if (widths[i] < 8) widths[i] = 8;
        }
        int total = 3;
        for (int w : widths) total += w + 3;
        std::cout << std::string(total, '-') << "\n";
        for (size_t i = 0; i < cols.size(); i++) {
            std::cout << "| " << std::left << std::setw(widths[i]) << cols[i].substr(0, widths[i]) << " ";
        }
        std::cout << "|\n" << std::string(total, '-') << "\n";
        for (auto& row : data) {
            for (size_t i = 0; i < cols.size(); i++) {
                std::string val = row.count(cols[i]) ? row.at(cols[i]) : "";
                if (val.length() > (size_t)widths[i]) val = val.substr(0, widths[i] - 3) + "...";
                std::cout << "| " << std::left << std::setw(widths[i]) << val << " ";
            }
            std::cout << "|\n";
        }
        std::cout << std::string(total, '-') << "\n";
    }

    double stringToDouble(const std::string& str) {
        try {
            return std::stod(str);
        }
        catch (...) {
            return 0.0;
        }
    }

    int stringToInt(const std::string& str) {
        try {
            return std::stoi(str);
        }
        catch (...) {
            return 0;
        }
    }

}