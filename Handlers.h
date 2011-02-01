// Ray tracing handlers
// -------------------------------------------------------------------
// Copyright (C) 2011 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include <Core/IListener.h>
#include <Devices/IKeyboard.h>
#include <Math/Vector.h>
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
        class TriangleMapWrapper;
        
        Display::AntTweakBar* atb;
        Renderers::OpenGL::PhotonRenderingView* rv;
        Renderers::IRenderer* renderer;
        RVRayTracer* ray;
        TriangleMapWrapper* triangleMap;
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
        Math::Vector<2, int> screenSize;
        
    public:
        HostTraceListener(Renderers::OpenGL::PhotonRenderingView* rv, Math::Vector<2, int> screen);

        void Handle(Devices::MouseButtonEventArg arg);
    };

    class AntToggler : public Core::IListener<Devices::KeyboardEventArg> {
    protected:
        Display::AntTweakBar* atb;
    public:
        AntToggler(Display::AntTweakBar* atb) : atb(atb) {}
        void Handle(Devices::KeyboardEventArg arg);
    };
}
