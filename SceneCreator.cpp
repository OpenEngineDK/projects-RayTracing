// Scene Creator
// -------------------------------------------------------------------
// Copyright (C) 2011 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "SceneCreator.h"

#include <Display/Camera.h>
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

    void SceneCreator::CreateScene(std::string name, ISceneNode *scene, Camera *cam, 
                                     TransformationNode * geomTrans){
        if (name.compare("sponza") == 0)
            CreateSponza(scene, cam, geomTrans);
        else if (name.compare("dragon") == 0)
            CreateDragon(scene, cam, geomTrans);
        else if (name.compare("bunny") == 0)
            CreateBunny(scene, cam, geomTrans);
        else
            CreateCornell(scene, cam, geomTrans);
    }
    
    void SceneCreator::CreateCornell(ISceneNode *sceneRoot, Camera *cam, 
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
    }

    void SceneCreator::CreateSponza(Scene::ISceneNode *sceneRoot, Display::Camera *cam, 
                                    Scene::TransformationNode * geomTrans){

        logger.info << "Creating Sponza Scene" << logger.end;
        
        geomTrans->SetScale(Vector<3, float>(0.1));
        sceneRoot->AddNode(geomTrans); sceneRoot = geomTrans;
        
        ISceneNode* sponza = LoadSponza();
        sceneRoot->AddNode(sponza);
        
        cam->SetPosition(Vector<3, float>(20.0f, 5.0f, 0.0f));
        cam->LookAt(Vector<3, float>(-1.0f, 5.0f, 0.0f));
    }

    void SceneCreator::CreateDragon(Scene::ISceneNode *sceneRoot, Display::Camera *cam, 
                                    Scene::TransformationNode * geomTrans){
        logger.info << "Creating Stanford Dragon Scene" << logger.end;
        ISceneNode* cornell = CreateCornellBox();
        sceneRoot->AddNode(cornell);

        geomTrans->SetScale(Vector<3, float>(40, 40, 40));
        geomTrans->SetPosition(Vector<3, float>(0, -7, 0));
        sceneRoot->AddNode(geomTrans);
        ISceneNode* dragon = LoadDragon();
        geomTrans->AddNode(dragon);
        
        cam->SetPosition(Vector<3, float>(-4.5f, 3.0f, 4.5f));
        cam->LookAt(Vector<3, float>(-0.8f, -1.0f, 0.0f));
    }

    void SceneCreator::CreateBunny(Scene::ISceneNode *sceneRoot, Display::Camera *cam, 
                                   Scene::TransformationNode * geomTrans){
        logger.info << "Creating Stanford Bunny Scene" << logger.end;
        ISceneNode* cornell = CreateCornellBox();
        sceneRoot->AddNode(cornell);

        geomTrans->SetScale(Vector<3, float>(40, 40, 40));
        geomTrans->SetPosition(Vector<3, float>(0, -7, 0));
        sceneRoot->AddNode(geomTrans);
        ISceneNode* bunny = LoadBunny();
        geomTrans->AddNode(bunny);
        
        //cam->SetPosition(Vector<3, float>(-4.5f, 3.0f, 4.5f));
        //cam->LookAt(Vector<3, float>(-0.8f, -1.0f, 0.0f));
    }

    ISceneNode* SceneCreator::CreateCornellBox() {
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
    
    ISceneNode* SceneCreator::CreateSmallBox() {
        MeshPtr box = MeshCreator::CreateCube(3, 1, Vector<3,float>(1.0f, 1.0f, 1.0f));
        
        IDataBlockPtr color = Float4DataBlockPtr(new DataBlock<4, float>(box->GetGeometrySet()->GetSize()));
        for (unsigned int i = 0; i < color->GetSize(); ++i)
            color->SetElement(i, Vector<4, float>(1.0, 1.0, 1.0, 0.4));
        
        box->GetGeometrySet()->AddAttributeList("color", color);
        
        return new MeshNode(box);
    }

    ISceneNode* SceneCreator::LoadSponza() {
        IModelResourcePtr mdl = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/sponza/Sponza.obj");
        mdl->Load();
        return mdl->GetSceneNode();
    }

    ISceneNode* SceneCreator::LoadDragon() {
        IModelResourcePtr duckRes = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/dragon/dragon_vrip_res2.ply");
        //IModelResourcePtr duckRes = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/bunny/bun_zipper_res4.ply");
        duckRes->Load();
        MeshNode* dragon = (MeshNode*) duckRes->GetSceneNode()->GetNode(0)->GetNode(0)->GetNode(0);

        const Vector<4, float> jadeGreen(0.0f, 0.647, 0.396f, 0.65f);
        GeometrySetPtr geom = dragon->GetMesh()->GetGeometrySet();
        IDataBlockPtr color = Float4DataBlockPtr(new DataBlock<4, float>(geom->GetSize()));
        for (unsigned int i = 0; i < color->GetSize(); ++i)
            color->SetElement(i, jadeGreen);

        geom->AddAttributeList("color", color);        

        return duckRes->GetSceneNode();
        
        //IDataBlockPtr color = Float4DataBlockPtr(new DataBlock<4, float>(box->GetGeometrySet()->GetSize()));

        /*        
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

        //duckRes->Unload();

        return dragon;
        */        
    }

    ISceneNode* SceneCreator::LoadBunny() {
        IModelResourcePtr duckRes = ResourceManager<IModelResource>::Create("projects/PhotonMapping/data/bunny/bun_zipper_res2.ply");
        duckRes->Load();
        MeshNode* bunny = (MeshNode*) duckRes->GetSceneNode()->GetNode(0)->GetNode(0)->GetNode(0);

        const Vector<4, float> bakersChocolate(0.36f, 0.2, 0.09f, 1.0f);
        const Vector<4, float> lightChocolate(0.667f, 0.49f, 0.361f, 1.0f);
        GeometrySetPtr geom = bunny->GetMesh()->GetGeometrySet();
        IDataBlockPtr color = Float4DataBlockPtr(new DataBlock<4, float>(geom->GetSize()));
        for (unsigned int i = 0; i < color->GetSize(); ++i)
            color->SetElement(i, bakersChocolate);

        geom->AddAttributeList("color", color);        

        return duckRes->GetSceneNode();
    }    
    
}
