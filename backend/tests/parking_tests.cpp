#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#define private public
#define PARKING_TESTING
#include "../parking.cpp"

int main() {
    const std::string data = "parkpilot_test_data.txt";
    const std::string history = "parkpilot_test_history.txt";
    std::remove(data.c_str());
    std::remove(history.c_str());
    ParkingLot lot(data, history);
    lot.loadData();

    assert(lot.findEmptySlot("Bike") == 0);
    assert(lot.findEmptySlot("Car") == 3);
    assert(lot.findEmptySlot("Truck") == 3);
    assert(lot.findEmptySlot("Accessible") == 8);
    assert(lot.findEmptySlot("Invalid") == -1);
    assert(ParkingLot::compatible("Bike", "Bike"));
    assert(!ParkingLot::compatible("Bike", "Car"));
    assert(ParkingLot::compatible("Car", "Truck"));
    assert(ParkingLot::calculateFee("Car", 0) == 20);
    assert(ParkingLot::calculateFee("Car", 3600) == 20);
    assert(ParkingLot::calculateFee("Car", 3601) == 40);
    assert(ParkingLot::calculateFee("Bike", 1) == 10);
    assert(ParkingLot::calculateFee("Truck", 1) == 40);
    assert(ParkingLot::calculateFee("Car", -1) == -1);
    assert(!ParkingLot::validRegistration("A!23"));
    assert(!ParkingLot::validRegistration("AB12CD"));
    assert(!ParkingLot::validRegistration("MH19PS10089"));
    assert(ParkingLot::validRegistration("PB10AB1234"));
    assert(ParkingLot::validRegistration("mh19ps1008"));
    assert(ParkingLot::validRegistration("22BH1234AA"));
    assert(!ParkingLot::validRegistration("22BH123"));

    std::ostringstream messages;
    std::streambuf* original = std::cout.rdbuf(messages.rdbuf());
    lot.parkVehicle("not-a-number", "Car");
    assert(lot.findVehicle("NOT-A-NUMBER") == -1);
    lot.parkVehicle("MH19PS1008", "Car");
    assert(lot.findVehicle("MH19PS1008") == 3);
    lot.parkVehicle("MH19PS1008", "Bike");
    assert(messages.str().find("Vehicle already parked") != std::string::npos);
    lot.slots[3].entryTime = static_cast<long>(std::time(0)) - 3601;
    lot.exitVehicle("MH19PS1008");
    assert(lot.findVehicle("MH19PS1008") == -1);
    assert(messages.str().find("FEE=40") != std::string::npos);

    for (int i = 0; i < TOTAL_SLOTS; ++i) {
        lot.slots[i].isOccupied = true;
        lot.slots[i].vehicleNumber = "FULL" + std::to_string(i);
        lot.slots[i].vehicleType = lot.slots[i].category;
        lot.slots[i].entryTime = static_cast<long>(std::time(0)) - 3601;
    }
    assert(lot.findEmptySlot("Bike") == -1);
    assert(lot.findEmptySlot("Car") == -1);
    assert(lot.findEmptySlot("Accessible") == -1);
    std::cout.rdbuf(original);

    std::remove(data.c_str());
    std::remove(history.c_str());
    std::cout << "ParkPilot C++ logic checks passed" << std::endl;
    return 0;
}

