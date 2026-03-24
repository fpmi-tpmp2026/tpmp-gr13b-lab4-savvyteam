#include "../includes/database.h"
#include "../includes/utils.h"
#include <iostream>
#include <sstream>
#include <fstream>

static int callback(void* data, int argc, char** argv, char** azColName) {
    auto* rows = (std::vector<std::map<std::string, std::string>>*)data;
    std::map<std::string, std::string> row;
    for (int i = 0; i < argc; i++) {
        row[azColName[i]] = argv[i] ? argv[i] : "";
    }
    rows->push_back(row);
    return 0;
}

Database::Database() : db(nullptr) {}

Database::~Database() {
    if (db) sqlite3_close(db);
}

bool Database::connect() {
    if (sqlite3_open("flower_shop.db", &db) != SQLITE_OK) {
        std::cerr << "Error opening DB: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    return true;
}

void Database::disconnect() {
    if (db) sqlite3_close(db);
    db = nullptr;
}

bool Database::exec(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "SQL error: " << err << std::endl;
        sqlite3_free(err);
        return false;
    }
    return true;
}

std::vector<std::map<std::string, std::string>> Database::query(const std::string& sql) {
    std::vector<std::map<std::string, std::string>> rows;
    char* err = nullptr;
    sqlite3_exec(db, sql.c_str(), callback, &rows, &err);
    if (err) sqlite3_free(err);
    return rows;
}

bool Database::initFromSQL() {
    std::ifstream file("sql/init.sql");
    if (!file.is_open()) {
        file.open("../sql/init.sql");
    }
    if (!file.is_open()) {
        std::cerr << "Cannot open init.sql\n";
        return false;
    }
    std::string content, line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    file.close();
    char* err = nullptr;
    if (sqlite3_exec(db, content.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "Error: " << err << std::endl;
        sqlite3_free(err);
        return false;
    }
    return true;
}

double Database::calcOrderCost(int arrangement_id, int quantity, const std::string& order_date, const std::string& completion_date) {
    std::stringstream ss;
    ss << "SELECT SUM(f.price_per_unit * ac.quantity) as total FROM arrangement_compositions ac "
        << "JOIN flowers f ON ac.flower_id = f.flower_id WHERE ac.arrangement_id = " << arrangement_id;
    auto res = query(ss.str());
    if (res.empty()) return 0;
    double base = Utils::stringToDouble(res[0]["total"]) * quantity;
    if (!completion_date.empty()) {
        int days = Utils::daysDifference(order_date, completion_date);
        auto cfg = query("SELECT config_value FROM config WHERE config_key = 'SURCHARGE_1_DAY_PERCENT'");
        double surch1 = cfg.empty() ? 25 : Utils::stringToDouble(cfg[0]["config_value"]);
        cfg = query("SELECT config_value FROM config WHERE config_key = 'SURCHARGE_2_DAY_PERCENT'");
        double surch2 = cfg.empty() ? 15 : Utils::stringToDouble(cfg[0]["config_value"]);
        if (days <= 1) base += base * surch1 / 100;
        else if (days <= 2) base += base * surch2 / 100;
    }
    return base;
}

bool Database::addOrderWithCost(int user_id, int arrangement_id, int quantity, const std::string& order_date, const std::string& completion_date) {
    std::stringstream ss;
    ss << "INSERT INTO orders (user_id, arrangement_id, quantity, order_date, completion_date, total_price) VALUES ("
        << user_id << "," << arrangement_id << "," << quantity << ",'"
        << order_date << "',";
    if (completion_date.empty()) ss << "NULL,NULL)";
    else ss << "'" << completion_date << "',NULL)";
    if (!exec(ss.str())) return false;
    double cost = calcOrderCost(arrangement_id, quantity, order_date, completion_date);
    auto last = query("SELECT last_insert_rowid() as id");
    if (!last.empty()) {
        int oid = Utils::stringToInt(last[0]["id"]);
        std::stringstream up;
        up << "UPDATE orders SET total_price = " << cost << " WHERE order_id = " << oid;
        exec(up.str());
    }
    return true;
}

bool Database::checkPriceIncrease(int flower_id, double new_price) {
    auto old = query("SELECT price_per_unit FROM flowers WHERE flower_id = " + std::to_string(flower_id));
    if (old.empty()) return false;
    double old_price = Utils::stringToDouble(old[0]["price_per_unit"]);
    double inc = ((new_price - old_price) / old_price) * 100;
    if (inc > 10) {
        std::stringstream ss;
        ss << "SELECT a.name FROM arrangement_compositions ac "
            << "JOIN arrangements a ON ac.arrangement_id = a.arrangement_id "
            << "WHERE ac.flower_id = " << flower_id;
        auto affected = query(ss.str());
        if (!affected.empty()) {
            std::cout << "\nWARNING: Price increase would affect these arrangements:\n";
            for (auto& row : affected) {
                std::cout << "  - " << row["name"] << "\n";
            }
            return false;
        }
    }
    return true;
}

bool Database::updateFlowerPrice(int flower_id, double new_price) {
    std::stringstream ss;
    ss << "UPDATE flowers SET price_per_unit = " << new_price << " WHERE flower_id = " << flower_id;
    return exec(ss.str());
}

std::vector<std::map<std::string, std::string>> Database::getOrdersByPeriod(const std::string& start, const std::string& end) {
    std::stringstream ss;
    ss << "SELECT o.order_id, u.last_name||' '||u.first_name as customer, a.name, o.quantity, "
        << "o.order_date, o.completion_date, o.total_price FROM orders o "
        << "JOIN users u ON o.user_id=u.user_id JOIN arrangements a ON o.arrangement_id=a.arrangement_id "
        << "WHERE o.order_date BETWEEN '" << start << "' AND '" << end << "' AND o.total_price NOT NULL";
    return query(ss.str());
}

std::vector<std::map<std::string, std::string>> Database::getMostPopularArrangement() {
    return query("SELECT a.arrangement_id, a.name, COUNT(o.order_id) as orders, SUM(o.quantity) as total_quantity, "
        "SUM(o.total_price) as revenue FROM arrangements a LEFT JOIN orders o ON a.arrangement_id=o.arrangement_id "
        "WHERE o.total_price NOT NULL GROUP BY a.arrangement_id ORDER BY orders DESC LIMIT 1");
}

std::vector<std::map<std::string, std::string>> Database::getOrdersByUrgency() {
    return query("SELECT CASE WHEN julianday(completion_date)-julianday(order_date)<=1 THEN '1 day (25%)' "
        "WHEN julianday(completion_date)-julianday(order_date)<=2 THEN '2 days (15%)' "
        "ELSE '>2 days (0%)' END as urgency, COUNT(*) as count, SUM(total_price) as total "
        "FROM orders WHERE completion_date NOT NULL AND total_price NOT NULL GROUP BY urgency");
}

std::vector<std::map<std::string, std::string>> Database::getFlowersUsage(const std::string& start, const std::string& end) {
    std::stringstream ss;
    ss << "SELECT f.name, f.variety, SUM(ac.quantity * o.quantity) as used FROM orders o "
        << "JOIN arrangement_compositions ac ON o.arrangement_id=ac.arrangement_id "
        << "JOIN flowers f ON ac.flower_id=f.flower_id "
        << "WHERE o.order_date BETWEEN '" << start << "' AND '" << end << "' "
        << "GROUP BY f.flower_id ORDER BY f.name, f.variety";
    return query(ss.str());
}

std::vector<std::map<std::string, std::string>> Database::getArrangementsSales(const std::string& start, const std::string& end) {
    std::stringstream ss;
    ss << "SELECT a.name, SUM(o.quantity) as sold, SUM(o.total_price) as revenue FROM orders o "
        << "JOIN arrangements a ON o.arrangement_id=a.arrangement_id "
        << "WHERE o.order_date BETWEEN '" << start << "' AND '" << end << "' AND o.total_price NOT NULL "
        << "GROUP BY a.arrangement_id ORDER BY revenue DESC";
    return query(ss.str());
}

std::vector<std::map<std::string, std::string>> Database::getOrdersByDate(const std::string& date) {
    std::stringstream ss;
    ss << "SELECT o.order_id, u.last_name||' '||u.first_name as customer, a.name, o.quantity, "
        << "o.order_date, o.completion_date, o.total_price FROM orders o "
        << "JOIN users u ON o.user_id=u.user_id JOIN arrangements a ON o.arrangement_id=a.arrangement_id "
        << "WHERE o.order_date = '" << date << "'";
    return query(ss.str());
}

std::vector<std::map<std::string, std::string>> Database::getAllFlowers() {
    return query("SELECT * FROM flowers ORDER BY name, variety");
}

std::vector<std::map<std::string, std::string>> Database::getAllArrangements() {
    return query("SELECT * FROM arrangements");
}

std::vector<std::map<std::string, std::string>> Database::getArrangementComposition(int arrangement_id) {
    std::stringstream ss;
    ss << "SELECT f.name, f.variety, ac.quantity, f.price_per_unit, ac.quantity*f.price_per_unit as cost "
        << "FROM arrangement_compositions ac JOIN flowers f ON ac.flower_id=f.flower_id "
        << "WHERE ac.arrangement_id=" << arrangement_id;
    return query(ss.str());
}

std::vector<std::map<std::string, std::string>> Database::getUserOrders(int user_id) {
    std::stringstream ss;
    ss << "SELECT o.order_id, a.name, o.quantity, o.order_date, o.completion_date, o.total_price "
        << "FROM orders o JOIN arrangements a ON o.arrangement_id=a.arrangement_id "
        << "WHERE o.user_id=" << user_id << " ORDER BY o.order_date DESC";
    return query(ss.str());
}

std::vector<std::map<std::string, std::string>> Database::getAllOrders() {
    return query("SELECT o.order_id, u.last_name||' '||u.first_name as customer, a.name, o.quantity, "
        "o.order_date, o.completion_date, o.total_price FROM orders o "
        "JOIN users u ON o.user_id=u.user_id JOIN arrangements a ON o.arrangement_id=a.arrangement_id "
        "ORDER BY o.order_date DESC");
}

std::vector<std::map<std::string, std::string>> Database::getConfig() {
    return query("SELECT * FROM config");
}

bool Database::addFlower(const std::string& name, const std::string& variety, double price) {
    std::stringstream ss;
    ss << "INSERT INTO flowers (name, variety, price_per_unit) VALUES ('"
        << name << "','" << variety << "'," << price << ")";
    return exec(ss.str());
}

bool Database::addArrangement(const std::string& name) {
    std::stringstream ss;
    ss << "INSERT INTO arrangements (name) VALUES ('" << name << "')";
    return exec(ss.str());
}

bool Database::addToComposition(int arrangement_id, int flower_id, int quantity) {
    std::stringstream ss;
    ss << "INSERT INTO arrangement_compositions (arrangement_id, flower_id, quantity) VALUES ("
        << arrangement_id << "," << flower_id << "," << quantity << ")";
    return exec(ss.str());
}

bool Database::updateConfig(const std::string& key, const std::string& value) {
    std::stringstream ss;
    ss << "UPDATE config SET config_value = '" << value << "' WHERE config_key = '" << key << "'";
    return exec(ss.str());
}