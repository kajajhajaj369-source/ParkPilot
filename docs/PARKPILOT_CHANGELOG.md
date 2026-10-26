# ParkPilot Change Log and Initial Audit

## Initial architecture

- Browser UI: `frontend/index.html`, `frontend/style.css`, and `frontend/script.js`; plain HTML/CSS/JavaScript.
- HTTP bridge: `server.js` uses Node.js built-in `http`, `child_process`, `fs`, and `path` modules. It serves `frontend/` and maps `/api/status`, `/api/list`, `/api/history`, `/api/search`, `/api/park`, `/api/exit`, and history management/export endpoints to CLI commands.
- Parking engine: `backend/parking.cpp` is compiled and invoked once per request. It owns a fixed `Slot[10]` array, allocation/search/exit/billing logic, and plain text persistence.
- Data: `data/parking_data.txt` stores live slot state; `data/parking_history.txt` stores append-only ENTRY/EXIT records (until the clear-history endpoint is used). The checked-in working data currently contains sample records; preserve it during implementation and testing.
- Request flow: browser `fetch` -> `server.js` route -> `execFile` of `backend/parking(.exe)` -> delimited stdout -> JSON response -> DOM update.

## Existing features and implementation

- Entry, duplicate detection, first-empty-slot allocation, exit, duration-based fee, and vehicle search: `backend/parking.cpp`.
- Status counts and live vehicle list: `backend/parking.cpp` and `frontend/script.js`.
- Entry/exit history and CSV export: backend `logHistory`/`printHistory`, `/api/history*` routes in `server.js`, history panel in `frontend/index.html` and `frontend/script.js`.
- Slot map, counters, forms, fee card, and history table: `frontend/index.html`, `frontend/script.js`, `frontend/style.css`.
- Root-level `index.html`, `script.js`, `style.css`, and `parking.cpp` duplicate older variants. The server serves `frontend/` and executes `backend/parking.cpp`; root copies are not in the live request path. The root C++ copy also references `HISTORY_FILE` without defining it and should not be selected as the build source.

## Build, run, and configuration

- Requirements documented in README: Node.js 14+ and `g++`; no npm dependencies or database.
- Build from `backend/`: `g++ -std=c++11 -o parking.exe parking.cpp` on Windows (or `g++ -std=c++11 -o parking parking.cpp` on Unix-like systems). `server.js` expects the platform-specific binary in `backend/`.
- Run from the project root: `node server.js`, then browse to `http://localhost:3000`.
- Configuration: port is hardcoded to 3000 in `server.js`; lot size and rates are hardcoded in `backend/parking.cpp`; storage paths are relative to `backend/`. No environment variables, database, npm scripts beyond `start`, or test suite are present.
- Existing compiled `backend/parking.exe` is present, so the server may work without rebuilding, but it can become stale after source edits.

## OOP concepts present

`backend/parking.cpp` has `Slot` and `ParkingLot` classes, a fixed array of objects, constructors, member functions, and basic separation between slot state and lot operations. `Slot` data is public, so encapsulation/information hiding is weak. There is no dynamic allocation, destructor-based resource handling, static member, method overloading, or justified friend function. Standard file streams already use scope-based resource management.

## Initial issues and risks

- Vehicle types are not validated and all vehicles use the first free slot; there are no slot categories or compatibility rules.
- Fee duration truncates seconds to whole minutes; negative/corrupt timestamps are not rejected. Rates include an undocumented fallback type.
- The checked-in root source is stale and does not compile as written; avoid confusing it with the active backend.
- The static-file path is built directly from the request path, request bodies have no size/error handling, and all JSON responses allow wildcard CORS. Frontend table rows interpolate persisted/user-provided strings into `innerHTML`.
- History clear is destructive and currently uses browser alerts/debug output for feedback. Do not clear existing history during verification.
- No formal tests or license file were found. Git metadata/history is unavailable in this workspace, so upstream provenance cannot be verified from repository history. README contains a generic learning-use statement rather than a standard license.

## Planned incremental changes

1. Keep the existing HTTP + CLI + text-file architecture and record this audit before implementation.
2. Standardize on the active `backend/` and `frontend/` paths; retain the unused root copies unless a targeted compatibility/documentation correction is needed.
3. Rename visible branding and package metadata to ParkPilot without changing API routes or storage formats.
4. Add slot categories and deterministic compatible allocation while retaining legacy persisted slot rows safely.
5. Validate registration/type/timestamps, calculate duration from timestamps with documented partial-hour rounding, and expose session duration/billing details.
6. Derive dashboard occupancy/active-session metrics from the live engine state; improve search/history feedback and safe rendering.
7. Add focused C++ logic checks and document only OOP concepts actually present in `docs/OOP_CONCEPT_MAPPING.md`.
8. Update setup, architecture, limitations, test instructions, and attribution notes in README. Preserve existing data and note unverifiable upstream history rather than claiming original authorship.

## Files likely to change

- `backend/parking.cpp`: slot model, validation, allocation, billing, status/search outputs, and testable logic boundary.
- `server.js`: request validation/error responses and existing API output translation if needed.
- `frontend/index.html`, `frontend/script.js`, `frontend/style.css`: ParkPilot branding, slot categories, analytics, feedback, and history/search presentation.
- `package.json`, `README.md`: project metadata and accurate Windows setup/testing/attribution.
- `docs/OOP_CONCEPT_MAPPING.md`: verified source-to-syllabus mapping.
- `docs/PARKPILOT_CHANGELOG.md`: audit and incremental change record.

## Baseline verification status

The repository contains no declared tests. The Git commands `git status` and `git log` fail because this workspace has no Git repository metadata. Baseline build and runtime checks will be recorded after the audit is saved; checks must use isolated temporary data so the existing `data/` records are preserved.
## Implemented changes

- Project branding and package metadata now use ParkPilot. The old `/api/*` route names and text-file paths remain stable.
- `backend/parking.cpp` now keeps the slot array private, assigns categories by bay number, accepts bikes only in bays 1–3, cars/trucks only in bays 4–8, and accessible vehicles only in bays 9–10. Selection is the first free compatible bay in slot order.
- Registration/type validation rejects malformed Indian registration numbers, unsupported entries, and duplicate active registrations. Standard state/RTO and Bharat (BH) series formats are accepted. Truck support and its ₹40/hour rate are preserved.
- Fees use elapsed seconds; each partial hour rounds up, minimum one hour. Future/invalid entry timestamps are refused at exit. Rates are Bike ₹10, Car ₹20, Truck ₹40, Accessible ₹20 per hour.
- Status output now includes occupied, available, percentage occupancy, and active sessions derived from the same loaded slot array. Search returns category and entry timestamp.
- `server.js` supports `PORT`, bounds JSON body size, handles malformed JSON, sets execution timeout, safely resolves static paths, and correctly escapes CSV quotes.
- The active dashboard is ParkPilot-branded, shows the computed occupancy/session metrics and bay category, escapes persisted values before rendering, and provides inline entry/exit/search feedback. Responsive rules support narrow screens.
- Added `backend/tests/parking_tests.cpp` and a cross-platform Node test runner. Added `docs/OOP_CONCEPT_MAPPING.md` and updated README setup, pricing, limitations, and attribution notes.

## Verification results

- `npm run build:backend`: passed with g++  C++11.
- `npm test`: passed; compatible/incompatible allocation, no compatible space, registration validation, fees at 0/3600/3601 seconds, duplicate entry, successful exit, and exit billing were checked using temporary files.
- `node --check server.js` and `node --check frontend/script.js`: passed.
- Browser static page request: returned the ParkPilot dashboard and four vehicle category options.
- End-to-end `/api/status` and `/api/list`: not verified in this managed Windows environment. Node's `child_process.execFile` returned `spawn EPERM` for the freshly compiled native executable; invoking the same executable directly from PowerShell succeeded. This is an environment execution restriction observed during verification, not evidence that the complete browser-to-engine flow passes.
- Existing checked-in `data/parking_data.txt` and `data/parking_history.txt` were not changed by tests; tests use temporary data/history paths and remove them.
- No repository test suite existed before this work. Git status/log remain unavailable because the supplied workspace has no `.git` metadata.

