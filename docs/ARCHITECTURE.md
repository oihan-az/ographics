**ographics** Framework — Architecture & Design

> **Status:** Early design / pre-alpha  
> **Author:** Oihan Abruña  
> **Project:** Modular, API-agnostic C++ graphics and GPU framework

---

## 1. Vision

**ographics** is a reusable C++ framework for applications that need GPU functionality.

The primary goal is to avoid rebuilding a graphics/compute engine from scratch for every project and to make GPU programming reusable across different applications.

The framework should be:

- Platform agnostic
- Graphics API agnostic
- Modular
- Extensible
- Testable
- Reusable across independent projects
- Suitable for both graphical and headless applications

The initial implementation will target **Windows + Vulkan**, with additional backends introduced progressively.

---

# 2. Framework

The framework is composed of five primary libraries.

```text
oplatform
ogfx
oframe
oengine
ogui
 ```

## 2.1 oplatform
 
**oplatform** provides platform and application-system abstractions.

Its purpose is to allow graphical applications to interact with the operating system without exposing platform-specific implementation details to higher-level systems.

Potential responsibilities include:

- Windowing
- Input

The initial windowing implementation will likely use GLFW.

## 2.2 ogfx

**ogfx** is the low-level, API-independent GPU abstraction.

Low-level primitives:
- Device
- Buffer
- Texture
- Sampler
- Shader
- Pipeline
- CommandList
- Queue
- Fence
- Semaphore
- Swapchain

Conceptually:
```text
ogfx public API
       │
       ▼
Backend abstraction
       │
 ┌─────┼─────────────┬───────────┐
 ▼     ▼             ▼           ▼
Vulkan OpenGL      Direct3D12   Metal
```
Only the backend implementation should know the underlying API.

**ogfx** must not require **oplatform**. A compute application should be able to initialize a GPU device without creating a window.

### 2.2.1 API Abstraction Philosophy

The abstraction should preserve important GPU concepts rather than reducing all APIs to their simplest common subset.

The framework should distinguish between:

- Core functionality
- Optional capabilities
- Backend-specific functionality

For example:

Core:
- Buffers
- Textures
- Command submission
- Compute
- Rasterization

Optional:
- Ray tracing
- Mesh shaders
- Bindless resources
- Variable rate shading

## 2.3 oframe

**oframe** is the higher-level GPU workload framework built on top of **ogfx**.

It provides the mechanisms required to describe, organize, schedule, and execute GPU workloads without imposing application-specific rendering or simulation strategies.

It may contain:

- GPU work graphs
- Passes
- GPU resources
- Scheduling
- Synchronization

A pass represents a unit of GPU work and its resource dependencies. The system should make it possible to add new types of passes without modifying the underlying GPU abstraction.

Passes may represent different types of GPU work, including:

- Compute
- Rasterization
- Ray tracing
- Copy/transfer operations

## 2.4 oengine

**oengine** is the higher-level engine layer.

It builds application-oriented concepts on top of **oframe** and **ogfx**.

Potential responsibilities include:

- Object management
- Asset management
- Camera management
- Rendering techniques

**oengine** is where engine-specific rendering decisions belong and should not be required for projects that only need low-level GPU functionality.

New rendering or GPU functionality should preferably be implemented as composable passes rather than requiring modifications to the core framework.

## 2.5 ogui

**ogui** provides graphical user interface integration with **Dear ImGui**.

It is intentionally kept separate so that graphics experiments and lower-level projects can use UI functionality without depending on the entire engine.

# 3. Dependency Graph

```text
                         Application
                              │
                           oengine
               ┌──────────────┼──────────────┐
               │              │              │
           oplatform        oframe          ogui
                              │
                              ▼
                             ogfx
```