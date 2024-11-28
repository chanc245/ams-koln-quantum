import express from "express";
import cors from "cors";
import { fileURLToPath } from "url";
import { dirname, join } from "path";
import http from "http";
import { WebSocketServer } from "ws";
import { createRequire } from "module";

const require = createRequire(import.meta.url);
const player = require("play-sound")();

const app = express();
const port = process.env.PORT || 3001;
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

app.use(cors());
app.use(express.json());
app.use(express.static(join(__dirname, "./public")));

app.get("/", (req, res) => {
  res.sendFile(join(__dirname, "./public/index.html"));
});

const server = http.createServer(app);

const wss = new WebSocketServer({ server });

let isAudioPlaying = false;

// WebSocket
wss.on("connection", (ws) => {
  console.log("New client connected!");

  ws.on("message", (message) => {
    console.log(`Received: ${message}`);

    if (message.toString() === "1") {
      if (!isAudioPlaying) {
        console.log("Playing audio...");
        isAudioPlaying = true;

        player.play("./audio/audio_test2.mp3", (err) => {
          if (err) {
            console.error("Error playing audio:", err);
          } else {
            console.log("Audio finished playing.");
          }
          isAudioPlaying = false;
        });
      } else {
        console.log("Audio is already playing. Ignoring request.");
      }
    }
  });

  ws.on("close", () => {
    console.log("Client disconnected.");
  });
});

server.listen(8080, () => {
  console.log("HTTP and WebSocket server running on http://localhost:8080/");
});

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});
