#include <algorithm>
#include <iostream>

#include "Line.hh"

GLuint Line::vao_id = 0;
GLuint Line::buffer_id = 0;
GLuint Line::instance_count = 0;

/**
 * Constructor
 */
Line::Line(Point position, Scale scale, Vector3 rotation, Colour colour, GLfloat thickness)
    : Movable(position), Scalable(scale), Rotatable(rotation), Coloured(colour)
{
    //Clamp thickness 0 <= x <= 1
    //this->thickness = std::clamp(thickness, 0., 1.);
    if (thickness > 1.)
        thickness = 1.;
    else if (thickness < 0.)
        thickness = 0.;

    this->thickness = thickness;
    this->instance_count++;
}

/**
 * Destructor.
 * Delete buffers.
 */
Line::~Line()
{
    this->instance_count--;
    if (this->instance_count == 0) {
        glDeleteBuffers(1, &this->buffer_id);
        glDeleteVertexArrays(1, &this->vao_id);
    }
}


/**
 * Initialise the buffer objects and vertex arrays.
 */
void Line::initialise_buffers()
{
    if (Line::vao_id != 0) {
        return;
    }

    //2 triangles
    GLuint buffer_size = 4*3;

    //Vertex & colour Data.
    GLfloat vertices[buffer_size];

    //Fill vertices
    vertices[0] = (this->thickness/2);      //x
    vertices[1] = 0.;                       //y
    vertices[2] = 0.;                       //z

    vertices[3] = - (this->thickness/2);    //x
    vertices[4] = 0.;                       //y
    vertices[5] = 0.;                       //z

    vertices[6] = vertices[0];              //x
    vertices[7] = 1.;                       //y
    vertices[8] = 0.;                       //z

    vertices[9]  = vertices[3];             //x
    vertices[10] = 1.;                      //y
    vertices[11] = 0.;                      //z

    //Generate vertex array
    glGenVertexArrays(1, &this->vao_id);
    glBindVertexArray(this->vao_id);

    //Upload vertex data
    glGenBuffers(1, &this->buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, this->buffer_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/**
 * Bind buffers and draw the line.
 *
 * @param model_matrix the current model_matrix to manipulate for sizing and positioning.
 */
void Line::draw(Matrix model_matrix)
{
    //Calculate the matrix transform
    //
    model_matrix = model_matrix * Matrix::identity().scale(this->scale).rotate(this->rotation).translate(this->position);

    //Upload the matrix to the shader
    this->shader->set_model_matrix(model_matrix);

    //Set the colour
    this->shader->set_uniform("colour", this->colour);

    //Bind
    glBindBuffer(GL_ARRAY_BUFFER, this->buffer_id);
    glBindVertexArray(this->vao_id);

    //Enable and set vertex attributes
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, (const void*)0);

    //Set the shader
    this->shader->use();

    //Perform the draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //Disable attribute pointers
    glDisableVertexAttribArray(0);

    //Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}