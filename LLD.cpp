#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include <sstream>
#include <thread>
using namespace std;

//------------------------------------------------------
// Enum to define vehicle types
enum class VehicleType {
    Bike,
    Car,
    Truck
};

//------------------------------------------------------
// Base Vehicle Class
class Vehicle {
public:
    string licensePlate;
    VehicleType type;
    
    Vehicle(const string& licensePlate, VehicleType type)
        : licensePlate(licensePlate), type(type) {}

    // Returns number of spots required.
    int getRequiredSpots() const {
        if (type == VehicleType::Truck)
            return 2;
        else
            return 1;
    }
};

//------------------------------------------------------
// ParkingSpot Class
class ParkingSpot {
public:
    int floorNumber;
    int spotNumber;
    bool isOccupied;
    string parkedVehicle; // License plate of parked vehicle (empty if none)

    ParkingSpot(int floorNumber, int spotNumber)
        : floorNumber(floorNumber), spotNumber(spotNumber), isOccupied(false) {}

    bool assignVehicle(const string& licensePlate) {
        if (isOccupied)
            return false;
        parkedVehicle = licensePlate;
        isOccupied = true;
        return true;
    }

    bool removeVehicle() {
        if (!isOccupied)
            return false;
        parkedVehicle.clear();
        isOccupied = false;
        return true;
    }
};

//------------------------------------------------------
// Floor Class: Each floor manages its parking spots.
class Floor {
public:
    int floorNumber;
    // Store parking spots as pointers
    vector<ParkingSpot*> spots;

    // Constructor
    Floor(int floorNumber, int numSpots)
        : floorNumber(floorNumber)
    {
        for (int i = 0; i < numSpots; ++i) {
            spots.push_back(new ParkingSpot(floorNumber, i));
        }
    }

    // Destructor to delete allocated ParkingSpot pointers
    ~Floor() {
        for (auto* spot : spots) {
            delete spot;
        }
        spots.clear();
    }

    // Find available spot(s) for a given vehicle.
    // Returns a vector of spot numbers if found; empty vector if not.
    vector<int> findAvailableSpots(const Vehicle* vehicle) {
        int required = vehicle->getRequiredSpots();
        vector<int> availableSpots;
        
        // For vehicles needing only 1 spot.
        if (required == 1) {
            for (auto* spot : spots) {
                if (!spot->isOccupied) {
                    availableSpots.push_back(spot->spotNumber);
                    return availableSpots; // return first available spot
                }
            }
        } 
        // For Truck: need 2 consecutive free spots.
        else if (required == 2) {
            for (size_t i = 0; i < spots.size() - 1; ++i) {
                if (!spots[i]->isOccupied && !spots[i+1]->isOccupied) {
                    availableSpots.push_back(spots[i]->spotNumber);
                    availableSpots.push_back(spots[i+1]->spotNumber);
                    return availableSpots;
                }
            }
        }
        return {}; // empty if not found
    }

    // Park vehicle in specified spots. Returns true if successful.
    bool parkVehicle(const Vehicle* vehicle, const vector<int>& spotNumbers) {
        // Verify that the spots are still available.
        for (int idx : spotNumbers) {
            if (idx < 0 || idx >= (int)spots.size() || spots[idx]->isOccupied)
                return false;
        }
        // Assign vehicle to the spots.
        for (int idx : spotNumbers) {
            spots[idx]->assignVehicle(vehicle->licensePlate);
        }
        return true;
    }

    // Remove vehicle from its spot(s). Returns true if vehicle was found.
    bool removeVehicle(const string& licensePlate) {
        bool removed = false;
        for (auto* spot : spots) {
            if (spot->isOccupied && spot->parkedVehicle == licensePlate) {
                spot->removeVehicle();
                removed = true;
            }
        }
        return removed;
    }

    // Count available spots on the floor.
    int availableSpotsCount() {
        int count = 0;
        for (auto* spot : spots) {
            if (!spot->isOccupied)
                count++;
        }
        return count;
    }
};

//------------------------------------------------------
// ParkingLot Class: Manages all floors and global operations.
class ParkingLot {
private:
    // Maps: licensePlate -> (floorNumber, spotNumbers)
    unordered_map<string, pair<int, vector<int>>> vehicleLocations;
    // Additional map to track actual Vehicle pointers (for memory cleanup)
    unordered_map<string, Vehicle*> vehiclesMap;

    // Mutex for concurrency:
    mutable std::mutex mtx;  // Protects access to vehicleLocations, vehiclesMap, and floors.

public:
    // Store floors as pointers
    vector<Floor*> floors;

    // Constructor
    ParkingLot(int numFloors, int spotsPerFloor) {
        for (int i = 0; i < numFloors; ++i) {
            floors.push_back(new Floor(i, spotsPerFloor));
        }
    }

    // Destructor to delete allocated Floor pointers
    ~ParkingLot() {
        for (auto* floor : floors) {
            delete floor;
        }
        floors.clear();
    }

    // Park a vehicle. Returns true if parked successfully.
    bool parkVehicle(Vehicle* vehicle) {
        // Lock the mutex to protect shared data.
        lock_guard<mutex> lock(mtx);

        // Check if vehicle is already parked.
        if (vehicleLocations.find(vehicle->licensePlate) != vehicleLocations.end()) {
            cout << "Vehicle " << vehicle->licensePlate << " is already parked." << endl;
            return false;
        }

        // Iterate floors to find available spot(s)
        for (auto* floor : floors) {
            vector<int> availableSpots = floor->findAvailableSpots(vehicle);
            if (!availableSpots.empty()) {
                if (floor->parkVehicle(vehicle, availableSpots)) {
                    // Save location: (floorNumber, spotNumbers)
                    vehicleLocations[vehicle->licensePlate] = {floor->floorNumber, availableSpots};
                    // Also store the Vehicle pointer for later cleanup
                    vehiclesMap[vehicle->licensePlate] = vehicle;

                    cout << "Parked " << vehicle->licensePlate << " on floor " 
                         << floor->floorNumber << " at spot(s): ";
                    for (int s : availableSpots)
                        cout << s << " ";
                    cout << endl;
                    return true;
                }
            }
        }

        cout << "Parking Lot Full or no suitable spot available for "
             << vehicle->licensePlate << endl;
        return false;
    }

    // Remove a vehicle based on license plate. Returns true if removed.
    bool removeVehicle(const string& licensePlate) {
        // Lock the mutex to protect shared data.
        lock_guard<mutex> lock(mtx);

        auto it = vehicleLocations.find(licensePlate);
        if (it == vehicleLocations.end()) {
            cout << "Vehicle " << licensePlate << " not found." << endl;
            return false;
        }
        int floorNumber = it->second.first;
        // Remove from floor.
        if (floors[floorNumber]->removeVehicle(licensePlate)) {
            vehicleLocations.erase(it);
            // Also delete the Vehicle pointer from vehiclesMap
            auto vt = vehiclesMap.find(licensePlate);
            if (vt != vehiclesMap.end()) {
                delete vt->second;       // Free the memory for this Vehicle
                vehiclesMap.erase(vt);
            }
            cout << "Vehicle " << licensePlate << " removed from floor " << floorNumber << endl;
            return true;
        }
        return false;
    }

    // Returns a vector of available spots count per floor.
    vector<int> getAvailableSpotsPerFloor() {
        // Lock the mutex to protect shared data.
        lock_guard<mutex> lock(mtx);

        vector<int> available;
        for (auto* floor : floors) {
            available.push_back(floor->availableSpotsCount());
        }
        return available;
    }

    // Checks if parking lot is full.
    bool isFull() {
        // Lock the mutex to protect shared data.
        lock_guard<mutex> lock(mtx);

        for (auto* floor : floors) {
            if (floor->availableSpotsCount() > 0)
                return false;
        }
        return true;
    }

    // Finds the vehicle location given a license plate.
    void findVehicle(const string& licensePlate) {
        // Lock the mutex to protect shared data.
        lock_guard<mutex> lock(mtx);

        auto it = vehicleLocations.find(licensePlate);
        if (it != vehicleLocations.end()) {
            auto& location = it->second; // (floorNumber, vector<int> of spots)
            cout << "Vehicle " << licensePlate << " is parked on floor " 
                 << location.first << " at spot(s): ";
            for (int s : location.second)
                cout << s << " ";
            cout << endl;
        } else {
            cout << "Vehicle " << licensePlate << " not found." << endl;
        }
    }
};

// Main function with a simple command terminal interface.
int main() {

    cout << "Enter the number of floors: ";
    int numFloors;
    cin>>numFloors;
    cout << "Enter the number of spots per floor: ";
    int spotsPerFloor;
    cin>>spotsPerFloor;   
    // Create ParkingLot on the stack (it manages Floor pointers internally)
    ParkingLot parkingLot(numFloors, spotsPerFloor);

    cout << "Parking Lot System" << endl;
    cout << "Commands:" << endl;
    cout << "  park_vehicle <license_plate> <vehicle_type>" << endl;
    cout << "  remove_vehicle <license_plate>" << endl;
    cout << "  available_spots" << endl;
    cout << "  is_full" << endl;
    cout << "  find_vehicle <license_plate>" << endl;
    cout << "  exit" << endl;

    string input;
    while (true) {
        cout << "\nEnter command: ";
        if (!std::getline(cin, input)) {
            break; // EOF or error
        }

        if (input.empty()) {
            continue;
        }

        istringstream iss(input);
        string command;
        iss >> command;

        if (command == "park_vehicle") {
            string license, typeStr;
            iss >> license >> typeStr;
            if (license.empty() || typeStr.empty()) {
                cout << "Invalid input. Usage: park_vehicle <license_plate> <vehicle_type>" << endl;
                continue;
            }

            VehicleType type;
            if (typeStr == "Bike")
                type = VehicleType::Bike;
            else if (typeStr == "Car")
                type = VehicleType::Car;
            else if (typeStr == "Truck")
                type = VehicleType::Truck;
            else {
                cout << "Unknown vehicle type." << endl;
                continue;
            }

            Vehicle* vehicle = new Vehicle(license, type);
            // Attempt to park in the lot
            parkingLot.parkVehicle(vehicle);

        }
        else if (command == "remove_vehicle") {
            string license;
            iss >> license;
            if (license.empty()) {
                cout << "Usage: remove_vehicle <license_plate>" << endl;
                continue;
            }
            parkingLot.removeVehicle(license);
        }
        else if (command == "available_spots") {
            vector<int> available = parkingLot.getAvailableSpotsPerFloor();
            for (size_t i = 0; i < available.size(); ++i) {
                cout << "Floor " << i << ": " << available[i] << " spots available." << endl;
            }
        }
        else if (command == "is_full") {
            if (parkingLot.isFull())
                cout << "Parking lot is full." << endl;
            else
                cout << "Parking lot has available spots." << endl;
        }
        else if (command == "find_vehicle") {
            string license;
            iss >> license;
            if (license.empty()) {
                cout << "Usage: find_vehicle <license_plate>" << endl;
                continue;
            }
            parkingLot.findVehicle(license);
        }
        else if (command == "exit") {
            break;
        }
        else {
            cout << "Invalid command." << endl;
        }
    }
    return 0;
}
