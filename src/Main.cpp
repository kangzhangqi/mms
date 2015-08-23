#include <thread>

#include <glut.h>
#include <fontstash.h> // TODO: MACK

#include "mouse/IMouseAlgorithm.h"
#include "mouse/MouseAlgorithms.h"
#include "sim/GraphicUtilities.h"
#include "sim/InterfaceType.h"
#include "sim/Key.h"
#include "sim/Logging.h"
#include "sim/Maze.h"
#include "sim/MazeGraphic.h"
#include "sim/Mouse.h"
#include "sim/MouseGraphic.h"
#include "sim/MouseInterface.h"
#include "sim/Param.h"
#include "sim/State.h"
#include "sim/SimUtilities.h"
#include "sim/TriangleGraphic.h"
#include "sim/units/Seconds.h"
#include "sim/World.h"

// Function declarations
void draw();
void keyPress(unsigned char key, int x, int y);
void specialKeyPress(int key, int x, int y);
void specialKeyRelease(int key, int x, int y);

// Initialization functions, declared in the order they should be called
void initRunAndLogging();
void initSimObjects();
void initGraphics(int argc, char* argv[]);
void initMouseAlgo();

// Global variable declarations
sim::Mouse* g_mouse;
sim::MazeGraphic* g_mazeGraphic;
sim::MouseGraphic* g_mouseGraphic;
sim::MouseInterface* g_mouseInterface;
sim::World* g_world;
IMouseAlgorithm* g_algorithm;

// The ID of the transformation matrix, which takes triangle graphic objects in
// the physical coordinate system and transforms them into the OpenGL system.
GLuint g_transformationMatixId;

// TODO: MACK
struct sth_stash* stash; // TODO
int droid; // TODO
GLuint vertex_buffer_object;
GLuint program;

int main(int argc, char* argv[]) {

    // Step 0: Initialze the current run and logging function
    initRunAndLogging();

    // Step 1: Initialize the simulation objects
    initSimObjects();

    // Step 2: Initialize the graphics
    initGraphics(argc, argv);

    // Step 3: Initialize the mouse algorithm
    initMouseAlgo();

    // Step 4: Start the physics loop
    std::thread physicsThread([](){
        g_world->simulate();
    });

    // Step 5: Start the solving loop
    std::thread solvingThread([](){

        // Wait for the window to appear
        sim::SimUtilities::sleep(sim::Seconds(sim::P()->glutInitDuration()));

        // Unfog the beginning tile if necessary
        if (sim::S()->interfaceType() == sim::InterfaceType::DISCRETE && sim::P()->discreteInterfaceUnfogTileOnEntry()) {
            g_mazeGraphic->setTileFogginess(0, 0, false);
        }

        // Finally, begin execution of the mouse algorithm
        g_algorithm->solve(
            g_mazeGraphic->getWidth(),
            g_mazeGraphic->getHeight(),
            sim::DIRECTION_TO_CHAR.at(sim::STRING_TO_DIRECTION.at(sim::P()->mouseStartingDirection())),
            g_mouseInterface);
    });

    // Step 6: Start the graphics loop
    sim::S()->enterMainLoop();
    glutMainLoop();
}

void draw() {

    // In order to ensure we're sleeping the correct amount of time, we time
    // the drawing operation and take it into account when we sleep.
    double start(sim::SimUtilities::getHighResTime());

    // Determine the starting index of the mouse
    static const int mouseTrianglesStartingIndex = sim::GraphicUtilities::TGB.size();

    // Make space for mouse updates and copy to the CPU buffer
    sim::GraphicUtilities::TGB.erase(sim::GraphicUtilities::TGB.begin() + mouseTrianglesStartingIndex, sim::GraphicUtilities::TGB.end());

    // Fill the CPU buffer with new mouse triangles
    g_mouseGraphic->draw();

    // Clear the vertex buffer object and copy over the CPU buffer
    glBufferData(GL_ARRAY_BUFFER, sim::GraphicUtilities::TGB.size() * sizeof(sim::TriangleGraphic), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sim::GraphicUtilities::TGB.size() * sizeof(sim::TriangleGraphic),
        &sim::GraphicUtilities::TGB.front());

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Enable scissoring so that the maps are only draw in specified locations
    glEnable(GL_SCISSOR_TEST);

    // Render the full map
    std::pair<int, int> fullMapPosition = sim::GraphicUtilities::getFullMapPosition();
    std::pair<int, int> fullMapSize = sim::GraphicUtilities::getFullMapSize();
    glScissor(fullMapPosition.first, fullMapPosition.second, fullMapSize.first, fullMapSize.second);
    glUniformMatrix4fv(g_transformationMatixId, 1, GL_TRUE, &sim::GraphicUtilities::getFullMapTransformationMatrix().front());
    glDrawArrays(GL_TRIANGLES, 0, 3 * sim::GraphicUtilities::TGB.size());

    // Render the zoomed map
    std::pair<int, int> zoomedMapPosition = sim::GraphicUtilities::getZoomedMapPosition();
    std::pair<int, int> zoomedMapSize = sim::GraphicUtilities::getZoomedMapSize();
    glScissor(zoomedMapPosition.first, zoomedMapPosition.second, zoomedMapSize.first, zoomedMapSize.second);
    glUniformMatrix4fv(g_transformationMatixId, 1, GL_TRUE, &sim::GraphicUtilities::getZoomedMapTransformationMatrix(
        g_mouse->getInitialTranslation(), g_mouse->getCurrentTranslation(), g_mouse->getCurrentRotation()).front());
    glDrawArrays(GL_TRIANGLES, 0, 3 * sim::GraphicUtilities::TGB.size());

    // We disable scissoring so that the glClear can take effect
    glDisable(GL_SCISSOR_TEST);

    // ------------- TODO: MACK: We should be able to disable scissoring after we draw all of the text...
#if(0)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

    glColor4ub(255,255,255,255); // TODO: Sets color to white
    glLoadIdentity();
    glOrtho(0, sim::P()->windowWidth(), 0, sim::P()->windowHeight(), -1, 1);
    glOrtho(0, 930, 0, 470, -1, 1);

    /* TODO: Rotation perhaps?
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 640, 480, 0, 1, -1);
    glMatrixMode(GL_MODELVIEW);
    */
    /*
    std::vector<float> mat = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1,
    };
    glLoadMatrixf(&mat.front());
    */
    /*
    glLoadMatrixf(&sim::GraphicUtilities::getZoomedMapTransformationMatrix(
        g_mouse->getInitialTranslation(), g_mouse->getCurrentTranslation(), g_mouse->getCurrentRotation()).front());
    */

    // TODO: Should be fine... I just have to figure out a mechanism for drawing it.
    glRotated(45.0, 0.0, 0.0, 1.0);
    sth_begin_draw(stash);
    for (int i = 0; i < 256; i += 1) {
        for (int j = 0; j < 2; j += 1) {
            for (int k = 0; k < 4; k += 1) {
                sth_draw_text(stash, droid, 12.0f, 14 + (i / 16) * 28 + 5 * k, 16 + (i % 16) * 28 + 9 * j, "X", NULL);
            }
        }
    }
    sth_end_draw(stash);
    glRotated(-45.0, 0.0, 0.0, 1.0);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glUseProgram(program);
    // TODO: Draw the mouse on top of the 
    //glDrawArrays(GL_TRIANGLES, 3 * mouseTrianglesStartingIndex, 3 * sim::GraphicUtilities::TGB.size());
    // ------------- TODO
#endif

    // Display the result
    glutSwapBuffers();

    // Get the duration of the drawing operation, in seconds. Note that this duration
    // is simply the total number of real seconds that have passed, which is exactly
    // what we want (since the frame-rate is perceived in real-time and not CPU time).
    double end(sim::SimUtilities::getHighResTime());
    double duration = end - start;

    // Notify the user of a late frame
    if (duration > 1.0/sim::P()->frameRate()) {
        IF_PRINT_ELSE_LOG(sim::P()->printLateFrames(), WARN,
            "A frame was late by %v seconds, which is %v percent late.",
            duration - 1.0/sim::P()->frameRate(),
            (duration - 1.0/sim::P()->frameRate())/(1.0/sim::P()->frameRate()) * 100);
    }

    // Sleep the appropriate amount of time, base on the drawing duration
    sim::SimUtilities::sleep(sim::Seconds(std::max(0.0, 1.0/sim::P()->frameRate() - duration)));
}

void keyPress(unsigned char key, int x, int y) {

    if (key == 'p') {
        // Pause the simulation (only in discrete mode)
        sim::S()->setPaused(!sim::S()->paused());
    }
    else if (key == 'f') {
        // Faster (only in discrete mode)
        sim::S()->setSimSpeed(sim::S()->simSpeed() * 1.5);
    }
    else if (key == 's') {
        // Slower (only in discrete mode)
        sim::S()->setSimSpeed(sim::S()->simSpeed() / 1.5);
    }
    else if (key == 'l') {
        // Cycle through the available layouts
        sim::S()->setLayout(sim::LAYOUT_CYCLE.at(sim::S()->layout()));
    }
    else if (key == 'r') {
        // Toggle rotate zoomed map, but only if zoomed map is visible
        if (sim::S()->layout() != sim::Layout::FULL) {
            sim::S()->setRotateZoomedMap(!sim::S()->rotateZoomedMap());
        }
    }
    else if (key == 'i') {
        // Zoom in, but only if zoomed map is visible
        if (sim::S()->layout() != sim::Layout::FULL) {
            sim::S()->setZoomedMapScale(sim::S()->zoomedMapScale() * 1.5);
        }
    }
    else if (key == 'o') {
        // Zoom out, but only if zoomed map is visible
        if (sim::S()->layout() != sim::Layout::FULL) {
            sim::S()->setZoomedMapScale(sim::S()->zoomedMapScale() / 1.5);
        }
    }
    else if (key == 't') {
        // Toggle wall truth visibility
        sim::S()->setWallTruthVisible(!sim::S()->wallTruthVisible());
        g_mazeGraphic->updateWalls();
    }
    else if (key == 'c') {
        // Toggle tile colors
        sim::S()->setTileColorsVisible(!sim::S()->tileColorsVisible());
        g_mazeGraphic->updateColor();
    }
    else if (key == 'x') {
        // Toggle tile text
        sim::S()->setTileTextVisible(!sim::S()->tileTextVisible());
    }
    else if (key == 'g') {
        // Toggle tile fog
        sim::S()->setTileFogVisible(!sim::S()->tileFogVisible());
        g_mazeGraphic->updateFog();
    }
    else if (key == 'w') {
        // Toggle wireframe mode
        sim::S()->setWireframeMode(!sim::S()->wireframeMode());
        glPolygonMode(GL_FRONT_AND_BACK, sim::S()->wireframeMode() ? GL_LINE : GL_FILL);
    }
    else if (key == 'q') {
        // Quit
        sim::SimUtilities::quit();
    }
    else if (std::string("0123456789").find(key) != std::string::npos) {
        // Press an input button
        int inputButton = std::string("0123456789").find(key);
        sim::S()->setInputButtonWasPressed(inputButton, true);
    }
}

void specialKeyPress(int key, int x, int y) {
    if (sim::SimUtilities::vectorContains(sim::ARROW_KEYS, sim::INT_TO_KEY.at(key))) {
        sim::S()->setArrowKeyIsPressed(sim::INT_TO_KEY.at(key), true);
    }
}

void specialKeyRelease(int key, int x, int y) {
    if (sim::SimUtilities::vectorContains(sim::ARROW_KEYS, sim::INT_TO_KEY.at(key))) {
        sim::S()->setArrowKeyIsPressed(sim::INT_TO_KEY.at(key), false);
    }
}

void initRunAndLogging() {

    // First, determine the runId (just datetime, for now)
    std::string runId = sim::SimUtilities::getDateTime();

    // Then we can initiliaze Logging
    sim::Logging::initialize(runId);

    // Initialize the State object in order to:
    // 0) Set the runId
    // 1) Avoid a race condition
    // 2) Register this thread as the main thread
    // 3) Initialize the Param object
    sim::S()->setRunId(runId);

    // Remove any excessive archived runs
    sim::SimUtilities::removeExcessArchivedRuns();
}

void initSimObjects() {
    sim::Maze* maze = new sim::Maze();
    g_mouse = new sim::Mouse(maze);
    g_mazeGraphic = new sim::MazeGraphic(maze);
    g_mouseGraphic = new sim::MouseGraphic(g_mouse);
    g_mouseInterface = new sim::MouseInterface(maze, g_mouse, g_mazeGraphic);
    g_world = new sim::World(maze, g_mouse);
}

void initGraphics(int argc, char* argv[]) {

    // GLUT Initialization
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
    glutInitWindowSize(sim::P()->defaultWindowWidth(), sim::P()->defaultWindowHeight());
    sim::GraphicUtilities::setWindowSize(sim::P()->defaultWindowWidth(), sim::P()->defaultWindowHeight());
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Micromouse Simulator");
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glutDisplayFunc(draw);
    glutIdleFunc(draw);
    glutKeyboardFunc(keyPress);
    glutSpecialFunc(specialKeyPress);
    glutSpecialUpFunc(specialKeyRelease);
    glPolygonMode(GL_FRONT_AND_BACK, sim::S()->wireframeMode() ? GL_LINE : GL_FILL);

    // When the window changes size, notify the graphic utilities
    glutReshapeFunc([](int width, int height){
        glViewport(0,0, width, height);
        sim::GraphicUtilities::setWindowSize(width, height);
    }); 

    // GLEW Initialization
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        sim::SimUtilities::print("Error: Unable to initialize GLEW.");
        sim::SimUtilities::quit();
    }

    // Generate vertex buffer object
    ///*GLuint*/ vertex_buffer_object; // TODO: MACK
    glGenBuffers(1,  &vertex_buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);

    // Generate the vertex shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    std::string str = 
        "#version 110\n"
        "attribute vec2 coordinate;"
        "attribute vec4 color;"
        "uniform mat4 transformationMatrix;"
        "void main(void) {"
        "    gl_Position = transformationMatrix * vec4(coordinate, 0.0, 1.0);"
        "    gl_FrontColor = color;"
        "}";
    const char *vs_source = str.c_str();
    glShaderSource(vs, 1, &vs_source, NULL);
    glCompileShader(vs);

    // Generate the rendering program
    /*GLuint*/ program = glCreateProgram(); // TODO: MACK
    glAttachShader(program, vs);
    glLinkProgram(program);
    glUseProgram(program);

    // Retrieve the attribute IDs and enable our attributes
    GLuint coordinate = glGetAttribLocation(program, "coordinate");
    GLuint color = glGetAttribLocation(program, "color");
    g_transformationMatixId = glGetUniformLocation(program, "transformationMatrix");
    glEnableVertexAttribArray(coordinate);
    glEnableVertexAttribArray(color);

    // Specify the information within our buffer
    glVertexAttribPointer(coordinate, 2, GL_DOUBLE, GL_FALSE, 6 * sizeof(double), 0);
    glVertexAttribPointer(color, 4, GL_DOUBLE, GL_FALSE, 6 * sizeof(double), (char*) NULL + 2 * sizeof(double));

// TODO: MACK
    /* create a font stash with a maximum texture size of 512 x 512 */
    stash = sth_create(512, 512);
    /* load truetype font */
    droid = sth_add_font(stash, (sim::SimUtilities::getProjectDirectory() + "res/fonts/DroidSerif-Regular.ttf").c_str());
// TODO: MACK

    // Lastly, initially populate the vertex buffer object with tile information
    g_mazeGraphic->draw();
}

void initMouseAlgo() {

    // First, check to ensure that the mouse algorithm is valid
    if (!MouseAlgorithms::isMouseAlgorithm(sim::P()->mouseAlgorithm())) {
        sim::SimUtilities::print("Error: \"" + sim::P()->mouseAlgorithm() + "\" is not a valid mouse algorithm.");
        sim::SimUtilities::quit();
    }
    g_algorithm = MouseAlgorithms::getMouseAlgorithm(sim::P()->mouseAlgorithm());

    // Initialize the mouse with the file provided
    std::string mouseFile = g_algorithm->mouseFile();
    bool success = g_mouse->initialize(mouseFile);
    if (!success) {
        sim::SimUtilities::print("Error: Unable to successfully initialize the mouse in the algorithm \""
            + sim::P()->mouseAlgorithm() + "\" from \"" + mouseFile + "\".");
        sim::SimUtilities::quit();
    }

    // Initialize the interface type
    if (!sim::SimUtilities::mapContains(sim::STRING_TO_INTERFACE_TYPE, g_algorithm->interfaceType())) {
        PRINT(ERROR, "\"%v\" is not a valid interface type. You must declare the "
            "interface type of the mouse algorithm \"%v\" to be either \"%v\" or \"%v\".",
            g_algorithm->interfaceType(),
            sim::P()->mouseAlgorithm(),
            sim::INTERFACE_TYPE_TO_STRING.at(sim::InterfaceType::DISCRETE),
            sim::INTERFACE_TYPE_TO_STRING.at(sim::InterfaceType::CONTINUOUS));
        sim::SimUtilities::quit();
    }
    sim::S()->setInterfaceType(sim::STRING_TO_INTERFACE_TYPE.at(g_algorithm->interfaceType()));
}
