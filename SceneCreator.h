// Scene Creator
// -------------------------------------------------------------------
// Copyright (C) 2011 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <string>

namespace OpenEngine {
    namespace Display { class BoundedCamera; }
    namespace Scene { 
        class ISceneNode; 
        class MeshNode; 
        class TransformationNode; 
    }
    
    class SceneCreator {
    public:
        static void CreateScene(std::string name, Scene::ISceneNode *scene, 
                                Display::BoundedCamera *cam, Scene::TransformationNode * geomTrans);

        //private:
        static void CreateCornell(Scene::ISceneNode *sceneRoot, Display::BoundedCamera *cam, 
                                  Scene::TransformationNode * geomTrans);
        static void CreateSponza(Scene::ISceneNode *sceneRoot, Display::BoundedCamera *cam, 
                                 Scene::TransformationNode * geomTrans);
        static void CreateDragon(Scene::ISceneNode *sceneRoot, Display::BoundedCamera *cam, 
                                 Scene::TransformationNode * geomTrans);
        static void CreateBunny(Scene::ISceneNode *sceneRoot, Display::BoundedCamera *cam, 
                                Scene::TransformationNode * geomTrans);
        static void CreateTieFighter(Scene::ISceneNode *sceneRoot, Display::BoundedCamera *cam, 
                                     Scene::TransformationNode * geomTrans);

        static Scene::MeshNode* CreateCornellBox();
        static Scene::ISceneNode* CreateSmallBox();
        static Scene::ISceneNode* LoadSponza();
        static Scene::ISceneNode* LoadDragon();
        static Scene::ISceneNode* LoadBunny();
        static Scene::ISceneNode* LoadTieFighter();
    };

}
