// the OpenGL version include also includes all previous versions
// Build note: Due to a minefield of preprocessor build flags, the gl_load.hpp must come after 
// the version include.
// Build note: Do NOT mistakenly include _int_gl_4_4.h.  That one doesn't define OpenGL stuff 
// first.
// Build note: Also need to link opengl32.lib (unknown directory, but VS seems to know where it 
// is, so don't put in an "Additional Library Directories" entry for it).
// Build note: Also need to link glload/lib/glloadD.lib.
#include "glload/include/glload/gl_4_4.h"
#include "glload/include/glload/gl_load.hpp"

// Build note: Must be included after OpenGL code (in this case, glload).
// Build note: Also need to link freeglut/lib/freeglutD.lib.  However, the linker will try to 
// find "freeglut.lib" (note the lack of "D") instead unless the following preprocessor 
// directives are set either here or in the source-building command line (VS has a
// "Preprocessor" section under "C/C++" for preprocessor definitions).
// Build note: Also need to link winmm.lib (VS seems to know where it is, so don't put in an 
// "Additional Library Directories" entry).
#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#include "freeglut/include/GL/freeglut.h"

// this linking approach is very useful for portable, crude, barebones demo code, but it is 
// better to link through the project building properties
#pragma comment(lib, "glload/lib/glloadD.lib")
#pragma comment(lib, "opengl32.lib")            // needed for glload::LoadFunctions()
#pragma comment(lib, "freeglut/lib/freeglutD.lib")
#ifdef WIN32
#pragma comment(lib, "winmm.lib")               // Windows-specific; freeglut needs it
#endif

// apparently the FreeType lib also needs a companion file, "freetype261d.pdb"
#pragma comment (lib, "freetype-2.6.1/objs/vc2010/Win32/freetype261d.lib")

// for printf(...)
#include <stdio.h>

// for basic OpenGL stuff
#include "OpenGlErrorHandling.h"
//#include "GenerateShader.h"
#include "ShaderStorage.h"

// for drawing shapes
//#include "GeometryData.h"
//#include "PrimitiveGeneration.h"

// for particles and where they live
#include "glm/vec2.hpp"
//#include "ParticleRegionCircle.h"
//#include "ParticleRegionPolygon.h"
//#include "ParticleEmitterPoint.h"
//#include "ParticleEmitterBar.h"
//#include "ParticleStorage.h"
//#include "ParticleUpdater.h"
#include "ParticleSsbo.h"
#include "PolygonFaceSsbo.h"
#include "ParticlePolygonComputeUpdater.h"

// for moving the shapes around in window space
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// for the frame rate counter
#include "FreeTypeEncapsulated.h"
#include "Stopwatch.h"

Stopwatch gTimer;

FreeTypeEncapsulated gTextAtlases;

// in a bigger program, uniform locations would probably be stored in the same place as the 
// shader programs
GLint gUnifMatrixTransformLoc;

//// in a bigger program, geometry data would be stored in some kind of "scene" or in a renderer
//// or behind door number 3 so that collision boxes could get at the vertex data
//GeometryData gCircleGeometry;
//GeometryData gPolygonGeometry;

// ??stored in scene??
ParticleSsbo gParticleBuffer;
PolygonFaceSsbo gPolygonFaceBuffer;

// in a bigger program, this would somehow be encapsulated and associated with both the circle
// geometry and the circle particle region, and ditto for the polygon
glm::mat4 gRegionTransformMatrix;

//// in a bigger program, ??where would particle stuff be stored??
//IParticleRegion *gpParticleRegionCircle;
//IParticleRegion *gpParticleRegionPolygon;
IParticleEmitter *gpParticleEmitterPoint;
//IParticleEmitter *gpParticleEmitterBar;
//ParticleUpdater gParticleUpdater;
ParticlePolygonComputeUpdater gParticleComputeUpdater;

// divide between the circle and the polygon regions
// Note: 
// - 10,000 particles => ~60 fps on my computer
// - 15,000 particles => 30-40 fps on my computer
const unsigned int MAX_PARTICLE_COUNT = 15000;
//ParticleStorage gParticleStorage;



/*-----------------------------------------------------------------------------------------------
Description:
    Encapsulates the rotating of a 2D vector by -90 degrees (+90 degrees not used in this demo).
Parameters:
    v   A const 2D vector.
Returns:
    A 2D vector rotated -90 degrees from the provided one.
Exception:  Safe
Creator:    John Cox (7-2-2016)
-----------------------------------------------------------------------------------------------*/
static glm::vec2 RotateNeg90(const glm::vec2 &v)
{
    return glm::vec2(v.y, -(v.x));
}

// TODO: header
// Manually (it's a demo, so eh?) generates a polygon centered around the window space origin (0,0) with corners at:
// - vec2(-0.25f, -0.5f);
// - vec2(+0.25f, -0.5f);
// - vec2(+0.5f, +0.25f);
// - vec2(-0.5f, +0.25f);
static void GeneratePolygonRegion(std::vector<PolygonFace> *polygonFaceCollection)
{
    glm::vec2 p1(-0.25f, -0.5f);
    glm::vec2 p2(+0.25f, -0.5f);
    glm::vec2 p3(+0.5f, +0.25f);
    glm::vec2 p4(-0.5f, +0.25f);
    PolygonFace face1(p1, p2, RotateNeg90(p2 - p1));
    PolygonFace face2(p2, p3, RotateNeg90(p3 - p2));
    PolygonFace face3(p3, p4, RotateNeg90(p4 - p3));
    PolygonFace face4(p4, p1, RotateNeg90(p1 - p4));

    polygonFaceCollection->push_back(face1);
    polygonFaceCollection->push_back(face2);
    polygonFaceCollection->push_back(face3);
    polygonFaceCollection->push_back(face4);
}


/*-----------------------------------------------------------------------------------------------
Description:
    Governs window creation, the initial OpenGL configuration (face culling, depth mask, even
    though this is a 2D demo and that stuff won't be of concern), the creation of geometry, and
    the creation of a texture.
Parameters:
    argc    (From main(...)) The number of char * items in argv.  For glut's initialization.
    argv    (From main(...)) A collection of argument strings.  For glut's initialization.
Returns:
    False if something went wrong during initialization, otherwise true;
Exception:  Safe
Creator:    John Cox (3-7-2016)
-----------------------------------------------------------------------------------------------*/
void Init()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);


    ShaderStorage &shaderStorageRef = ShaderStorage::GetInstance();

    // FreeType initialization
    shaderStorageRef.NewShader("freetype");
    shaderStorageRef.AddShaderFile("freetype", "freeType.vert", GL_VERTEX_SHADER);
    shaderStorageRef.AddShaderFile("freetype", "freeType.frag", GL_FRAGMENT_SHADER);
    shaderStorageRef.LinkShader("freetype");
    GLuint freeTypeProgramId = shaderStorageRef.GetShaderProgram("freetype");
    gTextAtlases.Init("FreeSans.ttf", freeTypeProgramId);

    
    
    // for the particle compute shader stuff
    std::string computeShaderKey = "compute particles";
    shaderStorageRef.NewShader(computeShaderKey);
    //shaderStorageRef.AddComputeShaderFile("particleCompute", "particleStructure.comp");
    //shaderStorageRef.AddComputeShaderFile("particleCompute", "particleMain.comp");
    shaderStorageRef.AddShaderFile(computeShaderKey, "particlePolygonRegion.comp", GL_COMPUTE_SHADER);
    //shaderStorageRef.CompileComputeShader("particleCompute");
    //shaderStorageRef.LinkComputeShader("particleCompute");
    shaderStorageRef.LinkShader(computeShaderKey);
    //GLuint computeProgramId = shaderStorageRef.GetShaderProgram(computeShaderKey);
    //InitComputeShader(computeProgramId);
    gParticleComputeUpdater.Init(computeShaderKey);


    
    //// generate a circular particle region, point and bar particle emitters, and join them them in a particle updater
    
    // generate a point emitter and a polygon region


    std::string renderParticlesShaderKey = "render particles";
    shaderStorageRef.NewShader(renderParticlesShaderKey);
    shaderStorageRef.AddShaderFile(renderParticlesShaderKey, "particleRender.vert", GL_VERTEX_SHADER);
    shaderStorageRef.AddShaderFile(renderParticlesShaderKey, "particleRender.frag", GL_FRAGMENT_SHADER);
    shaderStorageRef.LinkShader(renderParticlesShaderKey);

    gParticleBuffer.Init(MAX_PARTICLE_COUNT, shaderStorageRef.GetShaderProgram(renderParticlesShaderKey));

    //// the circle starts centered on the origin and the translate matrix will move it
    //// Note: The 1.0f makes it translatable.
    //gRegionTransformMatrix = glm::translate(glm::mat4(), glm::vec3(+0.3f, +0.3f, 0.0f));
    //gRegionTransformMatrix *= glm::rotate(glm::mat4(), 10.0f, glm::vec3(0.0f, 0.0f, 1.0f));

    //// circular particle region
    //float circleRadius = 0.5f;
    //glm::vec2 circleCenter = glm::vec2(0.0f, 0.0f);
    //gpParticleRegionCircle = new ParticleRegionCircle(circleCenter, circleRadius);
    //gpParticleRegionCircle->SetTransform(gRegionTransformMatrix);

    // polygon particle region
    std::vector<PolygonFace> polygonFaces;
    GeneratePolygonRegion(&polygonFaces);
    gPolygonFaceBuffer.Init(polygonFaces, shaderStorageRef.GetShaderProgram(renderParticlesShaderKey));
/*
    polygonCorners.push_back(glm::vec2(-0.25f, -0.5f));
    polygonCorners.push_back(glm::vec2(+0.25f, -0.5f));
    polygonCorners.push_back(glm::vec2(+0.5f, +0.25f));
    polygonCorners.push_back(glm::vec2(-0.5f, +0.25f));
    gPolygonFaceBuffer.Init()
    gpParticleRegionPolygon = new ParticleRegionPolygon(polygonCorners);
    gpParticleRegionPolygon->SetTransform(gRegionTransformMatrix);*/


    // stick the point emitter in the geometric center of the polygon region
    // Note: Repeat points do exist, but the average will sort that out.
    float sumFaceX = 0.0f;
    float sumFaceY = 0.0f;
    for (size_t faceIndex = 0; faceIndex < polygonFaces.size(); faceIndex++)
    {
        const PolygonFace &pf = polygonFaces[faceIndex];
        sumFaceX += pf._start.x + pf._end.x;
        sumFaceY += pf._start.y + pf._end.y;
    }
    glm::vec2 polygonRegionCenter(sumFaceX / (polygonFaces.size() * 2), 
        sumFaceY / (polygonFaces.size() / 2));
    gpParticleEmitterPoint = new ParticleEmitterPoint(polygonRegionCenter, 0.3f, 0.5f);
    gParticleComputeUpdater.AddEmitter(gpParticleEmitterPoint, 500);
    //gpParticleEmitterPoint->SetTransform(gRegionTransformMatrix);

    //// stick the emitter bar on the left side of the circle, have it emit right, and make the 
    //// particles slow compared to the point emitter 
    //// Note: Have to use vec4s instead of vec2s because glm::translate(...) only spits out mat4.
    //glm::vec2 barP1 = glm::vec2(-0.2f, +0.1f);
    //glm::vec2 barP2 = glm::vec2(-0.2f, -0.1f);
    //glm::vec2 emitDirection(+1.0f, 0.0f);
    //float minVel = 0.1f;
    //float maxVel = 0.3f;
    //gpParticleEmitterBar = new ParticleEmitterBar(barP1, barP2, emitDirection, minVel, maxVel);
    //gpParticleEmitterBar->SetTransform(gRegionTransformMatrix);

    //// stick the particle region and emitters into a single "updater" object
    //gParticleStorage.Init(particleProgramId, MAX_PARTICLE_COUNT);
    ////gParticleUpdater.SetRegion(gpParticleRegionCircle);
    //gParticleUpdater.SetRegion(gpParticleRegionPolygon);
    //gParticleUpdater.AddEmitter(gpParticleEmitterBar, 10);
    //gParticleUpdater.AddEmitter(gpParticleEmitterPoint, 10);
    //gParticleUpdater.ResetAllParticles(gParticleStorage._allParticles);
    
    // geometry for particle region borders
    std::string renderGeometryShaderKey = "render geometry";
    shaderStorageRef.NewShader(renderGeometryShaderKey);
    shaderStorageRef.AddShaderFile(renderGeometryShaderKey, "geometry.vert", GL_VERTEX_SHADER);
    shaderStorageRef.AddShaderFile(renderGeometryShaderKey, "geometry.frag", GL_FRAGMENT_SHADER);
    shaderStorageRef.LinkShader(renderGeometryShaderKey);
    //GLuint geometryProgramId = shaderStorageRef.GetShaderProgram(renderGeometryShaderKey);

    gUnifMatrixTransformLoc = shaderStorageRef.GetUniformLocation(renderGeometryShaderKey, "translateMatrixWindowSpace");
    
    //GenerateCircle(&gCircleGeometry, circleRadius, true);
    //gCircleGeometry.Init(geometryProgramId);

    //GeneratePolygonWireframe(&gPolygonGeometry, polygonCorners, false);
    //gPolygonGeometry.Init(geometryProgramId);


    // the timer will be used for framerate calculations
    gTimer.Init();
    gTimer.Start();
}

/*-----------------------------------------------------------------------------------------------
Description:
    This is the rendering function.  It tells OpenGL to clear out some color and depth buffers,
    to set up the data to draw, to draw than stuff, and to report any errors that it came across.
    This is not a user-called function.

    This function is registered with glutDisplayFunc(...) during glut's initialization.
Parameters: None
Returns:    None
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void Display()
{

    // TODO: in rendering shader, if particle is not active (if necessary, turn on the vertex attrib pointers that account for it), then make it black so that it won't show up


    //// TODO: apply these in the compute and render uniforms
    //// the circle starts centered on the origin and the translate matrix will move it
    //// Note: The 1.0f makes it translatable.
    //gRegionTransformMatrix = glm::translate(glm::mat4(), glm::vec3(+0.3f, +0.3f, 0.0f));
    //gRegionTransformMatrix *= glm::rotate(glm::mat4(), 10.0f, glm::vec3(0.0f, 0.0f, 1.0f));



    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update all particle locations

    // TODO: ??in the update method, bind both buffer bases to different binding points??





    ////??why isn't it drawing anything??
    //gParticleComputeUpdater.Update(MAX_PARTICLE_COUNT, 0.01f);



    ////gParticleUpdater.Update(gParticleStorage._allParticles, 0, 
    ////    gParticleStorage._allParticles.size(), 0.01f);

    // draw the particles
    glUseProgram(ShaderStorage::GetInstance().GetShaderProgram("render particles"));
    glBindVertexArray(gParticleBuffer.VaoId());
    //glBindBuffer(GL_ARRAY_BUFFER, gParticleStorage._arrayBufferId);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, gParticleStorage._sizeBytes, gParticleStorage._allParticles.data());
    //glDrawArrays(gParticleStorage._drawStyle, 0, gParticleStorage._allParticles.size());
    glDrawArrays(gParticleBuffer.DrawStyle(), 0, gParticleBuffer.NumVertices());






    // draw the particle region borders
    glUseProgram(ShaderStorage::GetInstance().GetShaderProgram("render geometry"));
    //gRegionTransformMatrix = glm::translate(glm::mat4(), glm::vec3(+0.3f, +0.3f, 0.0f));
    //glUniformMatrix4fv(gUnifMatrixTransformLoc, 1, GL_FALSE, glm::value_ptr(gRegionTransformMatrix));
    glBindVertexArray(gPolygonFaceBuffer.VaoId());
    //glBindVertexArray(gCircleGeometry._vaoId);
    //glDrawElements(gCircleGeometry._drawStyle, gCircleGeometry._indices.size(), GL_UNSIGNED_SHORT, 0);
    //glBindVertexArray(gPolygonGeometry._vaoId);
    //glDrawElements(gPolygonGeometry._drawStyle, gPolygonGeometry._indices.size(), GL_UNSIGNED_SHORT, 0);
    glDrawArrays(gPolygonFaceBuffer.DrawStyle(), 0, gPolygonFaceBuffer.NumVertices());


    // draw the frame rate once per second in the lower left corner
    // Note: The font textures' orgin is their lower left corner, so the "lower left" in screen 
    // space is just above [-1.0f, -1.0f].
    GLfloat color[4] = { 0.5f, 0.5f, 0.0f, 1.0f };
    char str[8];
    static int elapsedFramesPerSecond = 0;
    static double elapsedTime = 0.0;
    static double frameRate = 0.0;
    elapsedFramesPerSecond++;
    elapsedTime += gTimer.Lap();
    if (elapsedTime > 1.0)
    {
        frameRate = (double)elapsedFramesPerSecond / elapsedTime;
        elapsedFramesPerSecond = 0;
        elapsedTime -= 1.0f;
    }
    sprintf(str, "%.2lf", frameRate);
    float xy[2] = { -0.99f, -0.99f };
    float scaleXY[2] = { 1.0f, 1.0f };

    // the first time that "get shader program" runs, it will load the atlas
    glUseProgram(ShaderStorage::GetInstance().GetShaderProgram("freetype"));
    gTextAtlases.GetAtlas(48)->RenderText(str, xy, scaleXY, color);

    // clean up bindings
    glUseProgram(0);
    glBindVertexArray(0);       // unbind this BEFORE the buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // tell the GPU to swap out the displayed buffer with the one that was just rendered
    glutSwapBuffers();

    // tell glut to call this display() function again on the next iteration of the main loop
    // Note: https://www.opengl.org/discussion_boards/showthread.php/168717-I-dont-understand-what-glutPostRedisplay()-does
    // Also Note: This display() function will also be registered to run if the window is moved
    // or if the viewport is resized.  If glutPostRedisplay() is not called, then as long as the
    // window stays put and doesn't resize, display() won't be called again (tested with 
    // debugging).
    // Also Also Note: It doesn't matter where this is called in this function.  It sets a flag
    // for glut's main loop and doesn't actually call the registered display function, but I 
    // got into the habbit of calling it at the end.
    glutPostRedisplay();
}

/*-----------------------------------------------------------------------------------------------
Description:
    Tell's OpenGL to resize the viewport based on the arguments provided.  This is an 
    opportunity to call glViewport or glScissor to keep up with the change in size.
    
    This is not a user-called function.  It is registered with glutReshapeFunc(...) during 
    glut's initialization.
Parameters:
    w   The width of the window in pixels.
    h   The height of the window in pixels.
Returns:    None
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void Reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

/*-----------------------------------------------------------------------------------------------
Description:
    Executes when the user presses a key on the keyboard.

    This is not a user-called function.  It is registered with glutKeyboardFunc(...) during
    glut's initialization.

    Note: Although the x and y arguments are for the mouse's current position, this function does
    not respond to mouse presses.
Parameters:
    key     The ASCII code of the key that was pressed (ex: ESC key is 27)
    x       The horizontal viewport coordinates of the mouse's current position.
    y       The vertical window coordinates of the mouse's current position
Returns:    None
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void Keyboard(unsigned char key, int x, int y)
{
    // this statement is mostly to get ride of an "unreferenced parameter" warning
    printf("keyboard: x = %d, y = %d\n", x, y);
    switch (key)
    {
    case 27:
    {
        // ESC key
        glutLeaveMainLoop();
        return;
    }
    default:
        break;
    }
}

/*-----------------------------------------------------------------------------------------------
Description:
    I don't know what this does, but I've kept it around since early times, and this was the 
    comment given with it:
    
    "Called before FreeGLUT is initialized. It should return the FreeGLUT display mode flags 
    that you want to use. The initial value are the standard ones used by the framework. You can 
    modify it or just return you own set.  This function can also set the width/height of the 
    window. The initial value of these variables is the default, but you can change it."
Parameters:
    displayMode     ??
    width           ??
    height          ??
Returns:
    ??what??
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
unsigned int Defaults(unsigned int displayMode, int &width, int &height) 
{
    // this statement is mostly to get ride of an "unreferenced parameter" warning
    printf("Defaults: width = %d, height = %d\n", width, height);
    return displayMode; 
}

/*-----------------------------------------------------------------------------------------------
Description:
    Cleans up GPU memory.  This might happen when the processes die, but be a good memory steward
    and clean up properly.

    Note: A big program would have the textures, program IDs, buffers, and other things 
    encapsulated somewhere, and each other those would do the cleanup, but in this barebones 
    demo, I can just clean up everything here.
Parameters: None
Returns:    None
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
void CleanupAll()
{
    //// these deletion functions need the buffer ID, but they take a (void *) for the second 
    //// argument in the event that the user has an array of IDs (legacy OpenGL stuff that isn't 
    //// used much anymore, if at all), so pass in the buffer ID's address and tell it to delete 1
    //// Note: The second argument is treated like an array, so if I were to pass in the address 
    //// of a single GLuint and tell it to delete 2, then it will either (1) blow up or 
    //// (2) silently delete something I didn't want.  Both are bad, so treat it nice.
    //// Also Note: If I attempt to delete an ID that has already been deleted, that is ok.  OpenGL
    //// will silently swallow that.
    //glDeleteBuffers(1, &gCircleGeometry._arrayBufferId);
    //glDeleteBuffers(1, &gCircleGeometry._elementBufferId);
    //glDeleteVertexArrays(1, &gCircleGeometry._vaoId);
    //glDeleteBuffers(1, &gPolygonGeometry._arrayBufferId);
    //glDeleteBuffers(1, &gPolygonGeometry._elementBufferId);
    //glDeleteVertexArrays(1, &gPolygonGeometry._vaoId);

    //delete(gpParticleRegionCircle);
    //delete(gpParticleRegionPolygon);





    //delete(gpParticleEmitterBar);
    delete(gpParticleEmitterPoint);
}

/*-----------------------------------------------------------------------------------------------
Description:
    Program start and end.
Parameters:
    argc    The number of strings in argv.
    argv    A pointer to an array of null-terminated, C-style strings.
Returns:
    0 if program ended well, which it always does or it crashes outright, so returning 0 is fine
Exception:  Safe
Creator:    John Cox (2-13-2016)
-----------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    glutInit(&argc, argv);

    int width = 500;
    int height = 500;
    unsigned int displayMode = GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH | GLUT_STENCIL;
    displayMode = Defaults(displayMode, width, height);

    glutInitDisplayMode(displayMode);
    glutInitContextVersion(4, 4);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    // enable this for automatic message reporting (see OpenGlErrorHandling.cpp)
#define DEBUG
#ifdef DEBUG
    glutInitContextFlags(GLUT_DEBUG);
#endif

    glutInitWindowSize(width, height);
    glutInitWindowPosition(300, 200);
    int window = glutCreateWindow(argv[0]);

    glload::LoadTest glLoadGood = glload::LoadFunctions();
    // ??check return value??

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

    if (!glload::IsVersionGEQ(3, 3))
    {
        printf("Your OpenGL version is %i, %i. You must have at least OpenGL 3.3 to run this tutorial.\n",
            glload::GetMajorVersion(), glload::GetMinorVersion());
        glutDestroyWindow(window);
        return 0;
    }

    if (glext_ARB_debug_output)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB(DebugFunc, (void*)15);
    }

    Init();

    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutMainLoop();

    CleanupAll();

    return 0;
}