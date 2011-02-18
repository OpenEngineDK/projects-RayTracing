#include "BoundedCamera.h"
#include <Logging/Logger.h>

namespace OpenEngine {
namespace Display {

    BoundedCamera::BoundedCamera(IViewingVolume& volume)
        : Camera(volume), minBound(Vector<3, float>(-exp2f(80.0f))), 
        maxBound(Vector<3, float>(exp2f(80.0f))) {}

    void BoundedCamera::SetPosition(const Vector<3,float> position){
        volume.SetPosition(ApplyBounds(position));
    }

    void BoundedCamera::Move(const Vector<3,float> position){
        Camera::Move(position);
        volume.SetPosition(ApplyBounds(volume.GetPosition()));
    }

    void BoundedCamera::Move(const float l, const float t, const float v){
        Camera::Move(l, t, v);
        volume.SetPosition(ApplyBounds(volume.GetPosition()));
    }

    void BoundedCamera::SetMinimumBound(const Vector<3,float> bound){
        minBound = bound;
    }

    void BoundedCamera::SetMaximumBound(const Vector<3,float> bound){
        maxBound = bound;
    }

    Vector<3, float> BoundedCamera::ApplyBounds(Vector<3, float> pos){
        for (int i = 0; i < 3; ++i){
            if (pos.Get(i) < minBound.Get(i)) pos[i] = minBound.Get(i);
            if (pos.Get(i) > maxBound.Get(i)) pos[i] = maxBound.Get(i);
        }
        return pos;
    }
    
}
}
