#pragma once

// TODO: header
class SsboBase
{
public:
    SsboBase();
    virtual ~SsboBase();

    // derived class needs customized Init(...) function to initialize member values

    unsigned int VaoId() const;
    unsigned int BufferId() const;
    unsigned int DrawStyle() const;
    unsigned int NumItems() const;

protected:
    // can't be private because the derived classes need to set them
    
    // save on the large header inclusion of OpenGL and write out these primitive types instead 
    // of using the OpenGL typedefs
    // Note: IDs are GLuint (unsigned int), draw style is GLenum (unsigned int), GLushort is 
    // unsigned short.
    
    // vertex array buffer
    unsigned int _vaoId;

    // shader storage buffer object
    unsigned int _bufferId;

    // GL_TRIANGLES, GL_LINES, etc.
    unsigned int _drawStyle;

    // number of points, lines, triangles, etc.
    // Note: Is a complement draw style.
    unsigned int _numItems;

    //??binding point index? http://www.geeks3d.com/20140704/tutorial-introduction-to-opengl-4-3-shader-storage-buffers-objects-ssbo-demo/ ??
};
