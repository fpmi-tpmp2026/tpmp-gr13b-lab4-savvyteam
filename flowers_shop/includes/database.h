#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>

class Database {
private:
    sqlite3* db;

public:
    Database();
    ~Database();

    bool connect();
    void disconnect();
    bool exec(const std::string& sql);
    std::vector<std::map<std::string, std::string>> query(const std::string& sql);

    bool initFromSQL();

    double calcOrderCost(int arrangement_id, int quantity, const std::string& order_date, const std::string& completion_date);
    bool addOrderWithCost(int user_id, int arrangement_id, int quantity, const std::string& order_date, const std::string& completion_date);

    bool checkPriceIncrease(int flower_id, double new_price);
    bool updateFlowerPrice(int flower_id, double new_price);

    std::vector<std::map<std::string, std::string>> getOrdersByPeriod(const std::string& start, const std::string& end);
    std::vector<std::map<std::string, std::string>> getMostPopularArrangement();
    std::vector<std::map<std::string, std::string>> getOrdersByUrgency();
    std::vector<std::map<std::string, std::string>> getFlowersUsage(const std::string& start, const std::string& end);
    std::vector<std::map<std::string, std::string>> getArrangementsSales(const std::string& start, const std::string& end);
    std::vector<std::map<std::string, std::string>> getOrdersByDate(const std::string& date);

    std::vector<std::map<std::string, std::string>> getAllFlowers();
    std::vector<std::map<std::string, std::string>> getAllArrangements();
    std::vector<std::map<std::string, std::string>> getArrangementComposition(int arrangement_id);
    std::vector<std::map<std::string, std::string>> getUserOrders(int user_id);
    std::vector<std::map<std::string, std::string>> getAllOrders();
    std::vector<std::map<std::string, std::string>> getConfig();

    bool addFlower(const std::string& name, const std::string& variety, double price);
    bool addArrangement(const std::string& name);
    bool addToComposition(int arrangement_id, int flower_id, int quantity);
    bool updateConfig(const std::string& key, const std::string& value);
};

#endif