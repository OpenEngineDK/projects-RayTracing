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
        void SetLeafSkipping(bool s) { rv->GetRayTracer()->SetLeafSkipping(s); }
        bool GetLeafSkipping() { return rv->GetRayTracer()->GetLeafSkipping(); }
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

        RWValueCall<RVRayTracer, bool> *visual 
            = new RWValueCall<RVRayTracer, bool>
            (*ray, &RVRayTracer::GetVisualizeRays,
             &RVRayTracer::SetVisualizeRays);
        visual->name = "Visualize rays";
        values.push_back(visual);

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

        atb->AddBar(new InspectionBar("Ray Tracing", values));
    }
}
