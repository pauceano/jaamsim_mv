# Model ↔ View Message Catalog

## Architecture

```
MODEL PROCESS                          VIEW PROCESS
─────────────                          ────────────
HeadlessModel.java                     ViewApplication.java
JaamSimModel + Simulation              EntityStateCache + GUIFrame
         │                                    │
         │  PUB tcp:5556 ──────────────────►  │  (entity state, events, logs)
         │  ROUTER tcp:5557 ◄────────────────  │  (commands)
         │                                    │
```

All ZMQ messages are JSON-encoded, sent as two-part ZMQ frames: `[topic, payload]`.

---

## Model → View (PUB on tcp://*:5556)

### 1. simState — Full simulation snapshot

**Frequency:** Every 5 seconds (configurable via `ZmqConfig.SNAPSHOT_INTERVAL_MS`)

**Topic:** `simState`

```json
{
  "type": "simState",
  "simTime": 42.5,
  "runState": "RUNNING",
  "entities": [
    {
      "name": "Server1",
      "type": "Server",
      "position": [10.0, 0.0, 5.0],
      "size": [1.0, 1.0, 1.0],
      "outputs": {
        "QueueLength": 5,
        "Utilization": 0.75
      }
    }
  ]
}
```

| Field | Type | Description |
|-------|------|-------------|
| `type` | string | Always `"simState"` |
| `simTime` | number | Simulation time in seconds |
| `runState` | string | `"IDLE"`, `"RUNNING"`, or `"ENDED"` |
| `entities` | array | All active entities in the model |
| `entities[].name` | string | Entity name |
| `entities[].type` | string | Java class simple name |
| `entities[].position` | [number×3] | 3D world position |
| `entities[].size` | [number×3] | Bounding box size |
| `entities[].show` | string | `"true"` or `"false"` |
| `entities[].state` | string | StateEntity present state (if applicable) |
| `entities[].outputs` | object | Output name → numeric value |

**Used by:** `EntityStateCache` subscriber to populate EntityShadow cache for `RenderManager.collectProxies()`

---

### 2. entityState — Per-entity state delta

**Frequency:** Every 5 seconds, one message per active entity

**Topic:** `entityState`

```json
{
  "name": "Server1",
  "type": "Server",
  "position": [10.0, 0.0, 5.0],
  "size": [1.0, 1.0, 1.0],
  "show": "true",
  "state": "Working",
  "outputs": {
    "QueueLength": 5,
    "Utilization": 0.75
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| `name` | string | Entity name (cache key) |
| `type` | string | Java class simple name |
| `position` | [number×3] | 3D world position (meters) |
| `size` | [number×3] | Bounding box size (meters) |
| `show` | string | `"true"` or `"false"` — visibility |
| `state` | string | StateEntity present state (null if not StateEntity) |
| `outputs` | object | Output name → numeric value |

**Used by:** `EntityStateCache.parseAndStore()` — updates or creates `EntityShadow` in `ConcurrentHashMap`

---

### 3. runEvent — Run lifecycle events

**Frequency:** On state change (start → end, error)

**Topic:** `runEvent`

```json
{
  "type": "runEnded",
  "data": ""
}
```

```json
{
  "type": "runError",
  "data": "NullPointerException: ..."
}
```

| Field | Type | Description |
|-------|------|-------------|
| `type` | string | `"runEnded"` or `"runError"` |
| `data` | string | Empty for runEnded; error message for runError |

**Trigger:** `ZmqBridge` implements `RunListener` — calls `runEnded()` and `handleRuntimeError()`

---

### 4. logLine — Log messages

**Frequency:** Real-time, one message per log line

**Topic:** `logLine`

```json
{
  "line": "Server1: ServiceTime = 2.0 s"
}
```

| Field | Type | Description |
|-------|------|-------------|
| `line` | string | Log line text |

**Trigger:** `ZmqBridge` implements `LogListener` — calls `update()` on every `Log.logLine()`

---

## View → Model (ROUTER on tcp://*:5557)

### 5. PAUSE — Pause simulation

**Request:**
```json
{"cmd": "PAUSE"}
```

**Response:**
```json
{"status": "ok"}
```
```json
{"status": "error", "msg": "no model"}
```

**Handler:** `ZmqBridge.handlePause()` → `simModel.getGUIListener().pauseSimulation()`

---

### 6. SET_INPUT — Change entity input value

**Request:**
```json
{
  "cmd": "SET_INPUT",
  "entity": "Server1",
  "keyword": "ServiceTime",
  "value": "5.0 s"
}
```

**Response:**
```json
{"status": "ok"}
```
```json
{"status": "error", "msg": "entity not found: Server1"}
```

| Field | Type | Description |
|-------|------|-------------|
| `cmd` | string | Always `"SET_INPUT"` |
| `entity` | string | Target entity name |
| `keyword` | string | Input keyword to change |
| `value` | string | New value (JaamSim input syntax) |

**Handler:** `ZmqBridge.handleSetInput()` → `InputAgent.applyArgs(ent, keyword, value)`

---

## In-Process Bridges (not ZMQ — same JVM only)

These remain as direct Java method calls. They are NOT sent over ZMQ because they
are either synchronous queries (need immediate return values) or high-frequency
per-frame operations.

### GUIListener (Model → View)

| Method | Trigger | ZMQ? |
|--------|---------|------|
| `tickUpdate(long tick)` | Every simulation tick | ❌ Per-tick |
| `updateAll()` | After model edit | ❌ Synchronous |
| `gui_timeRunning()` | Play/pause toggle | ❌ Synchronous |
| `handleInputError(t, ent)` | Input validation fails | ❌ Carries Java objects |
| `pauseSimulation()` | — | ✅ Via PAUSE command |
| `getFontProvider()` | Text rendering | ❌ Synchronous query |
| `redraw()` | Viewport update | ❌ Per-frame |
| `copyToClipboard(s)` | Clipboard set | ❌ Synchronous |

### FontProvider (Model → View — synchronous queries, now cached)

| Method | Cache? | ZMQ? |
|--------|--------|------|
| `getRenderedStringSize(fk, h, s)` | ✅ `ConcurrentHashMap` in RenderManager | ❌ Per-frame, cached |
| `getRenderedStringPosition(fk, h, s, x, y)` | ❌ (mouse click only, rare) | ❌ |
| `isGood()` | ❌ (rare) | ❌ |

### RunListener (Model → Model, bridged to ZMQ)

| Method | ZMQ message |
|--------|-------------|
| `runEnded()` | `runEvent { type: "runEnded" }` |
| `handleRuntimeError(sm, t)` | `runEvent { type: "runError", data: ... }` |

### LogListener (Global Logger → ZMQ)

| Method | ZMQ message |
|--------|-------------|
| `update()` | `logLine { line: "..." }` (one per log entry) |

---

## Communication frequency summary

| Channel | Pattern | Rate | ZMQ |
|---------|---------|------|-----|
| Entity state (full) | PUB periodic | 0.2 Hz (every 5s) | ✅ simState |
| Entity state (delta) | PUB periodic | 0.2 Hz per entity | ✅ entityState |
| Run lifecycle | PUB on event | ~1 per run | ✅ runEvent |
| Log messages | PUB on event | ~10/s during run | ✅ logLine |
| Model commands | ROUTER async | User-initiated | ✅ PAUSE, SET_INPUT |
| Render frame | Direct call | 30 FPS | ❌ — local only |
| Font metrics | Direct call (cached) | 30 FPS | ❌ — local cache |
| Clipboard | Direct call | Interactive | ❌ — local only |
