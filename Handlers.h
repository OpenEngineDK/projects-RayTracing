// Ray tracing handlers
// -------------------------------------------------------------------
// Copyright (C) 2011 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Core/IListener.h>
#include <Renderers/IRenderer.h>

namespace OpenEngine {
    namespace Devices { class MouseButtonEventArg; }
    namespace Display { class AntTweakBar; }
    namespace Renderers{
        namespace OpenGL{ class PhotonRenderingView; }
    }
    namespace Scene { class TransformationNode; }
    namespace Utils { class InspectionBar; }
    
    class RayInspectionBar : public Core::IListener<Renderers::RenderingEventArg>{
    private:
        class RVRayTracer;
        
        Display::AntTweakBar* atb;
        Renderers::OpenGL::PhotonRenderingView* rv;
        Renderers::IRenderer* renderer;
        RVRayTracer* ray;
        Scene::TransformationNode* geomTrans;
        
    public:
        RayInspectionBar(Display::AntTweakBar* atb, 
                         Renderers::OpenGL::PhotonRenderingView* rv,
                         Renderers::IRenderer* renderer,
                         Scene::TransformationNode* geomTrans);

        void Handle(Renderers::RenderingEventArg);
    };

    class HostTraceListener : public Core::IListener<Devices::MouseButtonEventArg> {
    private:
        Renderers::OpenGL::PhotonRenderingView* rv;
    public:
        HostTraceListener(Renderers::OpenGL::PhotonRenderingView* rv);

        void Handle(Devices::MouseButtonEventArg arg);
    };
}
