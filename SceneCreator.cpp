// Scene Creator
// -------------------------------------------------------------------
// Copyright (C) 2011 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "SceneCreator.h"

#include "Display/BoundedCamera.h"
#include <Geometry/GeometrySet.h>
#include <Geometry/Mesh.h>
#include <Resources/IDataBlock.h>
#include <Resources/IModelResource.h>
#include <Resources/ResourceManager.h>
#include <Scene/ISceneNode.h>
#include <Scene/MeshNode.h>
#include <Scene/TransformationNode.h>
#include <Utils/MeshCreator.h>

using namespace std;

namespace OpenEngine {
    using namespace Display;
    using namespace Resources;
    using namespace Scene;
    using namespace Utils;

    void SceneCreator::CreateScene(std::string name, ISceneNode *scene, BoundedCamera *cam, 
                                     TransformationNode * geomTrans){
        if (name.compare("sponza") == 0)
            CreateSponza(scene, cam, geomTrans);
        else if (name.compare("dragon") == 0)
            CreateDragon(scene, cam, geomTrans);
        else if (name.compare("bunny") == 0)
            CreateBunny(scene, cam, geomTrans);
        else if (name.compare("cornell") == 0)
            CreateCornell(scene, cam, geomTrans);
        else        {
            logger.info << "Unknown scene, defaulting to Cornell Box" << logger.end;
            CreateCornell(scene, cam, geomTrans);
        }
    }
    
    void SceneCreator::CreateCornell(ISceneNode *sceneRoot, BoundedCamera *cam, 
                                     TransformationNode *geomTrans){
        logger.info << "Creating Cornell Box Scene" << logger.end;
        ISceneNode* cornell = CreateCornellBox();
        sceneRoot->AddNode(cornell);

        ISceneNode* box = SceneCreator::CreateSmallBox();
        geomTrans->SetRotation(Quaternion<float>(0.0f, -Math::PI/8.0f, 0.0f));
        geomTrans->Move(2.0f, -3.48f, 1.0);
        sceneRoot->AddNode(geomTrans);
        geomTrans->AddNode(box);
        
        ISceneNode* bigBox = SceneCreator::CreateSmallBox();
        TransformationNode* bigTrans = new TransformationNode();
        bigTrans->SetRotation(Quaternion<float>(0.0f, Math::PI/8.0f, 0.0f));
        bigTrans->Move(-1.5f, -1.98f, -3.0);
        bigTrans->SetScale(Vector<3, float>(1.0f, 2.0f, 1.0f));
        sceneRoot->AddNode(bigTrans);
        bigTrans->AddNode(bigBox);

        cam->SetPosition(Vector<3, float>(-4.5f, 3.0f, 4.5f));
        cam->LookAt(Vector<3, float>(-0.8f, -1.0f, 0.0f));

        cam->SetMinimumBound(Vector<3, float>(-4.9f));
        cam->SetMaximumBound(Vector<3, float>(4.9f));
        
        //cam->SetPosition(Vector<3, float>(0.0f, 2.0f, 9.5f));
        //cam->LookAt(Vector<3, float>(0.0f, 0.0f, 0.0f));
    }

    void SceneCreator::CreateSponza(Scene::ISceneNode *sceneRoot, Display::BoundedCamera *cam, 
                                    Scene::TransformationNode * geomTrans){

        logger.info << "Creating Sponza Scene" << logger.end;
        
        geomTrans->SetScale(Vector<3, float>(0.1));
        sceneRoot->AddNode(geomTrans); sceneRoot = geomTrans;
        
        ISceneNode* sponza = LoadSponza();
        sceneRoot->AddNode(sponza);
        
        cam->SetPosition(Vector<3, float>(20.0f, 5.0f, 0.0f));
        cam->LookAt(Vector<3, float>(-1.0f, 9.0f, 0.0f));
    }

    void SceneCreator::CreateDragon(Scene::ISceneNode *sceneRoot, Display::BoundedCamera *cam, 
                                    Scene::TransformationNode * geomTrans){
        logger.info << "Creating Stanford Dragon Scene" << logger.end;
        MeshNode* cornell = CreateCornellBox();
        sceneRoot->AddNode(cornell);

        geomTrans->SetScale(Vector<3, float>(40, 40, 40));
        geomTrans->SetPosition(Vector<3, float>(1, -7, -2));
        geomTrans->SetRotation(Quaternion<float>(0.0f, 0.5f, 0.0f));
        sceneRoot->AddNode(geomTrans);
        ISceneNode* dragon = LoadDragon();
        geomTrans->AddNode(dragon);
        
        cam->SetPosition(Vector<3, float>(0.0f, 3.0f, 4.5f));
        cam->LookAt(Vector<3, float>(0.0f, 0.0f, 0.0f));

        cam->SetMinimumBound(Vector<3, float>(-4.9f));
        cam->SetMaximumBound(Vector<3, float>(4.9f));

    }

    void SceneCreator::CreateBunny(Scene::ISceneNode *sceneRoot, Display::BoundedCamera *cam, 
                                   Scene::TransformationNode * geomTrans){
        logger.info << "Creating Stanford Bunny Scene" << logger.end;
        MeshNode* cornell = CreateCornellBox();
        sceneRoot->AddNode(cornell);

        geomTrans->SetScale(Vector<3, float>(40, 40, 40));
        geomTrans->SetPosition(Vector<3, float>(0, -6.5, 0));
        sceneRoot->AddNode(geomTrans);
        ISceneNode* bunny = LoadBunny();
        geomTrans->AddNode(bunny);
        
        cam->SetMinimumBound(Vector<3, float>(-4.9f));
        cam->SetMaximumBound(Vector<3, float>(4.9f));

        //cam->SetPosition(Vector<3, float>(-4.5f, 3.0f, 4.5f));
        //cam->LookAt(Vector<3, float>(-0.8f, -1.0f, 0.0f));
    }

    MeshNode* SceneCreator::CreateCornellBox() {
        MeshPtr box = MeshCreator::CreateCube(10, 1, Vector<3,float>(1.0f, 1.0f, 1.0f), true);
        
        IDataBlockPtr color = Float4DataBlockPtr(new DataBlock<4, float>(box->GetGeometrySet()->GetSize()));
        for (unsigned int i = 0; i < color->GetSize(); ++i)
            color->SetElement(i, Vector<4, float>(1.0, 1.0, 1.0, 1.0));
        
        box->GetGeometrySet()->AddAttributeList("color", color);

        IDataBlockPtr colors = box->GetGeometrySet()->GetColors();
        Vector<4,float> red(1.0f, 0.0f, 0.0f, 1.0f);
        colors->SetElement(8, red);
        colors->SetElement(9, red);
        colors->SetElement(10, red);
        colors->SetElement(11, red);
        Vector<4,float> blue(0.0f, 0.0f, 0.8f, 1.0f);
        colors->SetElement(12, blue);
        colors->SetElement(13, blue);
        colors->SetElement(14, blue);
        colors->SetElement(15, blue);
        // Vector<4,float> transparent(0.0f, 0.0f, 0.8f, 0.0f);
        // colors->SetElement(16, transparent);
        // colors->SetElement(17, transparent);
        // colors->SetElement(18, transparent);
        // colors->SetElement(19, transparent);
        
        return new MeshNode(box);
    }
    
    ISceneNode* SceneCreator::CreateSmallBox() {
        MeshPtr box = MeshCreator::CreateCube(3, 1, Vector<3,float>(1.0f, 1.0f, 1.0f));
        
        IDataBlockPtr color = Float4DataBlockPtr(new DataBlock<4, float>(box->GetGeometrySet()->GetSize()));
        for (unsigned int i = 0; i < color->GetSize(); ++i)
            color->SetElement(i, Vector<4, float>(1.0, 1.0, 1.0, 1.0));
        
        box->GetGeometrySet()->AddAttributeList("color", color);
        
        return new MeshNode(box);
    }

    ISceneNode* SceneCreator::LoadSponza() {
        IModelResourcePtr mdl = ResourceManager<IModelResource>::Create("sponza/Sponza.obj");
        mdl->Load();
        return mdl->GetSceneNode();
    }

    ISceneNode* SceneCreator::LoadDragon() {
        IModelResourcePtr duckRes = ResourceManager<IModelResource>::Create("dragon/dragon_vrip_res2.ply");
        duckRes->Load();
        MeshNode* dragon = (MeshNode*) duckRes->GetSceneNode()->GetNode(0)->GetNode(0)->GetNode(0);

        const Vector<4, float> jadeGreen(0.0f, 0.647, 0.396f, 0.5f);
        GeometrySetPtr geom = dragon->GetMesh()->GetGeometrySet();
        IDataBlockPtr color = Float4DataBlockPtr(new DataBlock<4, float>(geom->GetSize()));
        for (unsigned int i = 0; i < color->GetSize(); ++i)
            color->SetElement(i, jadeGreen);

        geom->AddAttributeList("color", color);        

        return duckRes->GetSceneNode();
    }

    ISceneNode* SceneCreator::LoadBunny() {
        IModelResourcePtr duckRes = ResourceManager<IModelResource>::Create("bunny/bun_zipper.ply");
        duckRes->Load();
        MeshNode* bunny = (MeshNode*) duckRes->GetSceneNode()->GetNode(0)->GetNode(0)->GetNode(0);

        const Vector<4, float> bakersChocolate(0.36f, 0.2, 0.09f, 0.7f);
        const Vector<4, float> lightChocolate(0.667f, 0.49f, 0.361f, 1.0f);
        GeometrySetPtr geom = bunny->GetMesh()->GetGeometrySet();
        IDataBlockPtr color = Float4DataBlockPtr(new DataBlock<4, float>(geom->GetSize()));
        for (unsigned int i = 0; i < color->GetSize(); ++i)
            color->SetElement(i, bakersChocolate);

        geom->AddAttributeList("color", color);        

        return duckRes->GetSceneNode();
    }    
    
}
