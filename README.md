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
## Key Pointer Changes:
1. Floors are stored as `Floor*` in a `vector<Floor*>`.
2. ParkingSpots are stored as `ParkingSpot*` in a `vector<ParkingSpot*>` within each Floor.
3. Vehicles are dynamically allocated (`new Vehicle(...)`) in `main` and tracked in a map to be properly deleted on removal.

## Thread-Safe:
- Uses a global `std::mutex` in the `ParkingLot` to protect shared data from concurrent access.

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

