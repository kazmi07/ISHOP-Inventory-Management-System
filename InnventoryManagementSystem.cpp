#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <map>
#include <functional>
#include <iterator>
#include <sstream>

class InsufficientStockException : public std::runtime_error {
private:
    std::string itemName;
    int requested;
    int available;
    
public:
    InsufficientStockException(const std::string& name, int req, int avail)
        : std::runtime_error("Insufficient stock available"),
          itemName(name), requested(req), available(avail) {}
    
    const char* what() const noexcept override {
        static std::string msg = "Insufficient stock for " + itemName + 
                                 ": Requested " + std::to_string(requested) + 
                                 ", Available " + std::to_string(available);
        return msg.c_str();
    }
};

class InvalidPriceException : public std::invalid_argument {
public:
    InvalidPriceException(double price)
        : std::invalid_argument("Price cannot be negative: " + std::to_string(price)) {}
};

class InvalidDiscountException : public std::invalid_argument {
public:
    InvalidDiscountException(double discount)
        : std::invalid_argument("Discount must be between 0-100: " + std::to_string(discount)) {}
};

class FileIOException : public std::runtime_error {
public:
    FileIOException(const std::string& filename, const std::string& operation)
        : std::runtime_error("File operation failed: " + operation + " on " + filename) {}
};

class Product {
protected:
    std::string productId;
    std::string name;
    std::string category;
    double price;
    int stock;
    static int totalProducts;
    
public:
    Product(const std::string& id, const std::string& n, const std::string& cat, 
            double p, int s = 0)
        : productId(id), name(n), category(cat), stock(s) {
        if (p < 0) {
            throw InvalidPriceException(p);
        }
        price = p;
        totalProducts++;
    }
    
    virtual ~Product() {
        totalProducts--;
    }
    
    virtual void display() const {
        std::cout << "ID: " << productId 
                  << " | Name: " << name 
                  << " | Category: " << category
                  << " | Price: Rs." << price 
                  << " | Stock: " << stock;
    }
    
    virtual double calculateDiscountedPrice(double discount) const {
        if (discount < 0 || discount > 100) {
            throw InvalidDiscountException(discount);
        }
        return price * (1 - discount / 100.0);
    }
    
    virtual std::string getType() const = 0;
    
    virtual std::string toCSV() const = 0;
    virtual void fromCSV(const std::string& csvLine) = 0;
    
    std::string getId() const { return productId; }
    std::string getName() const { return name; }
    std::string getCategory() const { return category; }
    double getPrice() const { return price; }
    int getStock() const { return stock; }
    
    void setPrice(double newPrice) {
        if (newPrice < 0) {
            throw InvalidPriceException(newPrice);
        }
        price = newPrice;
    }
    
    void updateStock(int quantity) {
        int newStock = stock + quantity;
        if (newStock < 0) {
            throw InsufficientStockException(name, -quantity, stock);
        }
        stock = newStock;
    }
    
    static int getTotalProducts() {
        return totalProducts;
    }
    
    friend void displayProductDetails(const Product& p);
};

int Product::totalProducts = 0;

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

class Clothing : public Product {
private:
    std::string size;
    std::string color;
    std::string material;
    
public:
    Clothing(const std::string& id = "", const std::string& n = "", double p = 0, int s = 0,
             const std::string& sz = "", const std::string& col = "", const std::string& mat = "")
        : Product(id, n, "Clothing", p, s), size(sz), color(col), material(mat) {}
    
    void display() const override {
        Product::display();
        std::cout << " | Size: " << size 
                  << " | Color: " << color 
                  << " | Material: " << material;
    }
    
    std::string getType() const override {
        return "Clothing";
    }
    
    std::string toCSV() const override {
        std::stringstream ss;
        ss << "Clothing," << productId << "," << name << "," << price << "," 
           << stock << "," << size << "," << color << "," << material;
        return ss.str();
    }
    
    void fromCSV(const std::string& csvLine) override {
        auto tokens = split(csvLine, ',');
        if (tokens.size() >= 8 && tokens[0] == "Clothing") {
            productId = tokens[1];
            name = tokens[2];
            price = std::stod(tokens[3]);
            stock = std::stoi(tokens[4]);
            size = tokens[5];
            color = tokens[6];
            material = tokens[7];
        }
    }
    
    std::string getSize() const { return size; }
    std::string getColor() const { return color; }
    std::string getMaterial() const { return material; }
};

class Stationery : public Product {
private:
    std::string brand;
    std::string itemType;
    
public:
    Stationery(const std::string& id = "", const std::string& n = "", double p = 0, int s = 0,
               const std::string& br = "", const std::string& type = "")
        : Product(id, n, "Stationery", p, s), brand(br), itemType(type) {}
    
    void display() const override {
        Product::display();
        std::cout << " | Brand: " << brand 
                  << " | Type: " << itemType;
    }
    
    std::string getType() const override {
        return "Stationery";
    }
    
    std::string toCSV() const override {
        std::stringstream ss;
        ss << "Stationery," << productId << "," << name << "," << price << "," 
           << stock << "," << brand << "," << itemType;
        return ss.str();
    }
    
    void fromCSV(const std::string& csvLine) override {
        auto tokens = split(csvLine, ',');
        if (tokens.size() >= 7 && tokens[0] == "Stationery") {
            productId = tokens[1];
            name = tokens[2];
            price = std::stod(tokens[3]);
            stock = std::stoi(tokens[4]);
            brand = tokens[5];
            itemType = tokens[6];
        }
    }
};

class Accessory : public Product {
private:
    bool isElectronic;
    std::string accessoryType;
    
public:
    Accessory(const std::string& id = "", const std::string& n = "", double p = 0, int s = 0,
              bool electronic = false, const std::string& type = "")
        : Product(id, n, "Accessory", p, s), 
          isElectronic(electronic), accessoryType(type) {}
    
    void display() const override {
        Product::display();
        std::cout << " | Type: " << accessoryType 
                  << " | Electronic: " << (isElectronic ? "Yes" : "No");
    }
    
    double calculateDiscountedPrice(double discount) const override {
        double baseDiscount = discount;
        if (!isElectronic) {
            baseDiscount += 5;
        }
        return Product::calculateDiscountedPrice(baseDiscount);
    }
    
    std::string getType() const override {
        return "Accessory";
    }
    
    std::string toCSV() const override {
        std::stringstream ss;
        ss << "Accessory," << productId << "," << name << "," << price << "," 
           << stock << "," << (isElectronic ? "1" : "0") << "," << accessoryType;
        return ss.str();
    }
    
    void fromCSV(const std::string& csvLine) override {
        auto tokens = split(csvLine, ',');
        if (tokens.size() >= 7 && tokens[0] == "Accessory") {
            productId = tokens[1];
            name = tokens[2];
            price = std::stod(tokens[3]);
            stock = std::stoi(tokens[4]);
            isElectronic = (tokens[5] == "1");
            accessoryType = tokens[6];
        }
    }
};

template<typename T>
class Inventory {
private:
    std::vector<T> products;
    std::string inventoryName;
    
public:
    Inventory(const std::string& name) : inventoryName(name) {}
    
    void addProduct(T product) {
        products.push_back(product);
    }
    
    void removeProduct(const std::string& id) {
        auto it = std::remove_if(products.begin(), products.end(),
            [&id](const T& p) { return p->getId() == id; });
        
        if (it != products.end()) {
            products.erase(it, products.end());
            std::cout << "Product " << id << " removed successfully.\n";
        } else {
            std::cout << "Product not found.\n";
        }
    }
    
    T findProduct(const std::string& id) {
        for (auto& p : products) {
            if (p->getId() == id) {
                return p;
            }
        }
        return nullptr;
    }
    
    void displayAll() const {
        std::cout << "\n=== " << inventoryName << " Inventory ===\n";
        if (products.empty()) {
            std::cout << "No products in inventory.\n";
            return;
        }
        
        for (const auto& p : products) {
            p->display();
            std::cout << "\n";
        }
    }
    
    std::vector<T> filterProducts(std::function<bool(const T)> condition) {
        std::vector<T> result;
        std::copy_if(products.begin(), products.end(), 
                     std::back_inserter(result), condition);
        return result;
    }
    
    int getTotalStock() const {
        return std::accumulate(products.begin(), products.end(), 0,
            [](int sum, const T& p) { return sum + p->getStock(); });
    }
    
    double getTotalValue() const {
        return std::accumulate(products.begin(), products.end(), 0.0,
            [](double sum, const T& p) { return sum + (p->getPrice() * p->getStock()); });
    }
    
    const std::vector<T>& getAllProducts() const {
        return products;
    }
    
    void saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw FileIOException(filename, "save");
        }
        
        for (const auto& product : products) {
            file << product->toCSV() << "\n";
        }
        file.close();
    }
    
    void loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return;
        }
        
        products.clear();
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            auto tokens = split(line, ',');
            if (tokens.empty()) continue;
            
            std::string type = tokens[0];
            Product* product = nullptr;
            
            if (type == "Clothing") {
                product = new Clothing();
            } else if (type == "Stationery") {
                product = new Stationery();
            } else if (type == "Accessory") {
                product = new Accessory();
            }
            
            if (product) {
                product->fromCSV(line);
                products.push_back(product);
            }
        }
        file.close();
    }
};

class OrderItem {
private:
    Product* product;
    int quantity;
    double unitPrice;
    
public:
    OrderItem(Product* p, int qty) 
        : product(p), quantity(qty), unitPrice(p->getPrice()) {}
    
    double getTotal() const {
        return unitPrice * quantity;
    }
    
    void display() const {
        std::cout << product->getName() << " x " << quantity 
                  << " @ Rs." << unitPrice 
                  << " = Rs." << getTotal() << "\n";
    }
    
    Product* getProduct() const { return product; }
    int getQuantity() const { return quantity; }
    
    std::string toCSV() const {
        std::stringstream ss;
        ss << product->getId() << "," << quantity << "," << unitPrice;
        return ss.str();
    }
};

class Order {
private:
    static int orderCounter;
    int orderId;
    std::string customerName;
    std::vector<OrderItem> items;
    double totalAmount;
    time_t orderDate;
    
public:
    Order(const std::string& customer = "") 
        : customerName(customer), totalAmount(0) {
        orderId = ++orderCounter;
        orderDate = time(nullptr);
    }
    
    void addItem(Product* product, int quantity) {
        if (quantity <= 0) {
            throw std::invalid_argument("Quantity must be positive");
        }
        
        if (product->getStock() < quantity) {
            throw InsufficientStockException(product->getName(), quantity, product->getStock());
        }
        
        items.emplace_back(product, quantity);
        totalAmount += items.back().getTotal();
        product->updateStock(-quantity);
    }
    
    void display() const {
        std::cout << "\n=== Order Details ===\n";
        std::cout << "Order ID: " << orderId << "\n";
        std::cout << "Customer: " << customerName << "\n";
        std::cout << "Date: " << ctime(&orderDate);
        std::cout << "\nItems:\n";
        
        for (const auto& item : items) {
            std::cout << "  ";
            item.display();
        }
        
        std::cout << "\nTotal Amount: Rs." << totalAmount << "\n";
        std::cout << "====================\n";
    }
    
    double getTotalAmount() const { return totalAmount; }
    int getOrderId() const { return orderId; }
    
    std::string toCSV() const {
        std::stringstream ss;
        ss << orderId << "," << customerName << "," << totalAmount << "," 
           << orderDate << "," << items.size();
        
        for (const auto& item : items) {
            ss << "," << item.toCSV();
        }
        
        return ss.str();
    }
    
    void fromCSV(const std::string& csvLine, Inventory<Product*>& inventory) {
        auto tokens = split(csvLine, ',');
        if (tokens.size() >= 5) {
            orderId = std::stoi(tokens[0]);
            customerName = tokens[1];
            totalAmount = std::stod(tokens[2]);
            orderDate = std::stol(tokens[3]);
            int itemCount = std::stoi(tokens[4]);
            
            int index = 5;
            for (int i = 0; i < itemCount; i++) {
                if (index + 2 < tokens.size()) {
                    std::string productId = tokens[index];
                    int quantity = std::stoi(tokens[index + 1]);
                    Product* product = inventory.findProduct(productId);
                    if (product) {
                        OrderItem item(product, quantity);
                        items.push_back(item);
                    }
                    index += 3;
                }
            }
            
            if (orderId > orderCounter) {
                orderCounter = orderId;
            }
        }
    }
};

int Order::orderCounter = 1000;

class InventoryStatistics {
public:
    template<typename T>
    static void generateReport(const Inventory<T>& inventory) {
        std::cout << "\n=== Inventory Statistics ===\n";
        
        const auto& products = inventory.getAllProducts();
        
        if (products.empty()) {
            std::cout << "No products in inventory.\n";
            return;
        }
        
        std::map<std::string, int> categoryCount;
        for (const auto& p : products) {
            categoryCount[p->getCategory()]++;
        }
        
        std::cout << "Products by Category:\n";
        for (const auto& pair : categoryCount) {
            std::cout << "  " << pair.first << ": " << pair.second << " products\n";
        }
        
        auto maxPriceIt = std::max_element(products.begin(), products.end(),
            [](const T& a, const T& b) {
                return a->getPrice() < b->getPrice();
            });
        
        if (maxPriceIt != products.end()) {
            std::cout << "Most Expensive Product: " << (*maxPriceIt)->getName() 
                      << " (Rs." << (*maxPriceIt)->getPrice() << ")\n";
        }
        
        std::cout << "Total Products: " << Product::getTotalProducts() << "\n";
        std::cout << "Total Stock Value: Rs." << inventory.getTotalValue() << "\n";
        std::cout << "Total Stock Quantity: " << inventory.getTotalStock() << "\n";
    }
};

void displayProductDetails(const Product& p) {
    std::cout << "\n=== Detailed Product Information ===\n";
    std::cout << "Product ID: " << p.productId << "\n";
    std::cout << "Name: " << p.name << "\n";
    std::cout << "Category: " << p.category << "\n";
    std::cout << "Price: Rs." << p.price << "\n";
    std::cout << "Current Stock: " << p.stock << "\n";
    std::cout << "===============================\n";
}

class iShopApp {
private:
    Inventory<Product*> mainInventory;
    std::vector<Order> orders;
    std::map<int, std::pair<std::string, void (iShopApp::*)()>> menuOptions;
    
    void initializeMenu() {
        menuOptions[1] = {"Add Product", &iShopApp::addProduct};
        menuOptions[2] = {"Display Inventory", &iShopApp::displayInventory};
        menuOptions[3] = {"Create Order", &iShopApp::createOrder};
        menuOptions[4] = {"View Orders", &iShopApp::viewOrders};
        menuOptions[5] = {"Generate Report", &iShopApp::generateReport};
        menuOptions[6] = {"Filter Products", &iShopApp::filterProducts};
        menuOptions[7] = {"Apply Discount", &iShopApp::applyDiscount};
        menuOptions[8] = {"Save Data", &iShopApp::saveData};
        menuOptions[9] = {"Load Data", &iShopApp::loadData};
        menuOptions[10] = {"Exit", &iShopApp::exitApp};
    }
    
    void saveData() {
        try {
            mainInventory.saveToFile("products.txt");
            saveOrdersToFile("orders.txt");
            std::cout << "Data saved successfully!\n";
        } catch (const std::exception& e) {
            std::cerr << "Error saving data: " << e.what() << "\n";
        }
    }
    
    void loadData() {
        try {
            mainInventory.loadFromFile("products.txt");
            loadOrdersFromFile("orders.txt");
            std::cout << "Data loaded successfully!\n";
        } catch (const std::exception& e) {
            std::cerr << "Error loading data: " << e.what() << "\n";
        }
    }
    
    void saveOrdersToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw FileIOException(filename, "save");
        }
        
        for (const auto& order : orders) {
            file << order.toCSV() << "\n";
        }
        file.close();
    }
    
    void loadOrdersFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return;
        }
        
        orders.clear();
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            Order order;
            order.fromCSV(line, mainInventory);
            orders.push_back(order);
        }
        file.close();
    }
    
public:
    iShopApp() : mainInventory("iShop - IBA Karachi") {
        initializeMenu();
    }
    
    void run() {
        loadData();
        
        int choice;
        do {
            displayMenu();
            std::cout << "Enter your choice: ";
            std::cin >> choice;
            
            if (menuOptions.find(choice) != menuOptions.end()) {
                if (choice == 10) {
                    (this->*menuOptions[choice].second)();
                    break;
                } else {
                    (this->*menuOptions[choice].second)();
                }
            } else {
                std::cout << "Invalid choice. Please try again.\n";
            }
            
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore();
            std::cin.get();
            
        } while (true);
    }
    
    void displayMenu() {
        std::cout << "\n=================================\n";
        std::cout << "     iShop Inventory System\n";
        std::cout << "     IBA Karachi Merch Store\n";
        std::cout << "=================================\n";
        
        for (const auto& option : menuOptions) {
            std::cout << option.first << ". " << option.second.first << "\n";
        }
        std::cout << "=================================\n";
    }
    
    void addProduct() {
        std::cout << "\n=== Add New Product ===\n";
        std::cout << "1. Clothing\n2. Stationery\n3. Accessory\n";
        std::cout << "Select product type: ";
        
        int type;
        std::cin >> type;
        
        std::string id, name;
        double price;
        int stock;
        
        std::cout << "Enter Product ID: ";
        std::cin >> id;
        std::cout << "Enter Product Name: ";
        std::cin.ignore();
        std::getline(std::cin, name);
        std::cout << "Enter Price: ";
        std::cin >> price;
        std::cout << "Enter Initial Stock: ";
        std::cin >> stock;
        
        try {
            switch(type) {
                case 1: {
                    std::string size, color, material;
                    std::cout << "Enter Size: ";
                    std::cin >> size;
                    std::cout << "Enter Color: ";
                    std::cin >> color;
                    std::cout << "Enter Material: ";
                    std::cin >> material;
                    
                    mainInventory.addProduct(new Clothing(id, name, price, stock, 
                                                          size, color, material));
                    break;
                }
                case 2: {
                    std::string brand, itemType;
                    std::cout << "Enter Brand: ";
                    std::cin >> brand;
                    std::cout << "Enter Item Type: ";
                    std::cin >> itemType;
                    
                    mainInventory.addProduct(new Stationery(id, name, price, stock, 
                                                            brand, itemType));
                    break;
                }
                case 3: {
                    std::string accessoryType;
                    char electronic;
                    std::cout << "Is Electronic (Y/N): ";
                    std::cin >> electronic;
                    std::cout << "Enter Accessory Type: ";
                    std::cin >> accessoryType;
                    
                    mainInventory.addProduct(new Accessory(id, name, price, stock, 
                                                          (electronic == 'Y' || electronic == 'y'), 
                                                          accessoryType));
                    break;
                }
                default:
                    std::cout << "Invalid type selected.\n";
                    return;
            }
            
            std::cout << "Product added successfully!\n";
            
        } catch (const std::exception& e) {
            std::cerr << "Error adding product: " << e.what() << "\n";
        }
    }
    
    void displayInventory() {
        mainInventory.displayAll();
        std::cout << "\nTotal Products in System: " << Product::getTotalProducts() << "\n";
    }
    
    void createOrder() {
        std::string customerName;
        std::cout << "\n=== Create New Order ===\n";
        std::cout << "Enter Customer Name: ";
        std::cin.ignore();
        std::getline(std::cin, customerName);
        
        Order order(customerName);
        char addMore;
        
        do {
            std::string productId;
            int quantity;
            
            std::cout << "Enter Product ID: ";
            std::cin >> productId;
            std::cout << "Enter Quantity: ";
            std::cin >> quantity;
            
            Product* product = mainInventory.findProduct(productId);
            if (product) {
                try {
                    order.addItem(product, quantity);
                    std::cout << "Item added to order.\n";
                } catch (const std::exception& e) {
                    std::cerr << "Error adding item: " << e.what() << "\n";
                }
            } else {
                std::cout << "Product not found.\n";
            }
            
            std::cout << "Add another item? (Y/N): ";
            std::cin >> addMore;
            
        } while (addMore == 'Y' || addMore == 'y');
        
        orders.push_back(order);
        order.display();
    }
    
    void viewOrders() {
        if (orders.empty()) {
            std::cout << "No orders placed yet.\n";
            return;
        }
        
        std::cout << "\n=== All Orders ===\n";
        for (const auto& order : orders) {
            order.display();
        }
    }
    
    void generateReport() {
        InventoryStatistics::generateReport(mainInventory);
    }
    
    void filterProducts() {
        std::cout << "\n=== Filter Products ===\n";
        std::cout << "1. By Category\n2. By Price Range\n3. Low Stock (<10)\n";
        std::cout << "Select filter option: ";
        
        int option;
        std::cin >> option;
        
        std::vector<Product*> filtered;
        
        switch(option) {
            case 1: {
                std::string category;
                std::cout << "Enter category (Clothing/Stationery/Accessory): ";
                std::cin >> category;
                
                filtered = mainInventory.filterProducts(
                    [&category](Product* p) { 
                        return p->getCategory() == category; 
                    });
                break;
            }
            case 2: {
                double minPrice, maxPrice;
                std::cout << "Enter minimum price: ";
                std::cin >> minPrice;
                std::cout << "Enter maximum price: ";
                std::cin >> maxPrice;
                
                filtered = mainInventory.filterProducts(
                    [minPrice, maxPrice](Product* p) { 
                        return p->getPrice() >= minPrice && p->getPrice() <= maxPrice; 
                    });
                break;
            }
            case 3: {
                filtered = mainInventory.filterProducts(
                    [](Product* p) { 
                        return p->getStock() < 10; 
                    });
                break;
            }
            default:
                std::cout << "Invalid option.\n";
                return;
        }
        
        if (filtered.empty()) {
            std::cout << "No products match the filter criteria.\n";
        } else {
            std::cout << "\n=== Filtered Products ===\n";
            for (const auto& p : filtered) {
                p->display();
                std::cout << "\n";
            }
            std::cout << "Total: " << filtered.size() << " products\n";
        }
    }
    
    void applyDiscount() {
        std::cout << "\n=== Apply Discount ===\n";
        std::string productId;
        double discount;
        
        std::cout << "Enter Product ID: ";
        std::cin >> productId;
        std::cout << "Enter Discount Percentage: ";
        std::cin >> discount;
        
        Product* product = mainInventory.findProduct(productId);
        if (product) {
            try {
                double discountedPrice = product->calculateDiscountedPrice(discount);
                std::cout << "Original Price: Rs." << product->getPrice() << "\n";
                std::cout << "Discounted Price (" << discount << "% off): Rs." 
                          << discountedPrice << "\n";
            } catch (const std::exception& e) {
                std::cerr << "Error applying discount: " << e.what() << "\n";
            }
        } else {
            std::cout << "Product not found.\n";
        }
    }
    
    void exitApp() {
        char choice;
        std::cout << "\nSave data before exiting? (Y/N): ";
        std::cin >> choice;
        
        if (choice == 'Y' || choice == 'y') {
            saveData();
        }
        
        std::cout << "\nThank you for using iShop Inventory System!\n";
        std::cout << "IBA Karachi Merch Store - See you again!\n";
    }
    
    ~iShopApp() {
        const auto& products = mainInventory.getAllProducts();
        for (auto p : products) {
            delete p;
        }
    }
};

int main() {
    try {
        iShopApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}