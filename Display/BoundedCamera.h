// Bounded Camera

#ifndef _BOUNDED_CAMERA_H_
#define _BOUNDED_CAMERA_H_

#include <Display/Camera.h>

namespace OpenEngine {
namespace Display {
    class BoundedCamera : public Camera {
    private:
        Vector<3, float> minBound, maxBound;
        
    public:
        BoundedCamera(IViewingVolume& volume);

        void SetPosition(const Vector<3,float> position);

        void Move(const Vector<3,float> position);
        void Move(const float l, const float t, const float v);

        void SetMinimumBound(const Vector<3,float> bound);
        void SetMaximumBound(const Vector<3,float> bound);

        Vector<3, float> ApplyBounds(Vector<3, float> pos);
    };
    
}
}

#endif
