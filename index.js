import express from "express";
import cors from "cors";
import { fileURLToPath } from "url";
import { dirname, join } from "path";
import http from "http";
import WebSocket, { WebSocketServer } from "ws";

const app = express();
const port = process.env.PORT || 3001;
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// Middleware setup
app.use(cors());
app.use(express.json());
app.use(express.static(join(__dirname, "./public")));

// Serve the HTML file from the public folder
app.get("/", (req, res) => {
  res.sendFile(join(__dirname, "./public/index.html"));
});

// Create an HTTP server
const server = http.createServer(app);

// Create a WebSocket server
const wss = new WebSocketServer({ server });

// WebSocket handling
wss.on("connection", (ws) => {
  console.log("A client connected.");

  // Send a welcome message to the client
  ws.send("Welcome to the WebSocket server!");

  // Handle incoming messages from the client
  ws.on("message", (message) => {
    console.log(`Received from ESP32: ${message}`);
  });

  // Handle client disconnection
  ws.on("close", () => {
    console.log("Client disconnected.");
  });
});

// Start the server
server.listen(8080, () => {
  console.log("HTTP and WebSocket server running on http://localhost:8080/");
});

app.listen(port, () => {
  console.log(`Express server running on port ${port}`);
});
