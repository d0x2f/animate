#pragma once

#include "../Animation.hh"
#include "../../VK/Quad.hh"
#include "../../VK/Shader.hh"
#include "../../Geometry/Definitions.hh"
#include "../../Object/Object.hh"

using namespace Animate::Object;
using namespace Animate::Geometry;

namespace Animate::Animation::Modulo
{
    class Modulo : public Animation
    {
        public:
            Modulo(Context *context);

            bool on_render();
            void on_tick(GLuint64 time_delta);
            void initialise();

        protected:
            std::shared_ptr<VK::Shader> shader;
    };
}
