#include "../includes/flower_shop.h"
#include "../includes/database.h"
#include "../includes/auth.h"
#include "../includes/utils.h"
#include <iostream>
#include <sstream>

FlowerShop::FlowerShop(Database& database, Auth& authentication) : db(database), auth(authentication) {}

void FlowerShop::run() {
    while (true) {
        if (!auth.isLoggedIn()) showMainMenu();
        else if (auth.isAdmin()) showAdminMenu();
        else showUserMenu();
    }
}

void FlowerShop::showMainMenu() {
    Utils::clearScreen();
    std::cout << "=== FLOWER SHOP ===\n1.Login\n2.Register\n3.Exit\nChoice: ";
    int ch;
    std::cin >> ch;
    if (ch == 1) {
        std::string email, phone;
        std::cout << "Email: "; std::cin >> email;
        std::cout << "Phone: "; std::cin >> phone;
        auth.login(email, phone, db);
        Utils::pressEnter();
    }
    else if (ch == 2) {
        std::string ln, fn, mn, phone, email;
        std::cout << "Last name: "; std::cin >> ln;
        std::cout << "First name: "; std::cin >> fn;
        std::cout << "Middle name: "; std::cin.ignore(); std::getline(std::cin, mn);
        std::cout << "Phone: "; std::cin >> phone;
        std::cout << "Email: "; std::cin >> email;
        if (auth.registerUser(ln, fn, mn, phone, email, db))
            std::cout << "Registered!\n";
        else std::cout << "Failed!\n";
        Utils::pressEnter();
    }
    else if (ch == 3) exit(0);
}

void FlowerShop::showUserMenu() {
    Utils::clearScreen();
    std::cout << "=== USER: " << auth.getUserName() << " ===\n"
        << "1.Flowers\n2.Arrangements\n3.View composition\n4.Make order\n5.My orders\n6.Logout\nChoice: ";
    int ch;
    std::cin >> ch;
    if (ch == 1) viewFlowers();
    else if (ch == 2) viewArrangements();
    else if (ch == 3) viewComposition();
    else if (ch == 4) makeOrder();
    else if (ch == 5) viewMyOrders();
    else if (ch == 6) auth.logout();
}

void FlowerShop::showAdminMenu() {
    Utils::clearScreen();
    std::cout << "=== ADMIN ===\n"
        << "1.Flowers\n2.Add flower\n3.Update flower price\n4.Delete flower\n"
        << "5.Arrangements\n6.Add arrangement\n7.All orders\n8.Reports\n9.Config\n10.Update config\n11.Logout\nChoice: ";
    int ch;
    std::cin >> ch;
    if (ch == 1) viewFlowers();
    else if (ch == 2) addFlower();
    else if (ch == 3) updateFlower();
    else if (ch == 4) {
        viewFlowers();
        int id; std::cout << "Flower ID: "; std::cin >> id;
        std::stringstream ss; ss << "DELETE FROM flowers WHERE flower_id=" << id;
        if (db.exec(ss.str())) std::cout << "Deleted\n";
        Utils::pressEnter();
    }
    else if (ch == 5) viewArrangements();
    else if (ch == 6) addArrangement();
    else if (ch == 7) viewAllOrders();
    else if (ch == 8) showReports();
    else if (ch == 9) viewConfig();
    else if (ch == 10) updateConfig();
    else if (ch == 11) auth.logout();
}

void FlowerShop::viewFlowers() {
    Utils::printTable(db.getAllFlowers());
    Utils::pressEnter();
}

void FlowerShop::viewArrangements() {
    Utils::printTable(db.getAllArrangements());
    Utils::pressEnter();
}

void FlowerShop::viewComposition() {
    viewArrangements();
    int id; std::cout << "Arrangement ID: "; std::cin >> id;
    Utils::printTable(db.getArrangementComposition(id));
    Utils::pressEnter();
}

void FlowerShop::makeOrder() {
    viewArrangements();
    int aid, qty;
    std::string cdate;
    std::cout << "Arrangement ID: "; std::cin >> aid;
    std::cout << "Quantity: "; std::cin >> qty;
    std::cout << "Completion date (YYYY-MM-DD, empty if not done): "; std::cin >> cdate;
    if (db.addOrderWithCost(auth.getUserId(), aid, qty, Utils::getCurrentDate(), cdate))
        std::cout << "Order placed!\n";
    else std::cout << "Failed!\n";
    Utils::pressEnter();
}

void FlowerShop::viewMyOrders() {
    Utils::printTable(db.getUserOrders(auth.getUserId()));
    Utils::pressEnter();
}

void FlowerShop::viewAllOrders() {
    Utils::printTable(db.getAllOrders());
    Utils::pressEnter();
}

void FlowerShop::viewConfig() {
    Utils::printTable(db.getConfig());
    Utils::pressEnter();
}

void FlowerShop::addFlower() {
    std::string name, variety;
    double price;
    std::cout << "Name: "; std::cin >> name;
    std::cout << "Variety: "; std::cin >> variety;
    std::cout << "Price: "; std::cin >> price;
    if (db.addFlower(name, variety, price)) std::cout << "Added!\n";
    Utils::pressEnter();
}

void FlowerShop::updateFlower() {
    viewFlowers();
    int id; double price;
    std::cout << "Flower ID: "; std::cin >> id;
    std::cout << "New price: "; std::cin >> price;
    if (db.checkPriceIncrease(id, price)) {
        if (db.updateFlowerPrice(id, price)) std::cout << "Updated!\n";
    }
    else std::cout << "Blocked (10% rule)!\n";
    Utils::pressEnter();
}

void FlowerShop::addArrangement() {
    std::string name;
    std::cout << "Name: "; std::cin.ignore(); std::getline(std::cin, name);
    if (db.addArrangement(name)) {
        std::cout << "Added! Now add flowers to composition.\n";
        viewFlowers();
        int aid = Utils::stringToInt(db.query("SELECT last_insert_rowid() as id")[0]["id"]);
        char more = 'y';
        while (more == 'y') {
            int fid, qty;
            std::cout << "Flower ID: "; std::cin >> fid;
            std::cout << "Quantity: "; std::cin >> qty;
            db.addToComposition(aid, fid, qty);
            std::cout << "Add more? (y/n): "; std::cin >> more;
        }
    }
    Utils::pressEnter();
}

void FlowerShop::updateConfig() {
    viewConfig();
    std::string key, val;
    std::cout << "Config key: "; std::cin >> key;
    std::cout << "New value: "; std::cin >> val;
    if (db.updateConfig(key, val)) std::cout << "Updated!\n";
    Utils::pressEnter();
}

void FlowerShop::showReports() {
    Utils::clearScreen();
    std::cout << "=== REPORTS ===\n"
        << "1.Orders by period (total money)\n"
        << "2.Most popular arrangement\n"
        << "3.Orders by urgency\n"
        << "4.Flowers usage by period\n"
        << "5.Arrangements sales by period\n"
        << "6.Orders by specific date\n"
        << "7.Back\nChoice: ";
    int ch;
    std::cin >> ch;
    if (ch == 1) reportOrdersByPeriod();
    else if (ch == 2) reportMostPopular();
    else if (ch == 3) reportByUrgency();
    else if (ch == 4) reportFlowersUsage();
    else if (ch == 5) reportArrangementsSales();
    else if (ch == 6) reportOrdersByDate();
}

void FlowerShop::reportOrdersByPeriod() {
    std::string start, end;
    std::cout << "Start date (YYYY-MM-DD): "; std::cin >> start;
    std::cout << "End date (YYYY-MM-DD): "; std::cin >> end;
    auto data = db.getOrdersByPeriod(start, end);
    double total = 0;
    for (auto& row : data) total += Utils::stringToDouble(row["total_price"]);
    std::cout << "\nTOTAL REVENUE: " << total << " BYN\n";
    Utils::printTable(data);
    Utils::pressEnter();
}

void FlowerShop::reportMostPopular() {
    auto data = db.getMostPopularArrangement();
    if (!data.empty()) {
        Utils::printTable(data);
        int aid = Utils::stringToInt(data[0]["arrangement_id"]);
        std::cout << "\n=== COMPOSITION ===\n";
        Utils::printTable(db.getArrangementComposition(aid));
    }
    Utils::pressEnter();
}

void FlowerShop::reportByUrgency() {
    Utils::printTable(db.getOrdersByUrgency());
    Utils::pressEnter();
}

void FlowerShop::reportFlowersUsage() {
    std::string start, end;
    std::cout << "Start date: "; std::cin >> start;
    std::cout << "End date: "; std::cin >> end;
    Utils::printTable(db.getFlowersUsage(start, end));
    Utils::pressEnter();
}

void FlowerShop::reportArrangementsSales() {
    std::string start, end;
    std::cout << "Start date: "; std::cin >> start;
    std::cout << "End date: "; std::cin >> end;
    Utils::printTable(db.getArrangementsSales(start, end));
    Utils::pressEnter();
}

void FlowerShop::reportOrdersByDate() {
    std::string date;
    std::cout << "Date (YYYY-MM-DD): "; std::cin >> date;
    Utils::printTable(db.getOrdersByDate(date));
    Utils::pressEnter();
}