# ParkPilot — Smart Parking Management System

ParkPilot is a college-level C++ OOP project that manages parking sessions through a small browser dashboard. The existing ParkWise codebase has been incrementally adapted; the existing Node/C++/text-file architecture and project data format are retained.

## Problem and objectives

Manually assigning parking bays makes it harder to track current occupancy, locate a vehicle, and calculate a consistent exit fee. ParkPilot provides a simple local dashboard for vehicle entry and exit, compatible slot allocation, live occupancy, search, and parking history.

## Features

- Park and exit motorcycles, cars, trucks, and accessible vehicles with duplicate-registration checks and Indian vehicle-number format validation (state/RTO registrations and BH series).
- Deterministic, first-compatible-slot allocation; slots 1–3 serve bikes, 4–8 cars and trucks, and 9–10 accessible vehicles.
- Live total, occupied, available, occupancy percentage, and active session metrics derived from persisted parking state.
- Registration search, current vehicle list, parking history, and CSV export.
- Duration-based hourly billing: bikes ₹10/hour, cars ₹20/hour, trucks ₹40/hour, accessible vehicles ₹20/hour.
- Text-file persistence; no database or third-party npm dependency.

## Architecture

```text
Browser (frontend/) --fetch--> Node HTTP server (server.js)
                                  |
                                  +--execFile--> C++ engine (backend/parking.cpp)
                                                       |
                                                       +--> data/parking_data.txt
                                                       +--> data/parking_history.txt
```

The browser calls the existing `/api/*` routes. Node serves the frontend and translates the C++ engine's simple delimited output to JSON. The C++ command-line process loads the current state, performs one operation, and saves changes. The root-level `index.html`, `script.js`, `style.css`, and `parking.cpp` are legacy copies; the server uses `frontend/` and `backend/parking.cpp`.

## Requirements and Windows setup

- Node.js 14 or later.
- A C++11 compiler available as `g++` (for example, MinGW-w64 or MSYS2).
- PowerShell or Command Prompt.

From the project directory:

```powershell
npm run build:backend
npm start
```

Then open [http://localhost:3000](http://localhost:3000). Set `PORT` to use a different HTTP port. The C++ engine expects to run with `backend/` as its working directory; the Node server does this automatically. Do not open the HTML file directly because API requests require the server.

To build manually on Windows:

```powershell
 g++ -std=c++11 -o backend/parking.exe backend/parking.cpp
```

## Tests

Run the focused C++ logic checks with:

```powershell
npm test
```

These checks cover compatible and incompatible slot selection, exhausted capacity, registration validation, and fee rounding. The test program uses temporary files and removes them; it does not access the checked-in parking records.

## Billing policy

The fee uses elapsed seconds between the saved entry timestamp and exit time. Any partial hour rounds up to the next whole hour, and every valid session has a one-hour minimum. A timestamp in the future or a missing/invalid saved entry timestamp is rejected at exit. Slot state stores the entry epoch time in seconds; history stores both entry and exit epoch times. Fee rates are currently constants in the C++ engine.

## OOP concepts

See [docs/OOP_CONCEPT_MAPPING.md](docs/OOP_CONCEPT_MAPPING.md) for the source-level mapping to Units 1–3. The code uses classes, encapsulated lot state, constructors, member functions, an array of objects, static utility functions, and scope-managed file streams. It does not use raw dynamic allocation or decorative friend functions.

## Limitations

- This is a local learning project, not a production parking or payment service. It has no authentication, multi-user coordination, or database transactions.
- The lot has ten fixed slots with category ranges defined in the C++ engine.
- The server uses plain HTTP and local text files; concurrent requests can race while saving state.
- Legacy root source/frontend copies remain for reference and are not used by the server.
- History clear permanently truncates the history file.

## Attribution and license

The workspace provided for this task has no license file or Git history, so the original upstream author and reuse terms could not be independently verified here. Existing source comments and the ParkWise README describe an inherited beginner parking project. This adaptation preserves that context and does not claim the original implementation was independently authored. Add the original project's required license/attribution before redistribution once its source and terms are confirmed. Node.js and the C++ standard library are third-party platform components and retain their respective licenses.

## Screenshots

No screenshots were present in the repository at the time of this update.

