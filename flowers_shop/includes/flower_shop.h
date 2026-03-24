#ifndef FLOWER_SHOP_H
#define FLOWER_SHOP_H

class Database;
class Auth;

class FlowerShop {
private:
    Database& db;
    Auth& auth;

    void showMainMenu();
    void showUserMenu();
    void showAdminMenu();

    void viewFlowers();
    void viewArrangements();
    void viewComposition();
    void makeOrder();
    void viewMyOrders();
    void viewConfig();

    void addFlower();
    void updateFlower();
    void addArrangement();
    void viewAllOrders();
    void updateConfig();

    void showReports();
    void reportOrdersByPeriod();
    void reportMostPopular();
    void reportByUrgency();
    void reportFlowersUsage();
    void reportArrangementsSales();
    void reportOrdersByDate();

public:
    FlowerShop(Database& database, Auth& authentication);
    void run();
};

#endif