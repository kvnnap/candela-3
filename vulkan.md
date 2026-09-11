# Candela-3 Vulkan Implementation Analysis

## Overview

**Candela-3** is a physically-based renderer using Vulkan as its graphics backend. The implementation demonstrates modern Vulkan practices with C++20 modules, vulkan.hpp RAII wrappers, and Vulkan 1.3+ features like dynamic rendering and synchronization2.

---

## 1. Project Structure

```
candela-3/
├── CMakeLists.txt              # Root CMake configuration
├── vcpkg.json                  # Dependencies
├── apps/candela-3/             # Main application
│   ├── main.cpp               # Entry point
│   ├── renderer/              # Vulkan implementation
│   │   ├── renderer.cppm      # Module interface (C++20)
│   │   ├── vulkan_instance.cpp
│   │   ├── vulkan_device.cpp
│   │   ├── vulkan_swapchain.cpp
│   │   ├── vulkan_pipeline.cpp
│   │   ├── vulkan_command.cpp
│   │   ├── vulkan_utils.cpp
│   │   └── vulkan_renderer.cpp
│   └── shaders/
│       └── shader.slang       # Slang shader source
└── libs/
    ├── core/window/           # GLFW window management
    └── core/util/             # Utilities
```

---

## 2. Vulkan Setup/Initialization

### Instance Creation
**File:** `renderer/vulkan_instance.cpp:27-115`

The Vulkan instance is created with:

**Extensions:**
- `VK_EXTDebugUtilsExtensionName` - Debug utilities for validation layer callbacks
- Window system extensions via `glfwGetRequiredInstanceExtensions()`

**Layers:**
- `VK_LAYER_KHRONOS_validation` - Enabled in debug builds only

**Application Info:**
```cpp
vk::ApplicationInfo appInfo{
    .pApplicationName = "Candela",
    .applicationVersion = vk::makeVersion(1, 0, 0),
    .pEngineName = "No Engine",
    .engineVersion = vk::makeVersion(1, 0, 0),
    .apiVersion = vk::ApiVersion14  // Vulkan 1.4
};
```

**Debug Messenger:** Uses `vk::DebugUtilsMessengerEXT` to capture Verbose, Warning, and Error messages with General, Performance, and Validation message types.

### Vulkan-hpp Configuration
**File:** `libs/external/CMakeLists.txt:78`

Dynamic dispatch is enabled via `VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1`, allowing runtime selection of Vulkan functions.

---

## 3. Physical & Logical Devices

### Physical Device Selection
**File:** `renderer/vulkan_device.cpp:31-80`

The device selection process:

1. **Required Extension:** `VK_KHR_swapchain` - Must support swapchain functionality
2. **Queue Family:** Must support both graphics and surface presentation
3. **Feature Chain (lines 62-69):**
   ```cpp
   auto features = phyDevice.getFeatures2<
       vk::PhysicalDeviceFeatures2, 
       vk::PhysicalDeviceVulkan11Features, 
       vk::PhysicalDeviceVulkan13Features, 
       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
   >();
   ```

**Required Features:**
| Feature | Version | Purpose |
|---------|---------|---------|
| `dynamicRendering` | Vulkan 1.3 | Pipeline-free rendering |
| `synchronization2` | Vulkan 1.3 | Improved synchronization |
| `shaderDrawParameters` | Vulkan 1.1 | Shader access to draw parameters |
| `extendedDynamicState` | EXT | Dynamic viewport/scissor state |

### Logical Device Creation
**File:** `renderer/vulkan_device.cpp:98-112`

- **Queue Priority:** 0.5f (single queue)
- **Extensions Enabled:** `VK_KHR_swapchain`
- **Features:** Chain of Vulkan 1.1, 1.3, and EXT features

---

## 4. Memory Management

**File:** `renderer/vulkan_utils.cpp:31-73`

### Buffer Creation Pattern

```cpp
void createBuffer(
    const VulkanDevice& device,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties,
    vk::raii::Buffer& buffer,
    vk::raii::DeviceMemory& bufferMemory
)
```

**Memory Type Finding (lines 34-42):**
The algorithm iterates through memory types and finds one that matches both the `typeFilter` (buffer requirements) and `properties` (CPU/GPU access patterns).

### Staging Buffer Pattern (lines 52-73)

For uploading data to GPU-local memory:

1. Create **staging buffer** with `HOST_VISIBLE | HOST_COHERENT` memory
2. Map memory and copy data with `memcpy`
3. Create **device buffer** with `DEVICE_LOCAL` memory
4. Submit transfer command and wait for completion

```cpp
// Staging (CPU accessible)
createBuffer(device, size, 
    vk::BufferUsageFlagBits::eTransferSrc, 
    vk::MemoryPropertyFlagBits::eHostVisible | eHostCoherent, 
    stagingBuffer, stagingBufferMemory);

// Device (GPU only)
createBuffer(device, size, 
    dstUsage | eTransferDst, 
    vk::MemoryPropertyFlagBits::eDeviceLocal, 
    dBuffer, dBufferMemory);

// Transfer
cmd.copyBuffer(stagingBuffer, dBuffer, vk::BufferCopy(0, 0, size));
```

---

## 5. Buffers & Images

### Vertex Buffer
**File:** `renderer/vulkan_renderer.cpp:156-160`

**Vertex Structure:**
```cpp
struct Vertex {
    glm::vec2 pos;    // location 0 - 8 bytes
    glm::vec3 color;  // location 1 - 12 bytes
};
// Total stride: 20 bytes
```

**Buffer Usage:** `eVertexBuffer | eTransferDst`

### Index Buffer
**File:** `renderer/vulkan_renderer.cpp:162-166`

```cpp
static const std::vector<std::uint16_t> indices = { 0, 1, 2 };
```

**Buffer Usage:** `eIndexBuffer | eTransferDst`

### Swapchain Image Views
**File:** `renderer/vulkan_swapchain.cpp:104-127`

- **View Type:** 2D
- **Format:** `eB8G8R8A8Srgb`
- **Aspect:** Color only
- **Levels/Layers:** 1 each

---

## 6. Shaders

### Shader Source
**File:** `shaders/shader.slang`

Uses **Slang** shading language, compiled to SPIR-V:

```slang
struct VSInput {
  float2 inPosition;
  float3 inColor;
};

struct VSOutput {
  float4 pos : SV_Position;
  float3 color;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
  VSOutput output;
  output.pos = float4(input.inPosition, 0.0, 1.0);
  output.color = input.inColor;
  return output;
}

[shader("fragment")]
float4 fragMain(VSOutput inVert) : SV_Target {
  return float4(inVert.color, 1.0);
}
```

### Shader Compilation
**File:** `apps/candela-3/CMakeLists.txt:100`

```bash
slangc shader.slang \
    -target spirv -profile spirv_1_4 \
    -emit-spirv-directly \
    -fvk-use-entrypoint-name \
    -entry "vertMain" -entry "fragMain" \
    -o shaders/shaders.spv
```

### Shader Module Creation
**File:** `renderer/vulkan_pipeline.cpp:22-29`

```cpp
vk::raii::ShaderModule VulkanPipeline::createShaderModule(
    const std::vector<std::byte>& code
) {
    vk::ShaderModuleCreateInfo createInfo{ 
        .codeSize = code.size() * sizeof(std::byte), 
        .pCode = reinterpret_cast<const std::uint32_t*>(code.data()) 
    };
    return { device.getDevice(), createInfo };
}
```

---

## 7. Graphics Pipeline

**File:** `renderer/vulkan_pipeline.cpp:31-116`

### Pipeline Configuration

| State | Setting |
|-------|---------|
| **Topology** | Triangle list |
| **Polygon Mode** | Fill |
| **Cull Mode** | Back face |
| **Front Face** | Clockwise |
| **Depth Clamp** | Disabled |
| **Blending** | Disabled |

### Vertex Input
**File:** `renderer/vulkan_pipeline.cpp:52-60`

```cpp
vk::VertexInputBindingDescription binding{ 
    0, 20, vk::VertexInputRate::eVertex 
};

std::array<vk::VertexInputAttributeDescription, 2> attribs = {
    vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, 0),   // pos
    vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, 8)  // color
};
```

### Dynamic States
**File:** `renderer/vulkan_pipeline.cpp:66-70`

```cpp
std::vector dynamicStates = {
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
};
```

### Dynamic Rendering
**File:** `renderer/vulkan_pipeline.cpp:100-103`

Uses Vulkan 1.3 dynamic rendering (no render pass):
```cpp
vk::PipelineRenderingCreateInfo{ 
    .colorAttachmentCount = 1, 
    .pColorAttachmentFormats = &format 
};
```

---

## 8. Rendering

**File:** `renderer/vulkan_renderer.cpp:40-105`

### Command Buffer Recording

1. **Image Layout Transition:** Using Synchronization2 barriers
2. **Begin Rendering:** `cmd.beginRendering()`
3. **Bind Pipeline:** Graphics pipeline
4. **Set Dynamic State:** Viewport and scissor
5. **Bind Vertex Buffer:** Single binding at 0
6. **Bind Index Buffer:** Uint16 indices
7. **Draw Indexed:** Single triangle

### Frame Rendering Loop
**File:** `renderer/vulkan_renderer.cpp:169-221`

```cpp
void VulkanRenderer::renderFrame()
{
    // 1. Wait for previous frame fence
    dev.waitForFences(*drawFence, vk::True, UINT64_MAX);
    
    // 2. Acquire swapchain image
    auto [result, imageIndex] = swapchain->getSwapchain().acquireNextImage(
        UINT64_MAX, *presentCompleteSemaphore, nullptr
    );
    
    // 3. Reset fence
    dev.resetFences(*drawFence);
    
    // 4. Record command buffer
    recordCommandBuffer(imageIndex, frameMod);
    
    // 5. Submit to queue
    vk::PipelineStageFlags waitDestinationStageMask(
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    );
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*presentCompleteSemaphore,
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*renderFinishedSemaphore
    };
    graphicsQueue.submit(submitInfo, *drawFence);
    
    // 6. Present
    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &*swapchain->getSwapchain(),
        .pImageIndices = &imageIndex
    };
    graphicsQueue.presentKHR(presentInfoKHR);
}
```

---

## 9. Synchronization

### Sync Objects
**File:** `renderer/vulkan_renderer.cpp:107-129`

**Per Frame-in-Flight:**
- `presentCompleteSemaphore` - Signals when image acquisition completes
- `drawFence` - CPU wait point for frame completion (starts signaled)

**Per Swapchain Image:**
- `renderFinishedSemaphore` - Signals when rendering completes

### Pipeline Barriers (Synchronization2)
**File:** `renderer/vulkan_utils.cpp:3-29`

Uses `vk::ImageMemoryBarrier2` and `cmd.pipelineBarrier2()`:

```cpp
vk::ImageMemoryBarrier2 barrier = {
    .srcStageMask = srcStageMask,
    .srcAccessMask = srcAccessMask,
    .dstStageMask = dstStageMask,
    .dstAccessMask = dstAccessMask,
    .oldLayout = oldLayout,
    .newLayout = newLayout,
    .image = image,
    .subresourceRange = { 
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .levelCount = 1,
        .layerCount = 1 
    }
};

vk::DependencyInfo dependencyInfo = {
    .dependencyFlags = {},
    .imageMemoryBarrierCount = 1,
    .pImageMemoryBarriers = &barrier
};
commandBuffer.pipelineBarrier2(dependencyInfo);
```

### Synchronization Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                        Frame N                                  │
├─────────────────────────────────────────────────────────────────┤
│ 1. waitForFences(drawFence[N])     → CPU waits for frame N-1    │
│ 2. acquireNextImage()             → Image acquired             │
│    Signal: presentCompleteSem[N]                              │
│ 3. resetFences(drawFence[N])                                   │
│ 4. Submit cmdBuffer:                                            │
│    Wait:  presentCompleteSem[N]                                 │
│    Signal: renderFinishedSem[imageIndex]                        │
│ 5. Present:                                                      │
│    Wait:  renderFinishedSem[imageIndex]                         │
│ 6. Return to step 1 (next frame)                                │
└─────────────────────────────────────────────────────────────────┘
```

---

## 10. Descriptor Sets & Resource Binding

**Current Status:** No descriptor sets in use.

**Pipeline Layout:** Empty (no set layouts or push constants)
```cpp
vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ 
    .setLayoutCount = 0, 
    .pushConstantRangeCount = 0 
};
```

**Current Binding Methods:**
- `bindVertexBuffers()` - Direct vertex buffer binding
- `bindIndexBuffer()` - Direct index buffer binding

**Future Extension Points:**
- Uniform buffers for transformation matrices
- Storage buffers for mesh data
- Combined image samplers for textures

---

## 11. Swapchain Management

**File:** `renderer/vulkan_swapchain.cpp`

### Surface Creation
**Lines 46-53:**
```cpp
if(!window.createVulkanSurface(&*inst, &_surface))
    throw std::runtime_error("failed to create window surface!");
```

### Swapchain Configuration

**Format Selection (lines 62-66):**
- Preferred: `eB8G8R8A8Srgb` + `eSrgbNonlinear`
- Fallback: First available format

**Present Mode (lines 67-68):**
- Preferred: `eMailbox` (triple buffering)
- Fallback: `eFifo` (vsync)

**Extent (lines 71-78):**
- Uses current surface extent
- Clamps to min/max supported values if needed

**Image Usage (lines 92):**
```cpp
.imageUsage = vk::ImageUsageFlagBits::eColorAttachment
```

### Swapchain Recreation
**Lines 39-44:**

Triggered by:
- Window resize
- `vk::Result::eErrorOutOfDateKHR`
- `vk::Result::eSuboptimalKHR`

```cpp
void VulkanSwapchain::recreate() {
    device->wait();  // Wait for device idle
    initSwapchain();  // Recreate swapchain and image views
}
```

---

## 12. Notable Patterns & Features

### C++20 Modules
Uses `module` declarations for clean compilation boundaries:
```cpp
module candela.renderer;
export module candela.renderer;
```

### Vulkan-hpp RAII Wrappers
All Vulkan objects use `vk::raii::*` smart pointers for automatic resource management.

### Modern Vulkan 1.3+ Features

| Feature | Benefit |
|---------|---------|
| Dynamic Rendering | No render pass objects needed |
| Synchronization2 | Simpler, more efficient barriers |
| Extended Dynamic State | Runtime viewport/scissor configuration |

### Frame-in-Flight Architecture
**MAX_FRAMES_IN_FLIGHT = 2**

- Allows frame N-1 to still be rendering while frame N begins
- Prevents pipeline stalls from CPU-GPU synchronization

### Validation Layer Debug Names
**File:** `renderer/vulkan_renderer.cpp:119-128`

Objects are named for debugging when validation layers are enabled:
```cpp
if constexpr(enableValidationLayers) {
    setDebugName(device.getDevice(), commandBuffers[i], 
        std::format("commandBuffers[{}]", i));
}
```

### Static Compile-time Conditional
```cpp
#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif
```

---

## 13. Summary of Key Files

| Component | File | Lines |
|-----------|------|-------|
| Module Interface | `renderer/renderer.cppm` | 215 |
| Instance | `renderer/vulkan_instance.cpp` | 135 |
| Device | `renderer/vulkan_device.cpp` | 128 |
| Swapchain | `renderer/vulkan_swapchain.cpp` | 127 |
| Pipeline | `renderer/vulkan_pipeline.cpp` | 116 |
| Command | `renderer/vulkan_command.cpp` | 44 |
| Utilities | `renderer/vulkan_utils.cpp` | 74 |
| Renderer | `renderer/vulkan_renderer.cpp` | 240 |
| Shader | `shaders/shader.slang` | 24 |

---

## 14. Potential Improvements

1. **Descriptor Sets:** Add uniform buffers for camera matrices and material data
2. **Depth Buffer:** Implement depth testing for 3D rendering
3. **Texture Support:** Add combined image samplers for textures
4. **Multi-Threaded Command Recording:** Parallel buffer building
5. **Memory Allocator:** Consider VMA for complex memory management
6. **Error Handling:** More robust swapchain recreation logic
