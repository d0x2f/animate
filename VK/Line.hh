#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "../Geometry/Definitions.hh"
#include "../Geometry/Matrix.hh"
#include "../Object/Property/Drawable.hh"
#include "../Object/Property/Movable.hh"
#include "../Object/Property/Scalable.hh"
#include "../Object/Property/Rotatable.hh"
#include "../Object/Property/Coloured.hh"

using namespace Animate::Geometry;
using namespace Animate::Object::Property;

namespace Animate::VK
{
    class Line : public Drawable, public Movable, public Scalable, public Rotatable, public Coloured
    {
        public:
            Line(std::weak_ptr<VK::Context> context, Point position, Scale scale, Vector3 rotation, Colour colour, GLfloat thickness);
            ~Line();

            void initialise_buffers() override;
            std::vector< std::weak_ptr<Buffer> > const get_buffers() override;

            void set_model_matrix(Matrix model_matrix) override;

        protected:
            std::weak_ptr<Buffer> vertex_buffer,
                                  index_buffer;
            GLfloat thickness;

            void create_vertex_buffer();
            void create_index_buffer();
    };
}
