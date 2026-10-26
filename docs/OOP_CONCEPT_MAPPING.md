# OOP Concept Mapping

This mapping describes concepts present in the active engine at `backend/parking.cpp`.

| Concept | Implementation | Viva explanation |
|---|---|---|
| Classes and objects | `Slot` at `backend/parking.cpp:14`; `ParkingLot` at `:32`; `ParkingLot lot` in `main` at `:202` | A `Slot` models one bay; `ParkingLot` owns and coordinates ten slot objects. |
| Encapsulation / information hiding | `ParkingLot` keeps `slots`, `dataPath`, and `historyPath` private at `backend/parking.cpp:33–36`; public operations expose parking behavior | Callers cannot directly edit the live lot array through the public interface. `Slot` remains a small data class whose fields are currently public. |
| Data members and member functions | `Slot` fields at `backend/parking.cpp:16–20` and `setCategory` at `:25`; `ParkingLot` operations include `loadData` at `:62`, `parkVehicle` at `:119`, `exitVehicle` at `:135`, and `showStatus` at `:152` | State is stored in objects and changed/read through related operations. |
| Constructors and constructor parameters | `Slot()` at `backend/parking.cpp:23` initializes defaults; `ParkingLot(const string& data, const string& history)` at `:52` selects storage paths, with defaults for normal execution | The default constructor supports normal operation, while tests can supply isolated paths. |
| Default arguments | `ParkingLot` constructor at `backend/parking.cpp:52` defaults to the application data and history paths | The object can be created without specifying paths in the normal app. |
| `this` pointer | Not explicitly used | No method needs explicit `this`; the compiler supplies it implicitly for member calls. |
| Static data/member functions | `ParkingLot::validRegistration` at `backend/parking.cpp:88`, `ParkingLot::compatible` at `:95`, and `ParkingLot::calculateFee` at `:111` are static helpers | They depend on arguments and domain rules, not on a particular lot object's mutable state. |
| Array of objects | `Slot slots[TOTAL_SLOTS]` at `backend/parking.cpp:34` | The fixed lot is represented by an array of ten `Slot` objects. |
| Dynamic object creation and memory management | No explicit `new`/`delete`; local `ParkingLot` object and standard file streams use automatic lifetime / RAII | Resources are released when scoped objects leave scope. |
| Friend functions | Not used | No external function needs privileged access to private lot state. |
| Method overloading | Not used | No artificial overloads were added. |
| Destructors and resource management | No custom destructor; `ifstream`/`ofstream` are local automatic objects | Standard streams close through their destructors, including on scope exit. |
| Separation of responsibilities | `Slot::setCategory`; `ParkingLot` handles persistence, allocation, search, fee calculation, and status | Slot initialization is local to the slot; multi-slot behavior belongs to the lot manager. |

