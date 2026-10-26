const http = require("http");
const { execFile } = require("child_process");
const fs = require("fs");
const path = require("path");

const PORT = Number(process.env.PORT) || 3000;
const isWindows = process.platform === "win32";
const PARKING_EXE = path.join(__dirname, "backend", isWindows ? "parking.exe" : "parking");
const FRONTEND_DIR = path.join(__dirname, "frontend");

function runParkingProgram(args) {
  return new Promise((resolve, reject) => {
    execFile(PARKING_EXE, args, { cwd: path.join(__dirname, "backend"), timeout: 5000 }, (error, stdout, stderr) => {
      if (error) reject(new Error(String(stderr || error.message).trim()));
      else resolve(stdout.trim());
    });
  });
}

function parseSimpleFormat(line) {
  return Object.fromEntries(line.split(";").map((part) => {
    const separator = part.indexOf("=");
    return separator < 0 ? [part.trim(), ""] : [part.slice(0, separator).trim(), part.slice(separator + 1).trim()];
  }));
}

function parseListFormat(line) {
  if (!line) return [];
  return line.split("|").filter(Boolean).map((entry) => Object.fromEntries(entry.split(",").map((part) => {
    const separator = part.indexOf("=");
    return separator < 0 ? [part.trim(), ""] : [part.slice(0, separator).trim(), part.slice(separator + 1).trim()];
  })));
}

function historyToCSV(records) {
  const headers = ["Event", "Slot", "Vehicle Number", "Vehicle Type", "Entry Time", "Exit Time", "Fee (Rs)"];
  const rows = [headers.join(",")];
  records.forEach((r) => {
    const date = (stamp) => stamp && stamp !== "0" ? new Date(Number(stamp) * 1000).toLocaleString() : "";
    rows.push([r.EVENT, r.SLOT, r.VEHICLE, r.TYPE, date(r.ENTRY), date(r.EXIT), r.FEE]
      .map((value) => `"${String(value ?? "").replace(/"/g, '""')}"`).join(","));
  });
  return rows.join("\r\n");
}

function sendJSON(res, status, value) {
  res.writeHead(status, { "Content-Type": "application/json; charset=utf-8", "X-Content-Type-Options": "nosniff" });
  res.end(JSON.stringify(value));
}

function readJSON(req) {
  return new Promise((resolve, reject) => {
    let body = "";
    req.on("data", (chunk) => {
      body += chunk;
      if (body.length > 8192) { reject(new Error("Request body is too large")); req.destroy(); }
    });
    req.on("end", () => {
      try { resolve(JSON.parse(body || "{}")); }
      catch (_) { reject(new Error("Request body must be valid JSON")); }
    });
    req.on("error", reject);
  });
}

function serveStaticFile(req, res) {
  let pathname;
  try { pathname = decodeURIComponent(new URL(req.url, "http://localhost").pathname || "/"); }
  catch (_) { res.writeHead(400); res.end("Invalid path"); return; }
  const relative = pathname === "/" ? "index.html" : pathname.replace(/^[/\\]+/, "");
  const filePath = path.resolve(FRONTEND_DIR, relative);
  if (!filePath.startsWith(FRONTEND_DIR + path.sep) && filePath !== path.join(FRONTEND_DIR, "index.html")) {
    res.writeHead(403); res.end("Forbidden"); return;
  }
  const types = { ".html": "text/html; charset=utf-8", ".css": "text/css; charset=utf-8", ".js": "application/javascript; charset=utf-8" };
  fs.readFile(filePath, (error, content) => {
    if (error) { res.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" }); res.end("File not found"); return; }
    res.writeHead(200, { "Content-Type": types[path.extname(filePath)] || "application/octet-stream", "X-Content-Type-Options": "nosniff" });
    res.end(content);
  });
}

const server = http.createServer(async (req, res) => {
  const parsed = new URL(req.url, "http://localhost");
  try {
    if (req.method === "GET" && parsed.pathname === "/api/status") {
      sendJSON(res, 200, parseSimpleFormat(await runParkingProgram(["status"]))); return;
    }
    if (req.method === "GET" && parsed.pathname === "/api/list") {
      sendJSON(res, 200, { vehicles: parseListFormat(await runParkingProgram(["list"])) }); return;
    }
    if (req.method === "GET" && parsed.pathname === "/api/history") {
      sendJSON(res, 200, { history: parseListFormat(await runParkingProgram(["history"])) }); return;
    }
    if (req.method === "GET" && parsed.pathname === "/api/history/download") {
      const records = parseListFormat(await runParkingProgram(["history"]));
      res.writeHead(200, { "Content-Type": "text/csv; charset=utf-8", "Content-Disposition": 'attachment; filename="parking_history.csv"' });
      res.end(historyToCSV(records)); return;
    }
    if (req.method === "POST" && parsed.pathname === "/api/history/clear") {
      sendJSON(res, 200, parseSimpleFormat(await runParkingProgram(["clearhistory"]))); return;
    }
    if (req.method === "GET" && parsed.pathname === "/api/search") {
      sendJSON(res, 200, parseSimpleFormat(await runParkingProgram(["search", String(parsed.searchParams.get("vehicleNumber") || "").slice(0, 32)]))); return;
    }
    if (req.method === "POST" && parsed.pathname === "/api/park") {
      const data = await readJSON(req);
      const output = await runParkingProgram(["park", String(data.vehicleNumber || "").slice(0, 32), String(data.vehicleType || "").slice(0, 20)]);
      sendJSON(res, 200, parseSimpleFormat(output)); return;
    }
    if (req.method === "POST" && parsed.pathname === "/api/exit") {
      const data = await readJSON(req);
      sendJSON(res, 200, parseSimpleFormat(await runParkingProgram(["exit", String(data.vehicleNumber || "").slice(0, 32)]))); return;
    }
    if (parsed.pathname.startsWith("/api/")) { sendJSON(res, 404, { FAIL: "NOT_FOUND", MESSAGE: "API route not found" }); return; }
    serveStaticFile(req, res);
  } catch (error) {
    if (!res.headersSent) sendJSON(res, 400, { FAIL: "REQUEST_ERROR", MESSAGE: error.message || "Request failed" });
  }
});

server.listen(PORT, () => console.log(`ParkPilot is running at http://localhost:${PORT}`));

