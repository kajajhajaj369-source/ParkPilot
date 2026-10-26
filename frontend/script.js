const API_BASE = "";
const $ = (id) => document.getElementById(id);

function updateClock() { $("liveClock").textContent = new Date().toLocaleTimeString(); }
setInterval(updateClock, 1000);
updateClock();
setTimeout(() => $("pole").classList.add("up"), 500);

const registrationInputs = ["vehicleNumberIn", "vehicleNumberOut", "searchInput"].map((id) => $(id));
registrationInputs.forEach((input) => input.addEventListener("input", () => { input.value = input.value.toUpperCase().replace(/[\s-]/g, ""); }));

function escapeHTML(value) {
  return String(value ?? "").replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;").replace(/'/g, "&#39;");
}
function formatEntryTime(seconds) {
  if (!seconds || seconds === "0") return "-";
  const date = new Date(Number(seconds) * 1000);
  return Number.isNaN(date.getTime()) ? "Invalid time" : date.toLocaleString();
}

async function loadStatus() {
  try {
    const response = await fetch(`${API_BASE}/api/status`);
    if (!response.ok) throw new Error("Status request failed");
    const data = await response.json();
    $("totalSlots").textContent = data.TOTAL ?? "--";
    $("availableSlots").textContent = data.AVAILABLE ?? "--";
    $("occupiedSlots").textContent = data.OCCUPIED ?? "--";
    $("occupancyValue").textContent = `${data.OCCUPANCY ?? "--"}%`;
    $("activeSessions").textContent = data.ACTIVE ?? "--";
  } catch (_) {
    ["totalSlots", "availableSlots", "occupiedSlots", "occupancyValue", "activeSessions"].forEach((id) => $(id).textContent = "Unavailable");
  }
}
async function loadVehicles() {
  try {
    const response = await fetch(`${API_BASE}/api/list`);
    if (!response.ok) throw new Error("Vehicle list request failed");
    const vehicles = (await response.json()).vehicles || [];
    $("vehicleTableBody").innerHTML = vehicles.length ? vehicles.map((v) => `<tr><td>#${escapeHTML(v.SLOT)}</td><td>${escapeHTML(v.VEHICLE)}</td><td>${escapeHTML(v.TYPE)}</td><td>${formatEntryTime(v.ENTRY)}</td></tr>`).join("") : '<tr><td colspan="4" class="table__empty">No active parking sessions. Enter a vehicle to get started.</td></tr>';
    const occupied = new Map(vehicles.map((v) => [Number(v.SLOT), v]));
    const total = Number($("totalSlots").textContent) || 10;
    $("slotMap").innerHTML = Array.from({ length: total }, (_, index) => {
      const number = index + 1;
      const vehicle = occupied.get(number);
      return `<div class="slot-bay ${vehicle ? "occupied" : "free"}" title="${escapeHTML(vehicle?.CATEGORY || (number <= 3 ? "Two-wheeler" : number <= 8 ? "Car" : "Accessible"))}">${number}</div>`;
    }).join("");
  } catch (_) {
    $("vehicleTableBody").innerHTML = '<tr><td colspan="4" class="table__empty">Could not load parked vehicles. Check the server and refresh.</td></tr>';
    $("slotMap").replaceChildren();
  }
}
async function loadHistory() {
  try {
    const response = await fetch(`${API_BASE}/api/history`);
    if (!response.ok) throw new Error("History request failed");
    const history = (await response.json()).history || [];
    const reversed = [...history].reverse();
    $("historyTableBody").innerHTML = reversed.length ? reversed.map((h) => {
      const isExit = h.EVENT === "EXIT";
      return `<tr><td>${isExit ? "Exit" : "Entry"}</td><td>#${escapeHTML(h.SLOT)}</td><td>${escapeHTML(h.VEHICLE)}</td><td>${escapeHTML(h.TYPE)}</td><td>${formatEntryTime(h.ENTRY)}</td><td>${isExit ? formatEntryTime(h.EXIT) : "-"}</td><td>${isExit ? `₹${escapeHTML(h.FEE)}` : "-"}</td></tr>`;
    }).join("") : '<tr><td colspan="7" class="table__empty">No parking history yet.</td></tr>';
  } catch (_) {
    $("historyTableBody").innerHTML = '<tr><td colspan="7" class="table__empty">Could not load parking history.</td></tr>';
  }
}
function refreshDashboard() { loadStatus(); loadVehicles(); loadHistory(); }

$("parkForm").addEventListener("submit", async (event) => {
  event.preventDefault();
  const vehicleNumber = $("vehicleNumberIn").value.trim().toUpperCase();
  const vehicleType = $("vehicleTypeIn").value;
  const result = $("parkResult");
  try {
    const response = await fetch(`${API_BASE}/api/park`, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ vehicleNumber, vehicleType }) });
    const data = await response.json();
    result.textContent = data.SUCCESS !== undefined ? `Parked in slot ${data.SLOT} (${data.CATEGORY})` : data.MESSAGE || "Could not park vehicle";
    result.className = `form__result ${data.SUCCESS !== undefined ? "ok" : "err"}`;
    if (data.SUCCESS !== undefined) { $("vehicleNumberIn").value = ""; refreshDashboard(); }
  } catch (_) { result.textContent = "Server error. Is server.js running?"; result.className = "form__result err"; }
});

$("exitForm").addEventListener("submit", async (event) => {
  event.preventDefault();
  const result = $("exitResult");
  try {
    const response = await fetch(`${API_BASE}/api/exit`, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ vehicleNumber: $("vehicleNumberOut").value.trim().toUpperCase() }) });
    const data = await response.json();
    if (data.SUCCESS !== undefined) {
      $("feeCard").hidden = false;
      $("feeValue").textContent = `₹${data.FEE}`;
      $("feeMeta").textContent = `Slot ${data.SLOT} · ${data.DURATION} minutes · rounded up to full hours`;
      result.textContent = "Vehicle exited and slot released.";
      result.className = "form__result ok";
      $("vehicleNumberOut").value = "";
      refreshDashboard();
    } else {
      $("feeCard").hidden = true;
      result.textContent = data.MESSAGE || "Vehicle not found";
      result.className = "form__result err";
    }
  } catch (_) { result.textContent = "Server error. Is server.js running?"; result.className = "form__result err"; }
});

$("searchForm").addEventListener("submit", async (event) => {
  event.preventDefault();
  const result = $("searchResult");
  try {
    const response = await fetch(`${API_BASE}/api/search?vehicleNumber=${encodeURIComponent($("searchInput").value.trim().toUpperCase())}`);
    const data = await response.json();
    result.textContent = data.SUCCESS !== undefined ? `Found in slot ${data.SLOT} (${data.CATEGORY || data.TYPE}), entered ${formatEntryTime(data.ENTRY)}` : data.MESSAGE || "Vehicle not found";
    result.className = `form__result ${data.SUCCESS !== undefined ? "ok" : "err"}`;
  } catch (_) { result.textContent = "Server error. Is server.js running?"; result.className = "form__result err"; }
});
$("refreshBtn").addEventListener("click", refreshDashboard);
$("clearHistoryBtn").addEventListener("click", async () => {
  if (!confirm("This will permanently delete all parking history. Continue?")) return;
  try {
    const response = await fetch(`${API_BASE}/api/history/clear`, { method: "POST" });
    const data = await response.json();
    if (!response.ok || data.FAIL) throw new Error(data.MESSAGE || "Could not clear history");
    await loadHistory();
  } catch (error) { alert(error.message || "Server error. Is server.js running?"); }
});
refreshDashboard();

