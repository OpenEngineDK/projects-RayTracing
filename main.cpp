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
#include <Renderers/DataBlockBinder.h>
#include <Resources/ResourceManager.h>
#include <Scene/PointLightNode.h>
#include <Scene/SceneNode.h>
#include <Scene/RenderStateNode.h>

// SDL extension
#include <Display/SDLEnvironment.h>

// Assimp
#include <Resources/AssimpResource.h>

// OpenGL
#include <Renderers/OpenGL/Renderer.h>
#include <Renderers/OpenGL/LightRenderer.h>
#include <Display/RenderCanvas.h>
#include <Display/OpenGL/TextureCopy.h>
#include <Display/OpenGL/FrameBufferBackend.h>

// Generic Handler
#include <Utils/BetterMoveHandler.h>
#include <Utils/QuitHandler.h>

// Mesh Utils
#include <Utils/MeshCreator.h>
#include <Utils/MeshTransformer.h>

// Photon Mapping stuff
#include <Renderers/OpenGL/PhotonRenderingView.h>

#include <Utils/CUDA/Utils.h>

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

ISceneNode* SetupScene(){

    RenderStateNode* rsNode = new RenderStateNode();
    rsNode->DisableOption(RenderStateNode::BACKFACE);
    rsNode->EnableOption(RenderStateNode::LIGHTING);
    rsNode->EnableOption(RenderStateNode::COLOR_MATERIAL);

    TransformationNode* lightTrans = new TransformationNode();
    lightTrans->SetPosition(Vector<3, float>(0.0f, 4.0f, 0.0f));
    rsNode->AddNode(lightTrans);

    PointLightNode* light = new PointLightNode();
    Vector<4, float> lightColor = Vector<4, float>(255.0f, 235.0f, 205.0f, 255.0f) / 255.0f;
    light->ambient = lightColor * 0.3;
    light->diffuse = lightColor * 0.7;
    light->specular = lightColor * 0.3;
    lightTrans->AddNode(light);

    MeshPtr box = MeshCreator::CreateCube(10, 1, Vector<3,float>(1.0f, 1.0f, 1.0f), true);
    Vector<4,float> red(1.0f, 0.0f, 0.0f, 1.0f);
    Vector<4,float> blue(0.0f, 0.0f, 0.8f, 1.0f);
    IDataBlockPtr colors = box->GetGeometrySet()->GetAttributeList("color");
    colors->SetElement(8, red);
    colors->SetElement(9, red);
    colors->SetElement(10, red);
    colors->SetElement(11, red);
    colors->SetElement(12, blue);
    colors->SetElement(13, blue);
    colors->SetElement(14, blue);
    colors->SetElement(15, blue);
    MeshNode* cornellBox = new MeshNode(box);
    rsNode->AddNode(cornellBox);

    // Dragon

    TransformationNode* dragonTrans = new TransformationNode();
    dragonTrans->SetScale(Vector<3, float>(40, 40, 40));
    dragonTrans->SetPosition(Vector<3, float>(0, -7, 0));
    rsNode->AddNode(dragonTrans);

    IModelResourcePtr duckRes = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/dragon/dragon_vrip_res4.ply");
    duckRes->Load();
    MeshNode* dragon = (MeshNode*) duckRes->GetSceneNode()->GetNode(0)->GetNode(0);

    GeometrySetPtr dragonGeom = dragon->GetMesh()->GetGeometrySet();
    IDataBlockPtr color = dragonGeom->GetDataBlock("color");
    if (color != NULL){
        *color *= 0.0f;
        *color += Vector<3, float>(0.0f, 165.0f/255.0f, 101.0f/255.0f);
    }else{
        IDataBlockPtr vertices = dragonGeom->GetDataBlock("vertex");
        float *c = new float[vertices->GetSize() * 4];
        for (unsigned int i = 0; i < vertices->GetSize() * 4; ){
            c[i++] = 0.0f;
            c[i++] = 165.0f/255.0f;
            c[i++] = 101.0f/255.0f;
            c[i++] = 1.0f;
        }
        color = Float4DataBlockPtr(new DataBlock<4, float>(vertices->GetSize(), c));
        dragonGeom = GeometrySetPtr(new GeometrySet(vertices, 
                                                    dragonGeom->GetDataBlock("normal"),
                                                    IDataBlockList(), color));
        Mesh* dragonMesh = new Mesh(dragon->GetMesh()->GetIndices(),
                                    dragon->GetMesh()->GetType(),
                                    dragonGeom, dragon->GetMesh()->GetMaterial());
        dragon = new MeshNode(MeshPtr(dragonMesh));
    }

    duckRes->Unload();
    dragonTrans->AddNode(dragon);
    
    return rsNode;
}

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
    
    IRenderer* renderer = new Renderer();

    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin());

    ISceneNode* scene = SetupScene();

    DataBlockBinder* bob = new DataBlockBinder(*renderer);
    bob->Bind(*scene);

    IFrame& frame = env->CreateFrame();

    Camera* camera  = new Camera(*(new PerspectiveViewingVolume(1, 4000)));

    PhotonRenderingView* renderingview = new PhotonRenderingView();
    renderer->InitializeEvent().Attach(*renderingview);    
    renderer->PreProcessEvent().Attach(*renderingview);
    renderer->ProcessEvent().Attach(*renderingview);

    LightRenderer *lightRenderer = new LightRenderer();
    renderer->PreProcessEvent().Attach(*lightRenderer);

    RenderCanvas* canvas = new RenderCanvas(new TextureCopy());
    //RenderCanvas* canvas = new RenderCanvas(new FrameBufferBackend(renderer));
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
