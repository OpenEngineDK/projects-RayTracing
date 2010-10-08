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
#include <Display/OpenGL/RenderCanvas.h>

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

    ISceneNode* cornellBox = new SceneNode();
    rsNode->AddNode(cornellBox);

    MeshPtr plane = MeshCreator::CreatePlane(10, 1);
    IDataBlockPtr c = IDataBlockPtr(new DataBlock<4, float>(plane->GetGeometrySet()->GetSize(),
                                                                Vector<4, float>(1.0f)));
    plane->GetGeometrySet()->AddAttributeList("color", c);
    
    MeshPtr top = MeshTransformer::Rotate(plane, Quaternion<float>(PI, 0.0f, 0.0f));
    top = MeshTransformer::Translate(top, Vector<3,float>(0.0f, 5.0f, 0.0f));
    MeshNode* topNode = new MeshNode(top);
    cornellBox->AddNode(topNode);

    MeshPtr bottom = MeshTransformer::Translate(plane, Vector<3,float>(0.0f, -5.0f, 0.0f));
    MeshNode* bottomNode = new MeshNode(bottom);
    cornellBox->AddNode(bottomNode);
    
    MeshPtr back = MeshTransformer::Rotate(plane, Quaternion<float>(PI/2.0f, 0.0f, 0.0f));
    back = MeshTransformer::Translate(back, Vector<3,float>(0.0f, .0f, -5.0f));
    MeshNode* backNode = new MeshNode(back);
    cornellBox->AddNode(backNode);

    MeshPtr front = MeshTransformer::Rotate(plane, Quaternion<float>(PI/-2.0f, 0.0f, 0.0f));
    front = MeshTransformer::Translate(front, Vector<3,float>(0.0f, .0f, 5.0f));
    MeshNode* frontNode = new MeshNode(front);
    cornellBox->AddNode(frontNode);

    plane = MeshCreator::CreatePlane(10, 1, Vector<3, float>(1.0f, 0.0f, 0.0f));
    c = IDataBlockPtr(new DataBlock<4, float>(plane->GetGeometrySet()->GetSize(),
                                              Vector<4, float>(1.0f, 0.0f, 0.0f, 1.0f)));
    plane->GetGeometrySet()->AddAttributeList("color", c);
    MeshPtr left = MeshTransformer::Rotate(plane, Quaternion<float>(0.0f, 0.0f, PI/-2.0f));
    left = MeshTransformer::Translate(left, Vector<3,float>(-5.0f, 0.0f, 0.0f));
    MeshNode* leftNode = new MeshNode(left);
    cornellBox->AddNode(leftNode);

    plane = MeshCreator::CreatePlane(10, 1, Vector<3, float>(0.0f, 0.0f, 0.8f));
    c = IDataBlockPtr(new DataBlock<4, float>(plane->GetGeometrySet()->GetSize(),
                                              Vector<4, float>(0.0f, 0.0f, 0.8f, 1.0f)));
    plane->GetGeometrySet()->AddAttributeList("color", c);
    MeshPtr right = MeshTransformer::Rotate(plane, Quaternion<float>(0.0f, 0.0f, PI/2.0f));
    right = MeshTransformer::Translate(right, Vector<3,float>(5.0f, 0.0f, 0.0f));
    MeshNode* rightNode = new MeshNode(right);
    cornellBox->AddNode(rightNode);

    // Dragon

    TransformationNode* dragonTrans = new TransformationNode();
    dragonTrans->SetScale(Vector<3, float>(40, 40, 40));
    dragonTrans->SetPosition(Vector<3, float>(0, -7, 0));
    cornellBox->AddNode(dragonTrans);

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
