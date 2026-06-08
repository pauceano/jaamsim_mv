# JaamSim Model-View Boundary

## Architecture Principle

The model layer (`basicsim`, `input`, `Commands`, `events`, `math`) has **zero imports** from view layer packages (`ui`, `render`, `controllers`). All communication between model and view goes through **listener/bridge interfaces** defined in the model layer.

---

## 1. Model Layer (Zero View Dependencies)

### Core Simulation
| Package | Classes | Responsibility |
|---------|---------|---------------|
| `basicsim` | `Entity`, `JaamSimModel`, `Simulation`, `ObjectType`, `Group`, `FileEntity`, `ScriptEntity`, `SoftwareInfo`, `DragAndDropable` | Entity base class, world model, simulation config, app info, DnD interface |
| `events` | `EventManager`, `ProcessTarget` | Discrete-event scheduling engine |
| `input` | `Input<T>`, `InputAgent`, `OutputHandle`, `ExpEvaluator` | Typed property system, input parsing, expression evaluation |
| `Commands` | `Command`, `KeywordCommand`, `DefineCommand`, `DeleteCommand` | Undo/redo command pattern. Zero imports from `ui/`, `controllers/`, `render/`. |

### Utilities (no view deps)
| Package | Classes |
|---------|---------|
| `math` | `Vec3d`, `Mat4d`, `Quaternion`, `Transform`, `Ray`, `AABB`, `Color4d`, `ConvexHull`, `MathUtils`, **`TessFontKey`** (moved from render), **`VisibilityInfo`** (moved from render) |
| `units` | `Unit`, `TimeUnit`, `DistanceUnit` (24 types) |
| `datatypes` | `DoubleVector`, `IntegerVector`, `BooleanVector` |
| `rng` | MRG1999a random number generator |

### Foundation Types (used across all layers)
| Class | Location | Notes |
|-------|----------|-------|
| `RenderException` | `basicsim/RenderException.java` | Generic runtime exception; moved from `render/`. Used by `math/`, `xml/`, `collada/`, `GLTF/`, `MeshFiles/`, `render/` |
| `DragAndDropable` | `basicsim/DragAndDropable.java` | DnD interface; moved from `ui/`. Implemented by `ObjectType` |
| `SoftwareInfo` | `basicsim/SoftwareInfo.java` | Application name/version constants; extracted from `ui/AboutBox` |

### Intra-Model Communication (no view involvement)
| Interface/Class | File | Purpose |
|-----------------|------|---------|
| `SubjectEntity` / `ObserverEntity` | `basicsim/SubjectEntity.java`, `basicsim/ObserverEntity.java` | Observer pattern for model state changes |
| `SubjectEntityDelegate` | `basicsim/SubjectEntityDelegate.java` | Delegate implementation |
| `StateEntityListener` | `states/StateEntityListener.java` | State machine change notifications |

---

## 2. Bridge Layer (Model Interfaces → View Implementations)

These interfaces live in the model layer. The view layer implements them. The model never imports concrete view classes.

### Primary Bridges

| Interface | Location | Implementor | Communication |
|-----------|----------|-------------|---------------|
| `GUIListener` | `basicsim/GUIListener.java` | `GUIFrame` (ui) | **The central model→view bus**. All UI updates, error dialogs, entity selection, trace listeners, font provider access, redraw requests, clipboard access |
| `EventTimeListener` | `events/EventTimeListener.java` | `JaamSimModel` | Simulation time ticks forwarded to GUI |

### Bridge Methods Detail

#### `GUIListener` — All methods:

```java
public interface GUIListener {
    // Error handling
    void handleInputError(Throwable t, Entity ent);
    void invokeErrorDialogBox(String title, String msg);
    void invokeErrorDialogBox(String title, String pre, String message, String post);

    // Simulation control
    void pauseSimulation();
    void exitProgram(int errorCode);
    void gui_tickUpdate(long tick);
    void gui_timeRunning();
    void gui_handleError(JaamSimModel sm, Throwable t);

    // UI refresh
    void updateObjectSelector(Entity ent);
    void updateModelBuilder();
    void updateInputEditor(Entity ent);
    void updateAll();
    void deleteEntity(Entity ent);
    void renameEntity(Entity ent, String newName);
    void setSelectedEntity(Entity ent, boolean bool);
    void redraw();

    // View window lifecycle
    void addView(View v);
    void removeView(View v);
    void createWindow(View v);
    void closeWindow(View v);
    void allowResizing(boolean bool);

    // Clipboard (decouples Graphics.EditableTextDelegate from ui.GUIFrame)
    void copyToClipboard(String str);
    String getStringFromClipboard();

    // Font services (delegates to FontProvider)
    FontProvider getFontProvider();

    // Event tracing
    void registerTraceListener(EventManager eventManager);
}
```

### Specialized Bridges

| Interface | Location | Implementor | Communication |
|-----------|----------|-------------|---------------|
| `EventTraceListener` | `events/EventTraceListener.java` | `EventViewer` (ui) | Event scheduling/dispatch tracing for debug UI |
| `RunListener` | `basicsim/RunListener.java` | `SimRun` (basicsim) | Run lifecycle events (started, ended, error) |
| `LogListener` | `basicsim/LogListener.java` | `LogBox` (ui) | Global log line output |
| `InputCallback` | `input/InputCallback.java` | Entity init blocks | Input value change triggers entity internal updates |

### Service Interfaces

| Interface | Location | Implementor | Communication |
|-----------|----------|-------------|---------------|
| `FontProvider` | `font/FontProvider.java` | `RenderManager` (controllers) | Font measurement services for text entities. Extracted from `render/` during decoupling. |

---

## 3. View Layer

### Swing UI Package (`ui`)
| Class | Imports From Model | Imports From View |
|-------|-------------------|-------------------|
| `GUIFrame` | `basicsim.*`, `math.*`, `render.DisplayModel`, `font.FontProvider`, `controllers.RenderManager` | `FrameBox`, `EventViewer` |
| `FrameBox` | `basicsim.*` | — |
| `EditBox` | `basicsim.*`, `input.*` | `FrameBox` |
| `ObjectSelector` | `basicsim.*`, `render.IconModel` | `FrameBox` |
| `EventViewer` | `events.*` | — |
| `LogBox` | `basicsim.*` | — |
| `AboutBox` | `basicsim.SoftwareInfo` | — |

### 3D Rendering Package (`render`)
Contains the former `DisplayModels/` subpackage (now merged):

| Class | Original Package | Notes |
|-------|-----------------|-------|
| `DisplayModel` | `DisplayModels/` | Base class for all visual representations |
| `ShapeModel` | `DisplayModels/` | 2D/3D shape rendering |
| `ColladaModel` | `DisplayModels/` | COLLADA 3D model support |
| `ImageModel` | `DisplayModels/` | Image/texture visual |
| `TextModel` | `DisplayModels/` | Text rendering config |
| `GraphModel` | `DisplayModels/` | Graph/chart visual |
| `PolylineModel` | `DisplayModels/` | Polyline rendering |
| `IconModel` | `DisplayModels/` | Icon display |

| Class | Imports From Model | Imports From View |
|-------|-------------------|-------------------|
| `Renderer` | `basicsim.*`, `math.*` | JOGL |
| `RenderProxy` (and subclasses) | `math.*` | `render.*` (same package) |
| `DisplayModelBinding` | `basicsim.*`, `math.*`, `Graphics.*` | `controllers.RenderManager` |
| `DisplayModel` | `basicsim.*`, `math.*`, `Graphics.*`, `input.*` | — |
| `RenderUtils` | `math.*`, AWT | Removed: `mergeTransAndScale`, `getInverseWithScale`, `rayClosePoint` → moved to `math.MathUtils` |

### Font Package (`font`) — Service Layer
| Class | Imports From Model | Role |
|-------|-------------------|------|
| `FontProvider` | `math.TessFontKey`, `math.Vec3d` | Interface for font measurement (moved from `render/`) |
| `TessFont` | `math.TessFontKey` | Tesselated font rendering |
| `TessString` | `math.*` | Individual string rendering |
| `BillboardString` | `math.*` | Billboard text |
| `OverlayString` | `math.*` | Overlay text |

### Visual Entity Layer (`Graphics`) — Hybrid
These are **entities** (extend `Entity`) but inherently visual. They import from the bridge.

| Class | View Imports | How It Communicates With View |
|-------|-------------|-------------------------------|
| `DisplayEntity` | `render.DisplayModel` (and subclasses), `math.VisibilityInfo`, `font.FontProvider`, `math.MathUtils` | `getJaamSimModel().getGUIListener()` for UI actions; `getDisplayBindings()` → `DisplayModelBinding.collectProxies()` for 3D rendering |
| `TextBasics` | `math.TessFontKey`, `font.FontProvider` | `getFontProvider()` → `FontProvider.getRenderedStringSize()` for font metrics; `gui.redraw()` for repaint |
| `OverlayText` | `math.TessFontKey`, `font.FontProvider` | Same as TextBasics |
| `EntityLabel` | `math.VisibilityInfo` | Uses VisibilityInfo for visibility control |
| `EditableTextDelegate` | — | Clipboard via `gui.copyToClipboard()` / `gui.getStringFromClipboard()` |

### Controller Package (`controllers`)
| Class | Imports From Model | Role |
|-------|-------------------|------|
| `RenderManager` | `basicsim.*`, `math.*`, `Graphics.*`, `input.*`, `render.*`, `font.FontProvider`, `basicsim.DragAndDropable` | Rendering loop singleton; implements `FontProvider`. Static `isGood()` renamed to `isReady()` to avoid collision with `FontProvider.isGood()` instance method. |
| `CameraControl` | `math.*`, `Graphics.*` | Mouse/keyboard → 3D camera |
| `RateLimiter` | — | UI update throttling (~30 FPS) |

---

## 4. Communication Flow Diagrams

### Model → View (via GUIListener)

```
Entity / Input change
  → JaamSimModel.storeAndExecute(Command)
    → gui.updateAll()     [GUIListener]
      → GUIFrame.updateUI()
        → RateLimiter → FrameBox.reset/refresh
```

### Simulation Tick → View

```
EventManager.dispatchEvents()
  → EventTimeListener.tickUpdate(tick)   [via JaamSimModel]
    → GUIListener.gui_tickUpdate(tick)   [via JaamSimModel]
      → GUIFrame.simTicks = tick
        → RenderManager.updateTime(tick)
          → GUIFrame.updateUI()
```

### 3D Rendering Pipeline

```
RenderManager.renderManagerLoop()
  → collectProxies()
    → for each DisplayEntity:
      → DisplayModelBinding.collectProxies(simTime, proxies)
        → creates PolygonProxy, LineProxy, etc.

  → renderer.setScene(scene)
    → renderer.queueRedraw()   [JOGL draw]
```

### Entity Font Measurement

```
TextBasics.getStringPosition(globalCoord)
  → getFontProvider()          [private helper]
    → getJaamSimModel().getGUIListener()
      → gui.getFontProvider()  [GUIListener]
        → GUIFrame.getFontProvider()
          → RenderManager.inst()  [implements FontProvider]
            → renderer.getTessFont(fontKey).getStringSize(...)
```

### Entity Edit — Clipboard Access

```
EditableTextDelegate.copy()
  → gui.copyToClipboard(copiedText)    [GUIListener]
    → GUIFrame.copyToClipboard(str)    [System clipboard]
```

### User Input in 3D Viewport

```
Mouse click → WindowInteractionListener
  → RenderManager.handleMouseClicked()
    → pickEntityForMouse()     [OpenGL picking]
    → FrameBox.setSelectedEntity()
    → entity.handleMouseClicked()
```

### Command → View (Decoupled)

Previously `DefineViewCommand` called `RenderManager.inst().createWindow()` and `FrameBox.setSelectedEntity()` directly. Now it routes through `GUIListener`:

```
DefineViewCommand.execute()
  → simModel.getGUIListener()
    → gui.createWindow(view)             [GUIListener]
    → gui.setSelectedEntity(view, false) [GUIListener]
      → GUIFrame.createWindow(view)
        → RenderManager.inst().createWindow(view)
```

---

## 5. Boundary Enforcement History

### Phase 1 — Routing through GUIListener
- Added `createWindow()`, `setSelectedEntity()`, `registerTraceListener()`, `exitProgram()`, `pauseSimulation()` to `GUIListener`
- `RunManager` and `EventTracer` now call `gui.xxx()` instead of `GUIFrame.xxx()` directly
- `DefineViewCommand` decoupled from `RenderManager`/`FrameBox`

### Phase 2 — Package Decoupling
- `DisplayModels/` (8 files) merged into `render/`
- `render.TessFontKey` → `math.TessFontKey`
- `render.VisibilityInfo` → `math.VisibilityInfo`
- `render.FontProvider` → new `font.FontProvider` interface
- `render.RenderException` → `basicsim.RenderException`
- `ui.DragAndDropable` → `basicsim.DragAndDropable`
- `ui.AboutBox.softwareName/version` → `basicsim.SoftwareInfo.NAME/VERSION`
- `ui.NaturalOrderComparator` → deleted (replaced by JDK `compareTo`)
- `render.RenderUtils` pure-math methods → `math.MathUtils`
- `font/` package established as service layer

### Phase 3 — Boundary Compilation Checks
- Added `copyToClipboard/getStringFromClipboard` to `GUIListener` to decouple `EditableTextDelegate` from `GUIFrame`
- Renamed `RenderManager.isGood()` to `isReady()` to resolve `static`/`instance` collision with `FontProvider`
- Renamed `GUIFrame` static clipboard methods to resolve collision with `GUIListener` instance methods
- Verified: model packages (`basicsim`, `input`, `events`, `Commands`, `states`, `math`) have zero imports from view packages (`ui`, `controllers`)
- Verified: entire `src/main/java` (545 files) compiles with zero errors

---

## 6. Dependency Graph Summary

```
┌──────────────────────────────────────────────────────────────┐
│                       MODEL LAYER                            │
│  ┌───────────┐  ┌────────┐  ┌──────────┐  ┌──────────────┐  │
│  │ basicsim  │  │ events │  │  input   │  │  Commands    │  │
│  │ (Entity,  │  │(EventM-│  │(Input<T>,│  │(Command,     │  │
│  │  JSM, Sim,│  │  anager)│  │ InputAgt)│  │ KeywordCmd)  │  │
│  │  SoftInfo,│  │        │  │          │  │              │  │
│  │  DnDable, │  │        │  │          │  │              │  │
│  │  RenderEx)│  │        │  │          │  │              │  │
│  └────┬──────┘  └────────┘  └──────────┘  └──────────────┘  │
│       │                                                      │
│  ┌────┴────────────────────────────────────────────────────┐ │
│  │                BRIDGE INTERFACES                         │ │
│  │  GUIListener  EventTimeListener  RunListener            │ │
│  │  FontProvider EventTraceListener LogListener            │ │
│  │  InputCallback  SubjectEntity/ObserverEntity            │ │
│  └──────────────────────────────────────────────────────────┘ │
└─────────────────────┬─────────────────────────────────────────┘
                      │ implements
                      ▼
┌──────────────────────────────────────────────────────────────┐
│                      VIEW LAYER                              │
│  ┌──────────┐  ┌────────────────────┐  ┌──────────────────┐  │
│  │  ui/     │  │   render/          │  │  controllers/    │  │
│  │ GUIFrame │  │  Renderer          │  │  RenderManager   │  │
│  │ FrameBox │  │  RenderProxy*      │  │  CameraControl   │  │
│  │ EditBox  │  │  DisplayModel*     │  │  RateLimiter     │  │
│  │ EventView│  │  DisplayModelBind  │  └──────────────────┘  │
│  │ LogBox   │  │  ShapeModel, etc.  │                        │
│  └──────────┘  └────────────────────┘                        │
│                                                               │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │  font/ (service layer: FontProvider, TessFont, etc.)     │ │
│  └──────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│  HYBRID LAYER (Entities with visual nature)                  │
│  ┌──────────────────────────────────────────────────────────┐│
│  │  Graphics/ (DisplayEntity, TextBasics, OverlayText,      ││
│  │             EntityLabel, Arrow, Shape, Graph, etc.)      ││
│  │  ProcessFlow/ (LinkedComponent, Server, Queue, etc.)     ││
│  │  BasicObjects/ (ToggleButton, BooleanIndicator, etc.)    ││
│  │  Thresholds/ (Threshold, ExpressionThreshold)             ││
│  │  FluidObjects/ (FluidTank, FluidPipe)                    ││
│  └──────────────────────────────────────────────────────────┘│
│  These import bridge interfaces (GUIListener, FontProvider)  │
│  and may import render.DisplayModel for input configuration  │
└──────────────────────────────────────────────────────────────┘
```

### Acceptable Cross-Layer Dependencies

These are inherent couplings that cross the boundary by necessity:

| Source | Target | Reason |
|--------|--------|--------|
| `basicsim/ObjectType.java` | `render/DisplayModel` | Palette needs to list all entity types including visual ones |
| `input/ActionListInput.java` | `render/Action.Binding` | Input parser for render-related animation action config |

---

## 7. Key Rules

1. **Model never imports view**: `basicsim/`, `input/`, `events/`, `Commands/`, `states/`, `math/` have **zero imports** from `ui/`, `render/`, or `controllers/`.
2. **Bridge interfaces are in model**: All listener interfaces live in `basicsim/` or `events/`. View implements them.
3. **`Graphics/` is hybrid**: These are model entities that inherently need visual services. They access view through `GUIListener` and `FontProvider` only.
4. **`render/DisplayModelBinding` is the 3D bridge**: One per (Entity, DisplayModel) pair. Reads entity state, produces `RenderProxy` objects.
5. **No static view calls from model**: All model→view goes through `gui = sm.getGUIListener()`. Static references to `GUIFrame`, `RenderManager`, `FrameBox` are prohibited in model packages.
6. **`static`/`instance` naming discipline**: When a bridge interface defines an instance method (e.g., `isGood()`, `copyToClipboard()`), any pre-existing static method with the same signature in the implementing class must be renamed to avoid Java name collision.
