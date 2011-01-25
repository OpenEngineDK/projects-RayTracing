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
    namespace Display { class Camera; }
    namespace Scene { 
        class ISceneNode; 
        class TransformationNode; 
    }
    
    class SceneCreator {
    public:
        static void CreateScene(std::string name, Scene::ISceneNode *scene, 
                                Display::Camera *cam, Scene::TransformationNode * geomTrans);

        //private:
        static void CreateCornell(Scene::ISceneNode *sceneRoot, Display::Camera *cam, 
                                  Scene::TransformationNode * geomTrans);
        static void CreateSponza(Scene::ISceneNode *sceneRoot, Display::Camera *cam, 
                                 Scene::TransformationNode * geomTrans);
        static void CreateDragon(Scene::ISceneNode *sceneRoot, Display::Camera *cam, 
                                 Scene::TransformationNode * geomTrans);
        static void CreateBunny(Scene::ISceneNode *sceneRoot, Display::Camera *cam, 
                                Scene::TransformationNode * geomTrans);

        static Scene::ISceneNode* CreateCornellBox();
        static Scene::ISceneNode* CreateSmallBox();
        static Scene::ISceneNode* LoadSponza();
        static Scene::ISceneNode* LoadDragon();
        static Scene::ISceneNode* LoadBunny();
    };

}
