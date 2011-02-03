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
    using namespace Math;
    using namespace Renderers;
    using namespace Renderers::OpenGL;
    using namespace Scene;
    using namespace Utils;

    class RayInspectionBar::RVRayTracer {
    private:
        PhotonRenderingView* rv;
    public:
        RVRayTracer(PhotonRenderingView* rv) : rv(rv) {}
        
        void SetVisualizeRays(bool v) {rv->GetRayTracer()->SetVisualizeRays(v);}
        bool GetVisualizeRays() { return rv->GetRayTracer()->GetVisualizeRays(); }
        void SetIntersectionAlgorithm(IRayTracer::IntersectionAlgorithm a) { rv->GetRayTracer()->SetIntersectionAlgorithm(a);}
        IRayTracer::IntersectionAlgorithm GetIntersectionAlgorithm() { return rv->GetRayTracer()->GetIntersectionAlgorithm(); }
        void SetLeafSkipping(bool s) { rv->GetRayTracer()->SetLeafSkipping(s); }
        bool GetLeafSkipping() { return rv->GetRayTracer()->GetLeafSkipping(); }
        void PrintTiming(bool p) { rv->GetRayTracer()->PrintTiming(p);}
        bool GetPrintTiming() { return rv->GetRayTracer()->GetPrintTiming(); }
        void SetRenderTime(float t) { }
        float GetRenderTime() { return rv->GetRayTracer()->GetRenderTime(); }
    };

    class RayInspectionBar::TriangleMapWrapper {
    private:
        PhotonRenderingView* rv;
    public:
        TriangleMapWrapper(PhotonRenderingView* rv) :rv(rv) {}
        
        void PrintTree() { rv->GetTriangleMap()->PrintTree(); }
        void SetConstructionTime(float t) {}
        float GetConstructionTime() { return rv->GetTriangleMap()->GetConstructionTime(); }
        TriangleMap::LowerAlgorithm GetLowerAlgorithm() { return rv->GetTriangleMap()->GetLowerAlgorithm(); }
        void SetLowerAlgorithm(TriangleMap::LowerAlgorithm l) { rv->GetTriangleMap()->SetLowerAlgorithm(l); }
        void SplitEmptySpace(bool s) { rv->GetTriangleMap()->SplitEmptySpace(s); }
        bool IsSplittingEmptySpace() { return rv->GetTriangleMap()->IsSplittingEmptySpace(); }
        void SetSplitMethod(TriangleMap::SplitMethod s) { rv->GetTriangleMap()->SetSplitMethod(s); }
        TriangleMap::SplitMethod GetSplitMethod() { return rv->GetTriangleMap()->GetSplitMethod(); }
        void SetPropagateBoundingBox(bool p) { rv->GetTriangleMap()->SetPropagateBoundingBox(p); }
        bool GetPropagateBoundingBox() { return rv->GetTriangleMap()->GetPropagateBoundingBox(); }
    };

    RayInspectionBar::RayInspectionBar(AntTweakBar* atb, 
                                       PhotonRenderingView* rv,
                                       IRenderer* renderer,
                                       TransformationNode* geomTrans) :
        atb(atb), rv(rv), renderer(renderer), 
        ray(new RVRayTracer(rv)), triangleMap(new TriangleMapWrapper(rv)),
        geomTrans(geomTrans){

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

        RWValueCall<RVRayTracer, bool> *leafSkip
            = new RWValueCall<RVRayTracer, bool>
            (*ray, &RVRayTracer::GetLeafSkipping,
             &RVRayTracer::SetLeafSkipping);
        leafSkip->name = "Leaf Skipping";
        values.push_back(leafSkip);

        RWValueCall<RVRayTracer, float> *rayTiming 
            = new RWValueCall<RVRayTracer, float>
            (*ray, &RVRayTracer::GetRenderTime, &RVRayTracer::SetRenderTime);
        rayTiming->name = "Ray tracer time";
        values.push_back(rayTiming);

        RWValueCall<TransformationNode, Vector<3, float> > *posGeom
            = new RWValueCall<TransformationNode, Vector<3, float> >
            (*geomTrans, &TransformationNode::GetPosition,
             &TransformationNode::SetPosition);
        posGeom->name = "Geometry position";
        posGeom->properties[STEP] = Vector<3,float>(0.1f);
        values.push_back(posGeom);
        
        RWValueCall<TransformationNode, Quaternion<float> > *rotateGeom
            = new RWValueCall<TransformationNode, Quaternion<float> >
            (*geomTrans, &TransformationNode::GetRotation,
             &TransformationNode::SetRotation);
        rotateGeom->name = "Geometry rotation";
        values.push_back(rotateGeom);
        
        RWValueCall<TransformationNode, Vector<3, float> > *geomScale
            = new RWValueCall<TransformationNode, Vector<3, float> >
            (*geomTrans, &TransformationNode::GetScale,
             &TransformationNode::SetScale);
        geomScale->name = "Geometry scale";
        geomScale->properties[STEP] = Vector<3,float>(0.1f);
        values.push_back(geomScale);
        
        RWValueCall<PhotonRenderingView, bool> *dynamicGeom
            = new RWValueCall<PhotonRenderingView, bool>
            (*rv, &PhotonRenderingView::GetTreeUpdate,
             &PhotonRenderingView::SetTreeUpdate);
        dynamicGeom->name = "Dynamic geometry";
        values.push_back(dynamicGeom);

        EnumRWValueCall<TriangleMapWrapper, TriangleMap::SplitMethod> *splitScheme
            = new EnumRWValueCall<TriangleMapWrapper, TriangleMap::SplitMethod>
            (*triangleMap, &TriangleMapWrapper::GetSplitMethod,
             &TriangleMapWrapper::SetSplitMethod, "SplittingScheme");
        splitScheme->name = "Splitting Scheme";        
        splitScheme->AddEnum("Box", TriangleMap::BOX);
        splitScheme->AddEnum("Divide", TriangleMap::DIVIDE);
//splitScheme->AddEnum("Split", TriangleMap::SPLIT);
        values.push_back(splitScheme);

        RWValueCall<TriangleMapWrapper, bool> *emptySplit
            = new RWValueCall<TriangleMapWrapper, bool>
            (*triangleMap, &TriangleMapWrapper::IsSplittingEmptySpace,
             &TriangleMapWrapper::SplitEmptySpace);
        emptySplit->name = "Maximize empty space";
        values.push_back(emptySplit);

        RWValueCall<TriangleMapWrapper, bool> *propagateAabbs
            = new RWValueCall<TriangleMapWrapper, bool>
            (*triangleMap, &TriangleMapWrapper::GetPropagateBoundingBox,
             &TriangleMapWrapper::SetPropagateBoundingBox);
        propagateAabbs->name = "Propagate Aabbs";
        values.push_back(propagateAabbs);

        EnumRWValueCall<TriangleMapWrapper, TriangleMap::LowerAlgorithm> *lowerType
            = new EnumRWValueCall<TriangleMapWrapper, TriangleMap::LowerAlgorithm>
            (*triangleMap, &TriangleMapWrapper::GetLowerAlgorithm,
             &TriangleMapWrapper::SetLowerAlgorithm, "LowerAlgorithm");
        lowerType->name = "Lower Algorithm";
        lowerType->AddEnum("Bitmap", TriangleMap::BITMAP);
        lowerType->AddEnum("Balanced", TriangleMap::BALANCED);
        lowerType->AddEnum("SAH", TriangleMap::SAH);
        values.push_back(lowerType);

        RWValueCall<TriangleMapWrapper, float> *treeTiming 
            = new RWValueCall<TriangleMapWrapper, float>
            (*triangleMap, &TriangleMapWrapper::GetConstructionTime, &TriangleMapWrapper::SetConstructionTime);
        treeTiming->name = "Construction time";
        values.push_back(treeTiming);

        ActionValueCall<TriangleMapWrapper> *printTree
            = new ActionValueCall<TriangleMapWrapper>
            (*triangleMap, &TriangleMapWrapper::PrintTree);
        printTree->name = "Print kd-tree";
        values.push_back(printTree);

        atb->AddBar(new InspectionBar("Ray Tracing", values));
    }


    HostTraceListener::HostTraceListener(Renderers::OpenGL::PhotonRenderingView* rv, 
                                         Vector<2, int> screen)
        : rv(rv), screenSize(screen) {}

    void HostTraceListener::Handle(Devices::MouseButtonEventArg arg){
        if (arg.button == BUTTON_RIGHT && arg.type == EVENT_PRESS){
            rv->GetRayTracer()->HostTrace(arg.state.x, screenSize.Get(1) - arg.state.y, 
                                          rv->GetTriangleMap()->GetNodes());
        }
    }

    void AntToggler::Handle(Devices::KeyboardEventArg arg){
        if (arg.type == EVENT_PRESS && arg.sym == KEY_F5)
            atb->ToggleEnabled();
    }
}
