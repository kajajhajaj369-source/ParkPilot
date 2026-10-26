#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

const int TOTAL_SLOTS = 10;
const string DATA_FILE = "../data/parking_data.txt";
const string HISTORY_FILE = "../data/parking_history.txt";

class Slot {
public:
    int slotNumber;
    bool isOccupied;
    string vehicleNumber;
    string vehicleType;
    string category;
    long entryTime;

    Slot() : slotNumber(0), isOccupied(false), vehicleNumber("-"),
             vehicleType("-"), category("Car"), entryTime(0) {}

    void setCategory(int number) {
        slotNumber = number;
        category = number <= 3 ? "Bike" : (number <= 8 ? "Car" : "Accessible");
    }
};

class ParkingLot {
private:
    Slot slots[TOTAL_SLOTS];
    string dataPath;
    string historyPath;

    void clearSlot(int index) {
        slots[index].isOccupied = false;
        slots[index].vehicleNumber = "-";
        slots[index].vehicleType = "-";
        slots[index].entryTime = 0;
    }

    void logHistory(const string& event, const Slot& slot, long exitTime, int fee) const {
        ofstream file(historyPath.c_str(), ios::app);
        if (file) file << event << " " << slot.slotNumber << " " << slot.vehicleNumber << " "
                       << slot.vehicleType << " " << slot.entryTime << " " << exitTime << " " << fee << endl;
    }

public:
    explicit ParkingLot(const string& data = DATA_FILE, const string& history = HISTORY_FILE)
        : dataPath(data), historyPath(history) { setupEmptySlots(); }

    void setupEmptySlots() {
        for (int i = 0; i < TOTAL_SLOTS; ++i) {
            slots[i].setCategory(i + 1);
            clearSlot(i);
        }
    }

    void loadData() {
        setupEmptySlots();
        ifstream file(dataPath.c_str());
        int number, occupied;
        string vehicle, type;
        long entry;
        while (file >> number >> occupied >> vehicle >> type >> entry) {
            int index = number - 1;
            if (index >= 0 && index < TOTAL_SLOTS && occupied == 1 && entry > 0 &&
                compatible(slots[index].category, type)) {
                slots[index].isOccupied = true;
                slots[index].vehicleNumber = vehicle;
                slots[index].vehicleType = type;
                slots[index].entryTime = entry;
            }
        }
    }

    void saveData() const {
        ofstream file(dataPath.c_str(), ios::trunc);
        if (!file) return;
        for (int i = 0; i < TOTAL_SLOTS; ++i)
            file << slots[i].slotNumber << " " << (slots[i].isOccupied ? 1 : 0) << " "
                 << slots[i].vehicleNumber << " " << slots[i].vehicleType << " " << slots[i].entryTime << endl;
    }

    static bool validRegistration(const string& plate) {
        string value = plate;
        transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(toupper(c)); });
        const size_t length = value.size();

        // Bharat series: YY BH #### XX (the final series may contain one or two letters).
        if (length >= 9 && length <= 10 && isdigit(static_cast<unsigned char>(value[0])) &&
            isdigit(static_cast<unsigned char>(value[1])) && value.substr(2, 2) == "BH" &&
            isdigit(static_cast<unsigned char>(value[4])) && isdigit(static_cast<unsigned char>(value[5])) &&
            isdigit(static_cast<unsigned char>(value[6])) && isdigit(static_cast<unsigned char>(value[7]))) {
            bool validSeries = true;
            for (size_t i = 8; i < length; ++i) validSeries = validSeries && isalpha(static_cast<unsigned char>(value[i]));
            return validSeries;
        }

        // State code + one/two digit RTO + one/three letter series + one/four digits.
        if (length < 6 || length > 11 || !isalpha(static_cast<unsigned char>(value[0])) ||
            !isalpha(static_cast<unsigned char>(value[1]))) return false;
        size_t index = 2;
        size_t rtoDigits = 0;
        while (index < length && rtoDigits < 2 && isdigit(static_cast<unsigned char>(value[index]))) { ++index; ++rtoDigits; }
        if (rtoDigits == 0) return false;
        size_t seriesLetters = 0;
        while (index < length && seriesLetters < 3 && isalpha(static_cast<unsigned char>(value[index]))) { ++index; ++seriesLetters; }
        if (seriesLetters == 0) return false;
        size_t registrationDigits = 0;
        while (index < length && registrationDigits < 4 && isdigit(static_cast<unsigned char>(value[index]))) { ++index; ++registrationDigits; }
        return registrationDigits > 0 && index == length;
    }

    static bool compatible(const string& category, const string& type) {
        return (category == type && (type == "Bike" || type == "Accessible")) || (category == "Car" && (type == "Car" || type == "Truck"));
    }

    int findEmptySlot(const string& type) const {
        for (int i = 0; i < TOTAL_SLOTS; ++i)
            if (!slots[i].isOccupied && compatible(slots[i].category, type)) return i;
        return -1;
    }

    int findVehicle(const string& vehicle) const {
        for (int i = 0; i < TOTAL_SLOTS; ++i)
            if (slots[i].isOccupied && slots[i].vehicleNumber == vehicle) return i;
        return -1;
    }

    static int calculateFee(const string& type, long durationSeconds) {
        if (durationSeconds < 0) return -1;
        int rate = type == "Bike" ? 10 : (type == "Truck" ? 40 : (type == "Accessible" || type == "Car" ? 20 : 0));
        if (!rate) return -1;
        long hours = max(1L, (durationSeconds + 3599) / 3600);
        return static_cast<int>(hours * rate);
    }

    void parkVehicle(string vehicle, const string& type) {
        transform(vehicle.begin(), vehicle.end(), vehicle.begin(), [](unsigned char c) { return static_cast<char>(toupper(c)); });
        if (!validRegistration(vehicle)) { cout << "FAIL;MESSAGE=Enter a valid Indian vehicle registration number" << endl; return; }
        if (type != "Bike" && type != "Car" && type != "Truck" && type != "Accessible") { cout << "FAIL;MESSAGE=Choose a supported vehicle category" << endl; return; }
        if (findVehicle(vehicle) != -1) { cout << "FAIL;MESSAGE=Vehicle already parked" << endl; return; }
        int index = findEmptySlot(type);
        if (index == -1) { cout << "FAIL;MESSAGE=No compatible slot available" << endl; return; }
        slots[index].isOccupied = true;
        slots[index].vehicleNumber = vehicle;
        slots[index].vehicleType = type;
        slots[index].entryTime = static_cast<long>(time(0));
        cout << "SUCCESS;SLOT=" << slots[index].slotNumber << ";VEHICLE=" << vehicle << ";TYPE=" << type
             << ";CATEGORY=" << slots[index].category << ";ENTRY=" << slots[index].entryTime << endl;
        logHistory("ENTRY", slots[index], 0, 0);
    }

    void exitVehicle(string vehicle) {
        transform(vehicle.begin(), vehicle.end(), vehicle.begin(), [](unsigned char c) { return static_cast<char>(toupper(c)); });
        if (!validRegistration(vehicle)) { cout << "FAIL;MESSAGE=Enter a valid Indian vehicle registration number" << endl; return; }
        int index = findVehicle(vehicle);
        if (index == -1) { cout << "FAIL;MESSAGE=Vehicle not found" << endl; return; }
        const long now = static_cast<long>(time(0));
        const long entry = slots[index].entryTime;
        if (entry <= 0 || now < entry) { cout << "FAIL;MESSAGE=Invalid parking timestamps" << endl; return; }
        const long seconds = now - entry;
        const int fee = calculateFee(slots[index].vehicleType, seconds);
        Slot completed = slots[index];
        cout << "SUCCESS;SLOT=" << completed.slotNumber << ";VEHICLE=" << vehicle << ";TYPE=" << completed.vehicleType
             << ";DURATION=" << seconds / 60 << ";DURATION_SECONDS=" << seconds << ";FEE=" << fee
             << ";RATE=" << (completed.vehicleType == "Bike" ? 10 : (completed.vehicleType == "Truck" ? 40 : 20)) << endl;
        logHistory("EXIT", completed, now, fee);
        clearSlot(index);
    }

    void showStatus() const {
        int occupied = 0;
        for (int i = 0; i < TOTAL_SLOTS; ++i) if (slots[i].isOccupied) ++occupied;
        cout << "TOTAL=" << TOTAL_SLOTS << ";AVAILABLE=" << TOTAL_SLOTS - occupied << ";OCCUPIED=" << occupied
             << ";OCCUPANCY=" << (occupied * 100 / TOTAL_SLOTS) << ";ACTIVE=" << occupied << endl;
    }

    void listVehicles() const {
        bool first = true;
        for (int i = 0; i < TOTAL_SLOTS; ++i) if (slots[i].isOccupied) {
            if (!first) cout << "|";
            cout << "SLOT=" << slots[i].slotNumber << ",VEHICLE=" << slots[i].vehicleNumber << ",TYPE=" << slots[i].vehicleType
                 << ",CATEGORY=" << slots[i].category << ",ENTRY=" << slots[i].entryTime;
            first = false;
        }
        cout << endl;
    }

    void searchVehicle(string vehicle) const {
        transform(vehicle.begin(), vehicle.end(), vehicle.begin(), [](unsigned char c) { return static_cast<char>(toupper(c)); });
        if (!validRegistration(vehicle)) { cout << "FAIL;MESSAGE=Enter a valid Indian vehicle registration number" << endl; return; }
        int index = findVehicle(vehicle);
        if (index == -1) { cout << "FAIL;MESSAGE=Vehicle not found" << endl; return; }
        cout << "SUCCESS;SLOT=" << slots[index].slotNumber << ";VEHICLE=" << slots[index].vehicleNumber << ";TYPE="
             << slots[index].vehicleType << ";CATEGORY=" << slots[index].category << ";ENTRY=" << slots[index].entryTime << endl;
    }

    void printHistory() const {
        ifstream file(historyPath.c_str());
        string event, vehicle, type;
        int slot, fee;
        long entry, exit;
        bool first = true;
        while (file >> event >> slot >> vehicle >> type >> entry >> exit >> fee) {
            if (!first) cout << "|";
            cout << "EVENT=" << event << ",SLOT=" << slot << ",VEHICLE=" << vehicle << ",TYPE=" << type
                 << ",ENTRY=" << entry << ",EXIT=" << exit << ",FEE=" << fee;
            first = false;
        }
        cout << endl;
    }

    void clearHistory() const {
        ofstream file(historyPath.c_str(), ios::trunc);
        if (!file) { cout << "FAIL;MESSAGE=Could not clear history" << endl; return; }
        cout << "SUCCESS;MESSAGE=History cleared" << endl;
    }
};

#ifndef PARKING_TESTING
int main(int argc, char* argv[]) {
    ParkingLot lot;
    lot.loadData();
    if (argc < 2) { cout << "FAIL;MESSAGE=No command given" << endl; return 0; }
    const string command = argv[1];
    if (command == "status") lot.showStatus();
    else if (command == "list") lot.listVehicles();
    else if (command == "history") lot.printHistory();
    else if (command == "clearhistory") lot.clearHistory();
    else if (command == "search" && argc >= 3) lot.searchVehicle(argv[2]);
    else if (command == "park" && argc >= 4) { lot.parkVehicle(argv[2], argv[3]); lot.saveData(); }
    else if (command == "exit" && argc >= 3) { lot.exitVehicle(argv[2]); lot.saveData(); }
    else cout << "FAIL;MESSAGE=Invalid command or missing arguments" << endl;
    return 0;
}
#endif

