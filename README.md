# Pointer-Based Parking Lot System in C++ 

This project implements a **pointer-based** multi-floor parking lot system 
supporting different vehicle types (Bike, Car, Truck) with varying parking 
space requirements.


## Low Level Design (LLD)

### Design Patterns Used
1. **Singleton Pattern**
   - ParkingLot class manages entire system
   - Thread-safe implementation with mutex

2. **Factory Method Pattern**
   - Vehicle creation based on type (Bike, Car, Truck)
   - Easily extensible for new vehicle types

3. **Composite Pattern**
   - Hierarchical structure: ParkingLot → Floors → ParkingSpots
   - Natural representation of physical layout

4. **Strategy Pattern**
   - Different parking strategies per vehicle type
   - Bikes/Cars: 1 spot
   - Trucks: 2 consecutive spots

### Class Structure
```plaintext
┌─────────────┐      ┌──────────┐      ┌──────────────┐
│ ParkingLot  │      │  Floor   │      │ ParkingSpot  │
├─────────────┤  1:n ├──────────┤  1:n ├──────────────┤
│-floors      │━━━━━▶│-spots    │━━━━━▶│-floorNumber  │
│-vehiclesMap │      │-floorNum │      │-spotNumber   │
│-mutex       │      │          │      │-isOccupied   │
└─────────────┘      └──────────┘      └──────────────┘
       ▲                                     ▲
       │                                     │
       │            ┌─────────┐              │
       └────────────│Vehicle  │────────────-─┘
                    ├─────────┤
                    │-type    │
                    │-license │
                    └─────────┘
```
## Key Points:
1. Floors are stored as `Floor*` in a `vector<Floor*>`.
2. ParkingSpots are stored as `ParkingSpot*` in a `vector<ParkingSpot*>` within each Floor.
3. Vehicles are dynamically allocated (`new Vehicle(...)`) in `main` and tracked in a map to be properly deleted on removal.


## How to Build:
    g++ -o parkinglot pointer_parkinglot.cpp

## Run:
    ./parkinglot

## Usage:
- park_vehicle <license_plate> <vehicle_type>
- remove_vehicle <license_plate>
- available_spots
- is_full
- find_vehicle <license_plate>
- exit

### Example:
    park_vehicle KA-01-1234 Car
    remove_vehicle KA-01-1234
    park_vehicle KA-02-1234 Truck
    exit

## Thread-Safe Example:
```cpp
#include <thread>

int main() {
    ParkingLot parkingLot(3, 10);  // 3 floors, 10 spots each

    auto worker1 = [&parkingLot]() {
        Vehicle v1("KA-01-1111", VehicleType::Car);
        parkingLot.parkVehicle(&v1);
    };

    auto worker2 = [&parkingLot]() {
        Vehicle v2("KA-02-2222", VehicleType::Bike);
        parkingLot.parkVehicle(&v2);
    };

    // Spawn threads
    std::thread t1(worker1);
    std::thread t2(worker2);

    t1.join();
    t2.join();

    // Check results
    parkingLot.findVehicle("KA-01-1111");
    parkingLot.findVehicle("KA-02-2222");

    return 0;
}
```

## Interactive Example Session:
```bash
$ ./parkinglot
Parking Lot System
Commands:
  park_vehicle <license_plate> <vehicle_type>
  remove_vehicle <license_plate>
  available_spots
  is_full
  find_vehicle <license_plate>
  exit

Enter command: park_vehicle KA-01-1234 Car
Parked KA-01-1234 on floor 0 at spot(s): 0

Enter command: park_vehicle KA-02-5678 Truck
Parked KA-02-5678 on floor 0 at spot(s): 1 2

Enter command: available_spots
Floor 0: 8 spots available.
Floor 1: 10 spots available.
Floor 2: 10 spots available.

Enter command: find_vehicle KA-02-5678
Vehicle KA-02-5678 is parked on floor 0 at spot(s): 1 2

Enter command: is_full
Parking lot has available spots.

Enter command: remove_vehicle KA-02-5678
Vehicle KA-02-5678 removed from floor 0

Enter command: exit