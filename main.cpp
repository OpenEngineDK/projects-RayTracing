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

// Generic Handler
#include <Utils/BetterMoveHandler.h>
#include <Utils/QuitHandler.h>

// Mesh Utils
#include <Utils/MeshCreator.h>
#include <Utils/MeshTransformer.h>

#include <Resources/FreeImage.h>

// Photon Mapping stuff
#include <Renderers/OpenGL/PhotonRenderingView.h>

#include <Utils/CUDA/Utils.h>

// Ant tweak bar
#include "Handlers.h"
#include <Display/AntTweakBar.h>
#include <Utils/IInspector.h>
#include <Utils/InspectionBar.h>


int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

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

ISceneNode* CreateCornellBox() {
    MeshPtr box = MeshCreator::CreateCube(10, 1, Vector<3,float>(1.0f, 1.0f, 1.0f), true);
    
    IDataBlockPtr colors = box->GetGeometrySet()->GetColors();
    Vector<4,float> red(1.0f, 0.0f, 0.0f, 1.0f);
    Vector<4,float> blue(0.0f, 0.0f, 0.8f, 1.0f);
    colors->SetElement(8, red);
    colors->SetElement(9, red);
    colors->SetElement(10, red);
    colors->SetElement(11, red);
    colors->SetElement(12, blue);
    colors->SetElement(13, blue);
    colors->SetElement(14, blue);
    colors->SetElement(15, blue);

    return new MeshNode(box);
}

ISceneNode* CreateSmallBox() {
    MeshPtr box = MeshCreator::CreateCube(3, 1, Vector<3,float>(1.0f, 1.0f, 1.0f));

    return new MeshNode(box);
}

ISceneNode* CreateDragon() {
    IModelResourcePtr duckRes = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/dragon/dragon_vrip_res3.ply");
    //IModelResourcePtr duckRes = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/bunny/bun_zipper_res2.ply");
    duckRes->Load();
    MeshNode* dragon = (MeshNode*) duckRes->GetSceneNode()->GetNode(0)->GetNode(0)->GetNode(0);

    GeometrySetPtr dragonGeom = dragon->GetMesh()->GetGeometrySet();
    IDataBlockPtr color = dragonGeom->GetDataBlock("color");
    if (color != NULL){
        *color *= 0.0f;
        *color += Vector<3, float>(0.0f, 165.0f/255.0f, 101.0f/255.0f);
    }else{
        color = Float4DataBlockPtr(new DataBlock<4, float>(dragonGeom->GetSize()));
        Vector<4, float> jadeGreen(0.0f, 0.647, 0.396f, 0.5f);
        Vector<4, float> bakersChocolate(0.36f, 0.2, 0.09f, 1.0f);
        Vector<4, float> lightChocolate(0.667f, 0.49f, 0.361f, 1.0f);
        for (unsigned int i = 0; i < color->GetSize(); ++i)
            color->SetElement(i, jadeGreen);
            
        dragonGeom = GeometrySetPtr(new GeometrySet(dragonGeom->GetDataBlock("vertex"),
                                                    dragonGeom->GetDataBlock("normal"),
                                                    IDataBlockList(), color));
        Mesh* dragonMesh = new Mesh(dragon->GetMesh()->GetIndices(),
                                    dragon->GetMesh()->GetType(),
                                    dragonGeom, dragon->GetMesh()->GetMaterial());
        dragon = new MeshNode(MeshPtr(dragonMesh));
    }

    duckRes->Unload();
    return dragon;
}

ISceneNode* CreateSponza() {
    IModelResourcePtr mdl = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/sponza/Sponza.obj");
    mdl->Load();
    return mdl->GetSceneNode();
}

ISceneNode* CreateForest() {
    IModelResourcePtr mdl = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/fairyForest/fairyForest.obj");
    mdl->Load();
    return mdl->GetSceneNode();
}

ISceneNode* CreateSibenik() {
    IModelResourcePtr mdl = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/sibenik/sibenik.obj");
    mdl->Load();
    return mdl->GetSceneNode();
}


TransformationNode *geomTrans = new TransformationNode();

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

    ISceneNode* cornellBox = CreateCornellBox();
    //ISceneNode* cornellBox = CreateSponza();
    //ISceneNode* cornellBox = CreateForest();
    //ISceneNode* cornellBox = CreateSibenik();
    rsNode->AddNode(cornellBox);

    ISceneNode* box = CreateSmallBox();
    TransformationNode* smallTrans = new TransformationNode();
    smallTrans->SetRotation(Quaternion<float>(0.0f, -Math::PI/8.0f, 0.0f));
    smallTrans->Move(2.0f, -3.48f, 1.0);
    rsNode->AddNode(smallTrans);
    smallTrans->AddNode(box);
    geomTrans = smallTrans;

    ISceneNode* bigBox = CreateSmallBox();
    TransformationNode* bigTrans = new TransformationNode();
    bigTrans->SetRotation(Quaternion<float>(0.0f, Math::PI/8.0f, 0.0f));
    bigTrans->Move(-1.5f, -1.98f, -3.0);
    bigTrans->SetScale(Vector<3, float>(1.0f, 2.0f, 1.0f));
    rsNode->AddNode(bigTrans);
    bigTrans->AddNode(bigBox);

    // Dragon
    /*
    TransformationNode* dragonTrans = new TransformationNode();
    dragonTrans->SetScale(Vector<3, float>(40, 40, 40));
    dragonTrans->SetPosition(Vector<3, float>(0, -7, 0));
    rsNode->AddNode(dragonTrans);
    ISceneNode* dragon = CreateDragon();
    dragonTrans->AddNode(dragon);
    geomTrans = dragonTrans;
    */

    return rsNode;
}

// Setup ant tweak bar
AntTweakBar* SetupAntTweakBar(PhotonRenderingView* rv, IRenderer* renderer,
                      IKeyboard* keyboard, IMouse* mouse){
    AntTweakBar* atb = new AntTweakBar();
    keyboard->KeyEvent().Attach(*atb);
    mouse->MouseMovedEvent().Attach(*atb);
    mouse->MouseButtonEvent().Attach(*atb);
    atb->AttachTo(*renderer);
    new RayInspectionBar(atb, rv, renderer, geomTrans);
    return atb;
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
    logger.info << "========= Efficient Algorithms for Ray Tracing Dynamic Scenes =========" << logger.end;
    logger.info << "========= Techniques for Efficient Ray Tracing of Dynamic Scenes =========" << logger.end;
    logger.info << "========= Efficient Ray Tracing of Dynamic Scenes on the GPU =========" << logger.end;
    logger.info << "========= Efficient GPU Accelerated Dynamic Ray Tracing =========" << logger.end;

    // setup the engine
    Engine* engine = new Engine;
    IEnvironment* env = new SDLEnvironment(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
    //IEnvironment* env = new SDLEnvironment(160, 120, 32, FRAME_FULLSCREEN);
    //IEnvironment* env = new SDLEnvironment(1024, 768, 32, FRAME_FULLSCREEN);
    //IEnvironment* env = new SDLEnvironment(1440, 900, 32, FRAME_FULLSCREEN);
    engine->InitializeEvent().Attach(*env);
    engine->ProcessEvent().Attach(*env);
    engine->DeinitializeEvent().Attach(*env);
    
    IRenderer* renderer = new Renderer();

    ResourceManager<IModelResource>::AddPlugin(new AssimpPlugin());
    ResourceManager<ITexture2D>::AddPlugin(new FreeImagePlugin());

    ISceneNode* scene = SetupScene();

    //DataBlockBinder* bob = new DataBlockBinder(*renderer);
    //bob->Bind(*scene);

    IFrame& frame = env->CreateFrame();

    Camera* camera  = new Camera(*(new PerspectiveViewingVolume(1, 4000)));
    camera->SetPosition(Vector<3, float>(-4.5f, 3.0f, 4.5f));
    camera->LookAt(Vector<3, float>(-0.8f, -1.0f, 0.0f));
    //camera->SetPosition(Vector<3, float>(0.0f, 0.0f, 0.0f));
    //camera->LookAt(Vector<3, float>(-2.5f, -5.0f, -4.5f));

    PhotonRenderingView* renderingview = new PhotonRenderingView();
    renderer->InitializeEvent().Attach(*renderingview);    
    renderer->PreProcessEvent().Attach(*renderingview);
    renderer->ProcessEvent().Attach(*renderingview);

    LightRenderer *lightRenderer = new LightRenderer();
    renderer->PreProcessEvent().Attach(*lightRenderer);

    RenderCanvas* canvas = new RenderCanvas(new TextureCopy());
    canvas->SetViewingVolume(camera);
    canvas->SetRenderer(renderer);
    canvas->SetScene(scene);
    frame.SetCanvas(canvas);

    QuitHandler* quit_h = new QuitHandler(*engine);
    env->GetKeyboard()->KeyEvent().Attach(*quit_h);

    AntTweakBar* atb = SetupAntTweakBar(renderingview, renderer, 
                                        env->GetKeyboard(), env->GetMouse());

    HostTraceListener* tracer = new HostTraceListener(renderingview, Vector<2, int>(SCREEN_WIDTH, SCREEN_HEIGHT));
    atb->MouseButtonEvent().Attach(*tracer);
    
    BetterMoveHandler *move = new BetterMoveHandler(*camera,
                                                    *(env->GetMouse()),
                                                    true);
    move->SetMoveScale(0.00001);

    engine->InitializeEvent().Attach(*move);
    engine->ProcessEvent().Attach(*move);
    atb->KeyEvent().Attach(*move);
    atb->MouseButtonEvent().Attach(*move);
    atb->MouseMovedEvent().Attach(*move);

    // Start the engine.
    engine->Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}
