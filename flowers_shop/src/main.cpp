#include "../includes/database.h"
#include "../includes/auth.h"
#include "../includes/flower_shop.h"
#include <iostream>

int main() {
    std::cout << "=== FLOWER SHOP MANAGEMENT SYSTEM ===\n";

    Database db;
    if (!db.connect()) {
        std::cerr << "Database connection failed!\n";
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    if (!db.initFromSQL()) {
        std::cerr << "Failed to initialize database!\n";
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }

    Auth auth;
    FlowerShop shop(db, auth);
    shop.run();

    return 0;
}