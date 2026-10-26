const { spawnSync } = require("child_process");
const fs = require("fs");
const path = require("path");
const root = path.resolve(__dirname, "../..");
const executable = path.join(root, "backend", process.platform === "win32" ? "parking-tests.exe" : "parking-tests");
const compiler = spawnSync("g++", ["-std=c++11", "-o", executable, path.join(__dirname, "parking_tests.cpp")], { cwd: root, stdio: "inherit" });
if (compiler.error || compiler.status !== 0) process.exit(compiler.status || 1);
const result = spawnSync(executable, [], { cwd: root, stdio: "inherit" });
try { fs.unlinkSync(executable); } catch (_) {}
process.exit(result.status || 0);

