#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>

#include "native_window.h"

namespace fl = filament;
namespace flut = utils;
namespace flm = math;

struct Vertex {
  flm::float2 position;
  uint32_t color;
};

static_assert(sizeof(Vertex) == 12, "sizeof(Vertex) != 12");

static const Vertex TRIANGLE_VERTICES[3] = {
  {{1, 0}, 0xffff0000u},
  {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
  {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

static constexpr uint8_t BAKED_COLOR_PACKAGE[] = {
    #include "/home/cgmb/filament/out/cmake-release/samples/generated/material/bakedColor.inc"
};

fl::Engine* g_fengine;
fl::SwapChain* g_fswapchain;
fl::Renderer* g_frenderer;
fl::Camera* g_fcamera;
fl::View* g_fview;
fl::Scene* g_fscene;
flut::Entity g_frenderable;

int g_window_size_x = 512;
int g_window_size_y = 512;
int g_frame_size_x = g_window_size_x;
int g_frame_size_y = g_window_size_y;

void display() {
  {
    // setup projection matrix
    constexpr float k_zoom = 1.5f;
    const uint32_t w = g_fview->getViewport().width;
    const uint32_t h = g_fview->getViewport().height;
    const float aspect = float(w) / h;
    g_fcamera->setProjection(fl::Camera::Projection::ORTHO,
      -aspect * k_zoom, aspect * k_zoom,
      -k_zoom, k_zoom, 0, 1);
  }

  // beginFrame() returns false if we need to skip a frame
  if (g_frenderer->beginFrame(g_fswapchain)) {
      g_frenderer->render(g_fview);
      g_frenderer->endFrame();
  }
}

void init(GLFWwindow* window) {
  fl::Engine* engine = fl::Engine::create();
  fl::SwapChain* swapChain = engine->createSwapChain(getNativeWindow(window));
  fl::Renderer* renderer = engine->createRenderer();

  fl::Camera* camera = engine->createCamera();
  fl::View* view = engine->createView();
  fl::Scene* scene = engine->createScene();

  view->setViewport({0, 0, uint32_t(g_frame_size_x), uint32_t(g_frame_size_y)});

  {
    // setup view matrix
    flm::double3 eye(0);
    flm::double3 at(0,0,-1);

    flm::double3 mTranslation = eye;
    flm::double3 dt = at - eye;
    double yz_length = std::sqrt((dt.y * dt.y) + (dt.z * dt.z));
    flm::double3 mRotation;
    mRotation.z = 0.0;
    mRotation.x = std::atan2(dt.y, -dt.z);
    mRotation.y = std::atan2(dt.x, yz_length);

    flm::mat4 rotate_z  = flm::mat4::rotate(mRotation.z, flm::double3(0, 0, 1));
    flm::mat4 rotate_x  = flm::mat4::rotate(mRotation.x, flm::double3(1, 0, 0));
    flm::mat4 rotate_y  = flm::mat4::rotate(mRotation.y, flm::double3(0, 1, 0));
    flm::mat4 translate = flm::mat4::translate(mTranslation);
    flm::mat4 view = translate * (rotate_y * rotate_x * rotate_z);
    camera->setModelMatrix(flm::mat4f(view));

/*
    // setup projection matrix
    double fovx = 65.0;
    double clipNear = 0.1;
    double clipFar = 11.0;
    double aspect = double(g_frame_size_x) / g_frame_size_y;
    camera->setProjection(fovx, aspect, clipNear, clipFar);
*/
  }

  view->setClearColor({0.1, 0.125, 0.25, 1.0});
  view->setPostProcessingEnabled(false);
  view->setDepthPrepass(fl::View::DepthPrepass::DISABLED);

  fl::VertexBuffer* vb = fl::VertexBuffer::Builder()
          .vertexCount(3)
          .bufferCount(1)
          .attribute(fl::VertexAttribute::POSITION, 0, fl::VertexBuffer::AttributeType::FLOAT2, 0, 12)
          .attribute(fl::VertexAttribute::COLOR, 0, fl::VertexBuffer::AttributeType::UBYTE4, 8, 12)
          .normalized(fl::VertexAttribute::COLOR)
          .build(*engine);
  vb->setBufferAt(*engine, 0,
          fl::VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));
  fl::IndexBuffer* ib = fl::IndexBuffer::Builder()
          .indexCount(3)
          .bufferType(fl::IndexBuffer::IndexType::USHORT)
          .build(*engine);
  ib->setBuffer(*engine,
          fl::IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
  fl::Material* material = fl::Material::Builder()
          .package((void*)BAKED_COLOR_PACKAGE, sizeof(BAKED_COLOR_PACKAGE))
          .build(*engine);

  flut::Entity renderable = flut::EntityManager::get().create();
  fl::RenderableManager::Builder(1)
          .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
          .material(0, material->getDefaultInstance())
          .geometry(0, fl::RenderableManager::PrimitiveType::TRIANGLES, vb, ib, 0, 3)
          .culling(false)
          .receiveShadows(false)
          .castShadows(false)
          .build(*engine, renderable);
  scene->addEntity(renderable);

  view->setCamera(camera);
  view->setScene(scene);

  g_fengine = engine;
  g_fswapchain = swapChain;
  g_frenderer = renderer;
  g_fcamera = camera;
  g_fview = view;
  g_frenderable = renderable;
  g_fscene = scene;

  // the transform was automatically created for us,
  // so let's grab it and set it
  auto& tcm = g_fengine->getTransformManager();
  tcm.setTransform(tcm.getInstance(renderable),
    flm::mat4f::rotate(M_PI_4, flm::float3{0, 0, 1}));
}

void key_press(GLFWwindow* window, int key, int scancode, int action,
                   int mods) {
  (void)window; (void)mods; (void)scancode; (void)action;
  if (action != GLFW_PRESS) {
    return;
  }

  if (key == GLFW_KEY_Q) {
    exit(0);
  }
}

void reshape_framebuffer(GLFWwindow* window, int w, int h) {
  (void)window; (void)w; (void)h;
  // probably should be doing something here
}

void reshape_window(GLFWwindow* window, int w, int h) {
  (void)window; (void)w; (void)h;
  // probably should be doing something here too
}

struct Terminator {
  ~Terminator() { glfwTerminate(); }
};

int main() {
  if (!glfwInit()) {
    fprintf(stderr, "GLFW initialization failed\n");
    exit(1);
  }
  Terminator t;

  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 2);

  GLFWwindow* window = glfwCreateWindow(g_window_size_x, g_window_size_y,
      "Filament GLFW Example", nullptr, nullptr);
  if (!window) {
    fprintf(stderr, "GLFW window creation failed\n");
    exit(1);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    fprintf(stderr, "GLAD initialization failed\n");
    exit(1);
  }

  glfwSetWindowSizeCallback(window, reshape_window);
  glfwSetFramebufferSizeCallback(window, reshape_framebuffer);
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwGetFramebufferSize(window, &g_window_size_x, &g_window_size_y);
  glfwSetKeyCallback(window, key_press);

  init(window);

  reshape_window(window, g_window_size_x, g_window_size_y);
  reshape_framebuffer(window, g_window_size_x, g_window_size_y);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    display();
    glfwSwapBuffers(window);
  }
  return 0;
}
