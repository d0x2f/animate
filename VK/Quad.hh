#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "../Geometry/Definitions.hh"
#include "../Geometry/Matrix.hh"
#include "Shader.hh"
#include "../Object/Property/Drawable.hh"
#include "../Object/Property/Movable.hh"
#include "../Object/Property/Scalable.hh"

using namespace Animate::Geometry;
using namespace Animate::Object::Property;

namespace Animate::VK
{
    class Quad : public Drawable, public Movable, public Scalable
    {
        public:
            Quad(std::weak_ptr<VK::Context> context, Point position = Point(), Scale size = Scale(1.,1.,1.));
            ~Quad();

            void set_texture_position(Position texture_position, Position texture_size = Position(1., 1.));
            std::weak_ptr<Buffer> const get_buffer() override;

            void draw(Matrix model_matrix) override;

            std::shared_ptr<Buffer> buffer;

        protected:
            Position texture_position;
            Position texture_size = Position(1., 1.);

            void initialise_buffers() override;
    };
}
