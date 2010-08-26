// main
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// OpenEngine stuff
#include <Core/Engine.h>
#include <Display/Camera.h>
#include <Display/PerspectiveViewingVolume.h>
#include <Logging/Logger.h>
#include <Logging/StreamLogger.h>
#include <Meta/Config.h>
#include <Scene/SceneNode.h>

// SDL extension
#include <Display/SDLEnvironment.h>

// OpenGL
#include <Renderers/OpenGL/Renderer.h>
#include <Display/OpenGL/RenderCanvas.h>

// Generic Handler
#include <Utils/BetterMoveHandler.h>
#include <Utils/QuitHandler.h>

// Photon Mapping stuff
#include <Renderers/OpenGL/PhotonRenderingView.h>

// name spaces that we will be using.
// this combined with the above imports is almost the same as
// fx. import OpenEngine.Logging.*; in Java.
using namespace OpenEngine::Core;
using namespace OpenEngine::Display;
using namespace OpenEngine::Display::OpenGL;
using namespace OpenEngine::Logging;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Utils;

/**
 * Main method for the first quarter project of CGD.
 * Corresponds to the
 *   public static void main(String args[])
 * method in Java.
 */
int main(int argc, char** argv) {
    // Setup logging facilities.
    Logger::AddLogger(new StreamLogger(&std::cout));

    // Print usage info.
    logger.info << "========= Efficient algorithms for Global Illumination =========" << logger.end;

    // setup the engine
    Engine* engine = new Engine;
    IEnvironment* env = new SDLEnvironment(800, 600, 32);
    engine->InitializeEvent().Attach(*env);
    engine->ProcessEvent().Attach(*env);
    engine->DeinitializeEvent().Attach(*env);
    
    IFrame& frame = env->CreateFrame();

    Camera* camera  = new Camera(*(new PerspectiveViewingVolume(1, 4000)));

    PhotonRenderingView* renderingview = new PhotonRenderingView();
    IRenderer* renderer = new Renderer();
    renderer->InitializeEvent().Attach(*renderingview);    
    renderer->PreProcessEvent().Attach(*renderingview);
    renderer->ProcessEvent().Attach(*renderingview);

    ISceneNode* scene = new SceneNode();

    RenderCanvas* canvas = new RenderCanvas();
    canvas->SetViewingVolume(camera);
    canvas->SetRenderer(renderer);
    canvas->SetScene(scene);
    frame.SetCanvas(canvas);

    QuitHandler* quit_h = new QuitHandler(*engine);
    env->GetKeyboard()->KeyEvent().Attach(*quit_h);

    BetterMoveHandler *move = new BetterMoveHandler(*camera,
                                                    *(env->GetMouse()),
                                                    true);

    engine->InitializeEvent().Attach(*move);
    engine->ProcessEvent().Attach(*move);
    env->GetKeyboard()->KeyEvent().Attach(*move);
    env->GetMouse()->MouseButtonEvent().Attach(*move);
    env->GetMouse()->MouseMovedEvent().Attach(*move);

    // Start the engine.
    engine->Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}


