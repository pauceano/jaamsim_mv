# JaamSim Architecture Guide

## Overview

JaamSim is a Java-based discrete-event simulation environment with a drag-and-drop GUI, 3D OpenGL rendering, and a fully undoable command-based editing model. This document describes the package structure and architecture, focusing on **Model-View communication**.

---

## 1. Top-Level Package Map (37 packages)

```
com.jaamsim/
├── basicsim/               ← CORE: Entity, JaamSimModel, Simulation, all listener interfaces
├── events/                 ← Discrete-event engine: EventManager, ProcessTarget
├── input/                  ← Input system: Input<T>, InputAgent, Output/OutputHandle, ExpEvaluator
├── Commands/               ← Command pattern for undo/redo
│
├── Graphics/               ← Visual entity layer: DisplayEntity, View, Shape, Text, Arrow, etc.
├── DisplayModels/          ← Visual representations: ShapeModel, ColladaModel, ImageModel, etc.
├── render/                 ← OpenGL rendering engine: Renderer, RenderProxy, DisplayModelBinding
├── controllers/            ← Bridge: RenderManager, CameraControl, RateLimiter
│
├── ui/                     ← Swing UI: GUIFrame (main window), FrameBox, EditBox, ObjectSelector
│
├── ProcessFlow/            ← SimEntity, Server, Queue, EntityGenerator, EntitySink, LinkedComponent
├── BasicObjects/           ← EntitySystem, DowntimeEntity, ToggleButton, ExpressionLogger
├── ResourceObjects/        ← Resource, ResourcePool, Seizable
├── FluidObjects/           ← Fluid tank/pipe/pump simulation
├── GameObjects/            ← GameEntity base class
├── CalculationObjects/     ← Controller, integrator, differentiator
├── Thresholds/             ← Threshold, ExpressionThreshold
├── ProbabilityDistributions/
├── Samples/                ← Sample providers (TimeSeries, etc.)
├── BooleanProviders/
├── StringProviders/
├── EntityProviders/
├── ColourProviders/
│
├── states/                 ← StateEntity, StateRecord, StateEntityListener
├── Statistics/             ← Statistical collectors
├── SubModels/              ← Sub-model (hierarchical) support
│
├── math/                   ← Vec3d, Mat4d, Quaternion, Ray, AABB, Transform
├── units/                  ← Unit, TimeUnit, DistanceUnit, etc. (24 unit types)
├── datatypes/              ← DoubleVector, IntegerVector, BooleanVector
│
├── font/                   ← Tesselated font rendering
├── collada/                ← COLLADA 3D model loading
├── GLTF/                   ← glTF 3D model loading
├── MeshFiles/              ← Mesh formats
│
├── xml/                    ← XML serialization
├── JSON/                   ← JSON parsing
├── rng/                    ← Random number generation (MRG1999a)
└── video/                  ← Video recording (PNG sequences)
```

---

## 2. Entity Class Hierarchy (Model Layer)

All simulation objects derive from `Entity`:

```
Entity (basicsim/Entity.java)
├── Simulation (basicsim/Simulation.java)         ← root entity; configures run duration, calendar, outputs
├── View (Graphics/View.java)                     ← camera/viewpoint (entity in model, rendered as 3D view)
├── DisplayEntity (Graphics/DisplayEntity.java)   ← BASE for all visible simulation objects
│   ├── Shape (Graphics/Shape.java)
│   ├── Text (Graphics/Text.java)
│   ├── Arrow (Graphics/Arrow.java)
│   ├── Graph (Graphics/Graph.java)
│   ├── OverlayText (Graphics/OverlayText.java)
│   ├── EntityLabel (Graphics/EntityLabel.java)
│   ├── Region (Graphics/Region.java)
│   ├── DirectedEntity (Graphics/DirectedEntity.java)  ← moveable/orientable
│   │   └── StateEntity (states/StateEntity.java)      ← state machine (Idle/Working/etc.)
│   │       ├── LinkedComponent (ProcessFlow/LinkedComponent.java)  ← implements SubjectEntity
│   │       │   ├── Server (ProcessFlow/Server.java)
│   │       │   ├── Queue (ProcessFlow/Queue.java)
│   │       │   ├── EntityGenerator (ProcessFlow/EntityGenerator.java)
│   │       │   ├── EntitySink (ProcessFlow/EntitySink.java)
│   │       │   ├── Seize (ProcessFlow/Seize.java)
│   │       │   ├── Release (ProcessFlow/Release.java)
│   │       │   ├── Branch (ProcessFlow/Branch.java)
│   │       │   ├── Assemble (ProcessFlow/Assemble.java)
│   │       │   ├── Combine (ProcessFlow/Combine.java)
│   │       │   └── Device (ProcessFlow/Device.java)
│   │       ├── DowntimeEntity (BasicObjects/DowntimeEntity.java)
│   │       ├── Threshold (Thresholds/Threshold.java)
│   │       └── GameEntity (GameObjects/GameEntity.java)
│   ├── ColladaShape (Graphics/ColladaShape.java)
│   └── SimEntity (ProcessFlow/SimEntity.java)   ← entities that flow through the process
├── SubModel (SubModels/SubModel.java)            ← container with sub-entities
├── Group (basicsim/Group.java)                   ← entity grouping/collections
├── ObjectType (basicsim/ObjectType.java)         ← palette item definition
├── FileEntity (basicsim/FileEntity.java)         ← file I/O
├── ScriptEntity (basicsim/ScriptEntity.java)     ← script-driven
├── DisplayModel (DisplayModels/DisplayModel.java)← visual representation config
│   ├── ShapeModel, ColladaModel, ImageModel, TextModel, GraphModel, PolylineModel
└── Unit (units/Unit.java)                        ← unit definitions
```

---

## 3. View Layer (Two Parallel Systems)

### 3A. Swing UI (`com.jaamsim.ui`)

```
OSFixJFrame (ui/OSFixJFrame.java)
├── GUIFrame (ui/GUIFrame.java)            ← MAIN WINDOW, implements GUIListener
│   - owns menus, toolbar, tool windows
│   - implements ALL GUIListener methods
│   - static getInstance(), getJaamSimModel()
│   - static updateUI() → RateLimiter → callback to FrameBox children
├── FrameBox (ui/FrameBox.java)            ← BASE for all tool windows
│   ├── ObjectSelector                     ← entity tree/navigator
│   ├── EditBox                            ← Input Editor (property grid)
│   ├── PropertyBox                        ← read-only property display
│   ├── OutputBox                          ← output values viewer
│   ├── EventViewer (implements EventTraceListener)
│   ├── LogBox                             ← log output window
│   ├── RunProgressBox                     ← simulation progress
│   └── EntityPallet                       ← drag-and-drop palette
├── GraphicBox (ui/GraphicBox.java)        ← 3D view container (JDialog)
└── Various editors (CellEditor, ColorEditor, ExpressionEditor...)
```

### 3B. 3D OpenGL Rendering (`com.jaamsim.render` + `com.jaamsim.controllers`)

```
controllers/RenderManager.java             ← Singleton driving the render loop
  - renderManagerLoop() — infinite loop: updateGraphics → collectProxies → renderer.queueRedraw()
  - collectProxies() — iterates DisplayEntities, calls binding.collectProxies(simTime, proxies)
  - updateGraphics(simTime) — calls de.updateGraphics(simTime) on every DisplayEntity
  - Handles mouse picking, entity selection, context menus on 3D view
  - Created by GUIFrame during startup

render/Renderer.java                       ← OpenGL/JOGL master renderer
render/RenderProxy.java                    ← Interface: scene objects that can render themselves
render/DisplayModelBinding.java            ← One binding per (Entity, DisplayModel) pair
  - observee — the Entity being rendered
  - abstract collectProxies(simTime, out) — collects RenderProxies for rendering

DisplayModel -> DisplayModelBinding:
  ShapeModel -> (inner class Binding extends DisplayModelBinding)
  ColladaModel -> (inner class Binding)
  TextModel -> (Binding, OverlayBinding, BillboardBinding)
  ImageModel -> (Binding, OverlayBinding)
  GraphModel -> (Binding)
  PolylineModel -> (Binding)
```

---

## 4. Model ↔ View Communication Channels

### Channel 1: `GUIListener` (Primary Bridge)

**File**: `basicsim/GUIListener.java`
**Implementor**: `GUIFrame` (main window)
**Holder**: `JaamSimModel.gui` (set via `setGUIListener()`)

```
Model → View notification flow:
  JaamSimModel.start() → EventManager loop
    → JaamSimModel.tickUpdate(tick) implements EventTimeListener
    → GUIListener.gui_tickUpdate(tick) → GUIFrame.simTicks = tick → RenderManager.updateTime(tick) → GUIFrame.updateUI()

  JaamSimModel.storeAndExecute(Command cmd) (after any input change)
    → GUIListener.updateAll() → GUIFrame.updateUI() → RateLimiter → FrameBox reset/refresh

  InputAgent.processKeyword(Entity, KeywordIndex) (input parsing)
    → GUIListener.handleInputError(t, ent) → error dialog in GUIFrame
```

Methods:
| Method | Trigger |
|--------|---------|
| `updateObjectSelector(Entity)` | Entity selection changed |
| `updateModelBuilder()` | Entity pallet needs refresh |
| `updateInputEditor(Entity)` | Input values changed |
| `updateAll()` | Full UI refresh after model edit |
| `gui_tickUpdate(long tick)` | Simulation time advanced |
| `gui_timeRunning()` | Simulation play/pause/resume toggled |
| `gui_handleError(JaamSimModel, Throwable)` | Runtime error during simulation |
| `handleInputError(Throwable, Entity)` | Input validation error |
| `addView(View)` / `removeView(View)` | View windows opened/closed |
| `createWindow(View)` / `closeWindow(View)` | View window lifecycle |
| `allowResizing(boolean)` | Presentation mode toggle |
| `invokeErrorDialogBox(String, String)` | General error popups |
| `deleteEntity(Entity)` | Entity deletion |
| `renameEntity(Entity, String)` | Entity rename |
| `setSelectedEntity(Entity, boolean)` | Entity selection (from model code) |
| `getFontProvider()` | Font rendering queries from model entities |
| `redraw()` | Request 3D viewport redraw |
| `copyToClipboard(String)` / `getStringFromClipboard()` | Clipboard access |
| `registerTraceListener(EventManager)` | Event trace registration |
| `exitProgram(int)` | Program exit |
| `pauseSimulation()` | Pause simulation from model code |

### Channel 2: `EventTimeListener` (Simulation Time → Model)

**File**: `events/EventTimeListener.java`
**Implementor**: `JaamSimModel`
**Set on**: `EventManager.setTimeListener()`

```
EventManager fires events →
  JaamSimModel.tickUpdate(tick) → GUIListener.gui_tickUpdate(tick) → GUIFrame updates clock display
  JaamSimModel.timeRunning() → GUIListener.gui_timeRunning() → GUIFrame updates play/pause button state
  JaamSimModel.handleError(t) → RunListener.handleRuntimeError(this, t) → SimRun reports error → GUIListener.gui_handleError()
```

### Channel 3: `ObserverEntity` / `SubjectEntity` (Intra-Model Observer)

**Files**: `basicsim/ObserverEntity.java`, `basicsim/SubjectEntity.java`
**Delegate**: `basicsim/SubjectEntityDelegate.java`

```
SubjectEntity.registerObserver(ObserverEntity)     ← watchers register
SubjectEntity.notifyObservers()                     ← state change triggers notification
  → ObserverEntity.observerUpdate(SubjectEntity)    ← each watcher is called
```

Used for **intra-model** communication, not directly for UI. Entities that implement `SubjectEntity`:
- `LinkedComponent` (ProcessFlow base) — also `implements SubjectEntity`
- `EntitySystem` (BasicObjects)
- `Device` (ProcessFlow)
- `ExpressionLogger`, `ExpressionStatistics` (BasicObjects)
- `DowntimeEntity` (BasicObjects)
- `Threshold`, `ExpressionThreshold` (Thresholds)
- `ToggleButton` (BasicObjects)
- `Controller` (CalculationObjects)

### Channel 4: `StateEntityListener` (State Machine → Observers)

**File**: `states/StateEntityListener.java`

```
When a StateEntity changes state:
  StateEntity.setPresentState() → StateEntityListener.updateForStateChange(ent, prevRecord, nextRecord)
```

### Channel 5: `EventTraceListener` (Event Engine → Debug UI)

**File**: `events/EventTraceListener.java`
**Implementors**: `EventViewer` (UI), `EventRecorder`, `EventTracer`

```
EventManager schedules/dispatches events →
  traceEvent(tick, priority, target) — event starts
  traceWait(tick, priority, target) — wait event scheduled
  traceSchedProcess(tick, priority, target) — future event scheduled
  traceProcessStart(target) / traceProcessEnd() — process lifecycle
  traceInterrupt(tick, priority, target) — event rescheduled
  traceKill(tick, priority, target) — event cancelled
  traceWaitUntil() / traceSchedUntil(target) — conditional events
  traceConditionalEval(target) / traceConditionalEvalEnded(wakeup, target)
```

Used by `EventViewer` UI to show real-time event execution timeline.

### Channel 6: `RunListener` (Run Lifecycle → Results)

**File**: `basicsim/RunListener.java`
**Implementor**: `SimRun`

```
JaamSimModel run completes → RunListener.runEnded() → SimRun.runEnded() → Scenario collects results
Runtime error → RunListener.handleRuntimeError(sm, t) → GUIListener.gui_handleError() in single-run mode
```

### Channel 7: `LogListener` (Global Logger → UI)

**File**: `basicsim/LogListener.java`
**Implementor**: `LogBox` (UI)

```
Log.logLine(line) → LogListener.update() → LogBox updates displayed log
```

### Channel 8: `WindowInteractionListener` (3D View Input → RenderManager)

**File**: `render/WindowInteractionListener.java`
**Implementors**: `CameraControl`, `RenderManager`

```
Mouse/keyboard events from JOGL 3D window →
  handleMouseClicked(windowID, x, y, modifiers, count) → entity picking → selection
  handleMouseMoved(...) → hover state
  handleKeyPressed(...) → keyboard shortcuts in 3D view
```

### Channel 9: `FontProvider` (Font Rendering → View)

**File**: `font/FontProvider.java`
**Implemented by**: `RenderManager`, `GUIFrame`
**Accessed via**: `GUIListener.getFontProvider()`

```
Model entities needing font metrics (TextBasics, OverlayText)
  → GUIListener.getFontProvider() (via JaamSimModel.gui)
    → FontProvider.getRenderedStringSize() / getRenderedStringPosition()
      → RenderManager delegates to Renderer for actual tessellated font queries
```

This channel enables model-layer entities (Text, OverlayText) to query font metrics (string dimensions, character positions) without importing render/classes. The `isGood()` check on `FontProvider` gates font operations to a healthy renderer.

### Channel 10: `InputCallback` (Input Value Change → Entity)

**File**: `input/InputCallback.java`

```
Input.setValue() → InputCallback.callback(Entity, Input<?>) → entity updates internal state
```

Used for internal Entity bookkeeping (name changes, parent changes, trace flags, user output changes).

### Channel 11: `Command` Pattern (User Edits → Undo/Redo)

**File**: `Commands/Command.java`
**Implementations**: `KeywordCommand`, `DefineCommand`, `DeleteCommand`, `DefineViewCommand`, `CoordinateCommand`, `ListCommand`

```
User edits input in UI →
  EditBox / ObjectSelector creates KeywordCommand →
  JaamSimModel.storeAndExecute(cmd) →
    cmd.execute() → InputAgent.processKeyword() → Input.setValue() →
    model changes, command added to undo list →
    gui.updateAll()
```

---

## 5. Complete Communication Flow Diagrams

### 5.1 Simulation Run Lifecycle

```
┌──────────┐    ┌──────────────┐    ┌─────────────┐    ┌────────────────┐    ┌─────────────┐
│ GUIFrame │    │  SimRun      │    │JaamSimModel │    │  EventManager   │    │  Entities   │
│  (View)  │    │  (basicsim)   │    │  (basicsim)  │    │  (events)       │    │  (model)    │
└────┬─────┘    └──────┬───────┘    └──────┬──────┘    └───────┬────────┘    └──────┬──────┘
     │                 │                   │                   │                    │
     │ click "Run"     │                   │                   │                    │
     │────────────────>│                   │                   │                    │
     │                 │ start(sm)         │                   │                    │
     │                 │──────────────────>│                   │                    │
     │                 │                   │ start(runListener)│                    │
     │                 │                   │──────────────────>│                    │
     │                 │                   │                   │ schedule events    │
     │                 │                   │                   │───────────────────>│
     │                 │                   │                   │                    │
     │                 │       ┌─ Event Loop ──────────────────────────────────────────┐
     │                 │       │  │                   │  tickUpdate(tick)  │          │
     │                 │       │  │<───────────────────│                    │          │
     │                 │       │  │                   │                    │          │
     │                 │       │  │ gui_tickUpdate()  │                    │          │
     │  updateUI()     │       │  │<───────────────────────────────────────┤          │
     │<────────────────────────────────────────────────────────────────────┤          │
     │  (clock update) │       │  │                   │                    │          │
     │                 │       │  │                   │  dispatch event    │          │
     │                 │       │  │                   │───────────────────>│ execute() │
     │                 │       │  │                   │                    │          │
     │                 │       │  │                   │  state change      │          │
     │                 │       │  │                   │  notifyObservers() │          │
     │                 │       │  │                   │<───────────────────│          │
     │                 │       └─ End Loop ────────────────────────────────────────────┘
     │                 │                   │                   │                    │
     │                 │  runEnded()       │                   │                    │
     │                 │<──────────────────│                   │                    │
     │  updateUI()     │                   │                   │                    │
     │<────────────────│                   │                   │                    │
```

### 5.2 User Edits an Input (Model Editing)

```
┌──────────┐    ┌──────────┐    ┌──────────────┐    ┌────────────┐    ┌───────────────┐
│ EditBox  │    │GUIFrame  │    │JaamSimModel  │    │ InputAgent │    │   Entity      │
│  (UI)    │    │  (View)  │    │  (basicsim)   │    │  (input)    │    │   (model)     │
└────┬─────┘    └────┬─────┘    └──────┬───────┘    └─────┬──────┘    └───────┬───────┘
     │               │                 │                   │                   │
     │ user edits    │                 │                   │                   │
     │ value         │                 │                   │                   │
     │──────────────>│                 │                   │                   │
     │               │ new KeywordCommand(ent, kw)         │                   │
     │               │────────────────>│                   │                   │
     │               │                 │ storeAndExecute(cmd)                  │
     │               │                 │──────────────────>│                   │
     │               │                 │ processKeyword()  │                   │
     │               │                 │──────────────────>│                   │
     │               │                 │                   │ setValue()        │
     │               │                 │                   │──────────────────>│
     │               │                 │                   │                   │
     │               │                 │ cmd added to      │  Input.callback() │
     │               │                 │ undoList          │<──────────────────│
     │               │                 │                   │                   │
     │               │                 │ gui.updateAll()   │                   │
     │               │                 │──────────────────>│                   │
     │               │                 │                   │                   │
     │ updateUI()    │                 │                   │                   │
     │<──────────────│                 │                   │                   │
     │ (refresh)     │                 │                   │                   │
```

### 5.3 3D Rendering Pipeline

```
GUIFrame.createGUI()
  → RenderManager.initialize(safeGraphics)
    → Renderer created on its own thread
    → renderManagerLoop():

   LOOP:
    1. JaamSimModel simModel = GUIFrame.getJaamSimModel()
    2. double renderTime = simModel.getEventManager().ticksToSeconds(simTick)
    3. updateGraphics(simModel, renderTime)        ← calls de.updateGraphics() on each DisplayEntity
    4. collectProxies(simModel, renderTime, max, scene)
         for each DisplayEntity:
           for each DisplayModelBinding binding:
             binding.collectProxies(renderTime, scene)  ← reads Entity state → generates RenderProxy objects
    5. renderer.setScene(scene)                      ← passes scene to renderer
    6. renderer.queueRedraw()                        ← triggers actual OpenGL draw
    7. redraw.waitForRedraw()                        ← waits for next frame request

   Triggered by:
    - GUIFrame.updateUI() → RateLimiter → RedrawCallback → wakes up render loop
    - GUIFrame.gui_tickUpdate() → RenderManager.updateTime(tick) → sets simTick
```

### 5.4 Mouse Click in 3D Viewport

```
JOGL window → WindowInteractionListener.handleMouseClicked()
  → RenderManager.handleMouseClicked(windowID, x, y, modifiers, count)
    → pickEntityForMouse(windowID, false)
      → renderer.getPickData(...) [OpenGL picking]
      → sort by depth
    → FrameBox.setSelectedEntity(ent, true)
      → GUIFrame.updateUI()
    → ent.handleMouseClicked(count, globalCoord, shift, ctrl, alt)
```

---

## 6. Key Architectural Principles

1. **JaamSimModel is the central hub** — it owns the Entity registry, the EventManager, the GUIListener, and the Simulation root. Every major communication path passes through it.

2. **`Simulation` is just an Entity** — it's the root entity with no visual representation. It configures run parameters (duration, calendar, output reports) and acts as the "bridge to the UI" (per javadoc). It lives in the model layer.

3. **All model changes go through `Command` objects** — this gives full undo/redo. `KeywordCommand` stores old and new `KeywordIndex` values for every input change. Commands in model packages (`Commands/`) must route view operations through `GUIListener`, never import view classes directly.

4. **Strict model→view boundary** — model packages (`basicsim`, `input`, `events`, `Commands`, `math`, `states`, `units`, `datatypes`, `rng`) must have **zero** imports from `ui/`, `render/`, or `controllers/`. Model code communicates with the view exclusively through listener/bridge interfaces defined in the model layer:
   - `GUIListener` (basicsim) — primary model→view channel
   - `FontProvider` (font) — font rendering queries from model entities
   - `EventTimeListener`, `RunListener`, `LogListener` — simulation lifecycle
   - `InputCallback` — input value change notifications
   
   **Acceptable exceptions** (documented and reviewed):
   - `basicsim/ObjectType.java` → `render/DisplayModel` (palette needs entity types)
   - `input/ActionListInput.java` → `render/Action` (input parser for config data)

5. **Two parallel view systems** — Swing (tool panels, property editors, entity pallet) and OpenGL/JOGL (3D viewports). Both update via `GUIListener.updateAll()` and the `RateLimiter`.

6. **Rate-limited UI updates** — `GUIFrame.updateUI()` goes through a `RateLimiter` thread that throttles to ~30 FPS (configurable), preventing UI flooding during high-speed simulation.

7. **`DisplayModelBinding` is the View/Model glue for 3D** — one `DisplayModelBinding` per (Entity, DisplayModel) pair. It reads Entity state (position, size, colour, etc.) and produces `RenderProxy` objects for the renderer. Each `DisplayModel` subclass defines its own inner `Binding` class.

8. **Observer pattern is intra-model only** — `SubjectEntity`/`ObserverEntity` connects model objects to each other (e.g., `LinkedComponent` watches state changes in upstream components). It does NOT connect model to view. Model→view goes through `GUIListener`.

9. **Static/instance naming discipline** — when adding an instance method to a class that already has a static method with the same name (causing collision when the class also implements an interface), rename the static method rather than breaking static callers. Pattern: `isGood()` → `isReady()` for static, keep `isGood()` as instance.

10. **Pure-math code lives in `math/`, not `render/`** — methods that perform geometric/matrix computations without render dependencies belong in `math/MathUtils.java`, not `render/RenderUtils.java`. Examples: `mergeTransAndScale`, `getInverseWithScale`, `rayClosePoint`.

---

## 7. Critical File Index

| Layer | File | Role |
|-------|------|------|
| Core | `basicsim/Entity.java` | Base for ALL simulation objects |
| Core | `basicsim/JaamSimModel.java` | Central hub: owns entities, event manager, GUI listener |
| Core | `basicsim/Simulation.java` | Root entity, run configuration, bridge to UI |
| Core | `events/EventManager.java` | Discrete-event scheduling engine |
| Core | `input/Input.java` | Typed property system for all entities |
| Core | `input/InputAgent.java` | Input parsing and application |
| Core | `input/OutputHandle.java` | Runtime output value access via reflection |
| Core | `input/ExpEvaluator.java` | Expression engine |
| Bridge | `basicsim/GUIListener.java` | **Primary** model→view interface |
| Bridge | `events/EventTimeListener.java` | Simulation time→model bridge |
| Bridge | `basicsim/SubjectEntity.java` | Observer pattern: subject role |
| Bridge | `basicsim/ObserverEntity.java` | Observer pattern: observer role |
| Bridge | `basicsim/RunListener.java` | Run completion callback |
| Bridge | `basicsim/LogListener.java` | Global log callback |
| Bridge | `font/FontProvider.java` | Font rendering queries from model entities |
| Bridge | `render/DisplayModelBinding.java` | Entity→RenderProxy bridge per DisplayModel |
| Bridge | `input/InputCallback.java` | Input change callback |
| Core | `basicsim/RenderException.java` | Generic runtime exception for parsing/loading errors |
| View | `ui/GUIFrame.java` | Main window, GUIListener implementation |
| View | `ui/FrameBox.java` | Base for all tool windows |
| View | `controllers/RenderManager.java` | 3D rendering loop, singleton |
| View | `render/RenderProxy.java` | Interface for all renderable objects |
| View | `render/Renderer.java` | OpenGL/JOGL master renderer |
| View | `controllers/RateLimiter.java` | UI update throttling |
| Model | `ProcessFlow/LinkedComponent.java` | Base for process objects (SubjectEntity) |
| Model | `states/StateEntity.java` | State machine base |
