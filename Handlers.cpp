// Ray tracing handlers
// -------------------------------------------------------------------
// Copyright (C) 2011 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "Handlers.h"

#include <Display/AntTweakBar.h>
#include <Renderers/OpenGL/PhotonRenderingView.h>
#include <Renderers/IRenderer.h>
#include <Scene/TransformationNode.h>
#include <Utils/IInspector.h>
#include <Utils/InspectionBar.h>
#include <Utils/CUDA/IRayTracer.h>
#include <Utils/CUDA/TriangleMap.h>

#include <Logging/Logger.h>

namespace OpenEngine {
    using namespace Utils;
    using namespace Renderers;
    using namespace Renderers::OpenGL;
    using namespace Scene;

    class RayInspectionBar::RVRayTracer {
    public:
        PhotonRenderingView* rv;
        RVRayTracer(PhotonRenderingView* rv) : rv(rv) {}
        
        void SetVisualizeRays(bool v) {rv->GetRayTracer()->SetVisualizeRays(v);}
        bool GetVisualizeRays() { return rv->GetRayTracer()->GetVisualizeRays(); }
        void SetIntersectionAlgorithm(IRayTracer::IntersectionAlgorithm a) { rv->GetRayTracer()->SetIntersectionAlgorithm(a);}
        IRayTracer::IntersectionAlgorithm GetIntersectionAlgorithm() { return rv->GetRayTracer()->GetIntersectionAlgorithm(); }
        void SetLeafSkipping(bool s) { rv->GetRayTracer()->SetLeafSkipping(s); }
        bool GetLeafSkipping() { return rv->GetRayTracer()->GetLeafSkipping(); }
        void PrintTiming(bool p) { rv->GetRayTracer()->PrintTiming(p);}
        bool GetPrintTiming() { return rv->GetRayTracer()->GetPrintTiming(); }
    };

    RayInspectionBar::RayInspectionBar(AntTweakBar* atb, 
                                       PhotonRenderingView* rv,
                                       IRenderer* renderer,
                                       TransformationNode* geomTrans) :
        atb(atb), rv(rv), renderer(renderer), ray(new RVRayTracer(rv)), geomTrans(geomTrans){

        renderer->InitializeEvent().Attach(*this);
    }

    void RayInspectionBar::Handle(RenderingEventArg) {
        ValueList values;
        
        EnumRWValueCall<PhotonRenderingView, PhotonRenderingView::RayTracerType> *traceType
            = new EnumRWValueCall<PhotonRenderingView, PhotonRenderingView::RayTracerType>
            (*rv, &PhotonRenderingView::GetRayTracerType,
             &PhotonRenderingView::SetRayTracerType, "RayTracerType");
        traceType->name = "Ray tracer type";
        traceType->AddEnum("Exhaustive", PhotonRenderingView::EXHAUSTIVE);
        traceType->AddEnum("kd-restart", PhotonRenderingView::KD_RESTART);
        traceType->AddEnum("Short stack", PhotonRenderingView::SHORTSTACK);
        values.push_back(traceType);
        

        EnumRWValueCall<RVRayTracer, IRayTracer::IntersectionAlgorithm> *intsect
            = new EnumRWValueCall<RVRayTracer, IRayTracer::IntersectionAlgorithm>
            (*ray, &RVRayTracer::GetIntersectionAlgorithm,
             &RVRayTracer::SetIntersectionAlgorithm, "IntSectAlg");
        intsect->name = "Intersection Alg";
        intsect->AddEnum("Woop", IRayTracer::WOOP);
        intsect->AddEnum("Moeller", IRayTracer::MOELLER);
        values.push_back(intsect);        

        RWValueCall<RVRayTracer, bool> *visual 
            = new RWValueCall<RVRayTracer, bool>
            (*ray, &RVRayTracer::GetVisualizeRays,
             &RVRayTracer::SetVisualizeRays);
        visual->name = "Visualize rays";
        values.push_back(visual);

        RWValueCall<RVRayTracer, bool> *rayTiming 
            = new RWValueCall<RVRayTracer, bool>
            (*ray, &RVRayTracer::GetPrintTiming,
             &RVRayTracer::PrintTiming);
        rayTiming->name = "Ray tracer time";
        values.push_back(rayTiming);

        RWValueCall<RVRayTracer, bool> *leafSkip
            = new RWValueCall<RVRayTracer, bool>
            (*ray, &RVRayTracer::GetLeafSkipping,
             &RVRayTracer::SetLeafSkipping);
        leafSkip->name = "Leaf Skipping";
        values.push_back(leafSkip);

        RWValueCall<TransformationNode, Vector<3, float> > *posGeom
            = new RWValueCall<TransformationNode, Vector<3, float> >
            (*geomTrans, &TransformationNode::GetPosition,
             &TransformationNode::SetPosition);
        posGeom->name = "Position geometry";
        posGeom->properties[STEP] = Vector<3,float>(0.1f);
        values.push_back(posGeom);
        
        RWValueCall<TransformationNode, Quaternion<float> > *rotateGeom
            = new RWValueCall<TransformationNode, Quaternion<float> >
            (*geomTrans, &TransformationNode::GetRotation,
             &TransformationNode::SetRotation);
        rotateGeom->name = "Rotate geometry";
        values.push_back(rotateGeom);
        
        RWValueCall<PhotonRenderingView, bool> *updateGeom
            = new RWValueCall<PhotonRenderingView, bool>
            (*rv, &PhotonRenderingView::GetTreeUpdate,
             &PhotonRenderingView::SetTreeUpdate);
        updateGeom->name = "Update geometry";
        values.push_back(updateGeom);

        TriangleMap* map = rv->GetTriangleMap();
        ActionValueCall<TriangleMap> *printTree
            = new ActionValueCall<TriangleMap>
            (*map, &TriangleMap::PrintTree);
        printTree->name = "Print kd-tree";
        values.push_back(printTree);

        atb->AddBar(new InspectionBar("Ray Tracing", values));
    }


    HostTraceListener::HostTraceListener(Renderers::OpenGL::PhotonRenderingView* rv)
        : rv(rv) {}

    void HostTraceListener::Handle(Devices::MouseButtonEventArg arg){
        if (arg.button == BUTTON_RIGHT && arg.type == EVENT_PRESS){
            //logger.info << "(" << arg.state.x << ", " << arg.state.y << ")" << logger.end;
            rv->GetRayTracer()->HostTrace(arg.state.x, arg.state.y, 
                                          rv->GetTriangleMap()->GetNodes());
        }
    }
}
