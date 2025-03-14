#include "application.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <queue>
#include <tuple>
#include <filesystem>

#include <flags/flags.h>

#include "../states/play-state.hpp"

#include<windows.h>
#include <Mmsystem.h>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")


// Include the Dear ImGui implementation headers
#define IMGUI_IMPL_OPENGL_LOADER_GLAD2
#include <imgui_impl/imgui_impl_glfw.h>
#include <imgui_impl/imgui_impl_opengl3.h>

#if !defined(NDEBUG)
// If NDEBUG (no debug) is not defined, enable OpenGL debug messages
#define ENABLE_OPENGL_DEBUG_MESSAGES
#endif

#include "texture/screenshot.hpp"

std::string default_screenshot_filepath() {
    std::stringstream stream;
    auto time = std::time(nullptr);
    
    struct tm localtime;
    localtime_s(&localtime, &time);
    stream << "screenshots/screenshot-" << std::put_time(&localtime, "%Y-%m-%d-%H-%M-%S") << ".png";
    return stream.str();
}

// This function will be used to log errors thrown by GLFW
void glfw_error_callback(int error, const char* description){
    std::cerr << "GLFW Error: " << error << ": " << description << std::endl;
}

// This function will be used to log OpenGL debug messages
void GLAPIENTRY opengl_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    std::string _source;
    std::string _type;
    std::string _severity;

    // What is the source of the message
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            _source = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _source = "WINDOW SYSTEM"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _source = "SHADER COMPILER"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            _source = "THIRD PARTY"; break;
        case GL_DEBUG_SOURCE_APPLICATION:
            _source = "APPLICATION"; break;
        case GL_DEBUG_SOURCE_OTHER: default:
            _source = "UNKNOWN"; break;
    }

    // What is the type of the message (error, warning, etc).
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            _type = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _type = "DEPRECATED BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _type = "UDEFINED BEHAVIOR"; break;
        case GL_DEBUG_TYPE_PORTABILITY:
            _type = "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            _type = "PERFORMANCE"; break;
        case GL_DEBUG_TYPE_OTHER:
            _type = "OTHER"; break;
        case GL_DEBUG_TYPE_MARKER:
            _type = "MARKER"; break;
        default:
            _type = "UNKNOWN"; break;
    }

    // How severe is the message
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            _severity = "HIGH"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            _severity = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW:
            _severity = "LOW"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            _severity = "NOTIFICATION"; break;
        default:
            _severity = "UNKNOWN"; break;
    }

    std::cout << "OpenGL Debug Message " << id << " (type: " << _type << ") of " << _severity
    << " raised from " << _source << ": " << message << std::endl;
}

void our::Application::configureOpenGL() {
    // Request that OpenGL is 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Only enable core functionalities (disable features from older OpenGL versions that were removed in 3.3)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Enable forward compatibility with newer OpenGL versions by removing deprecated functionalities
    // This is necessary for some platforms
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    //Make window size fixed (User can't resize it)
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //Set Number of sample used in MSAA (0 = Disabled)
    glfwWindowHint(GLFW_SAMPLES, 0);

    //Enable Double Buffering
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    //Set the bit-depths of the frame buffer
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);

    //Set Bits for Depth Buffer
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    //Set Bits for Stencil Buffer
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    //Set the refresh rate of the window (GLFW_DONT_CARE = Run as fast as possible)
    glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
}

our::WindowConfiguration our::Application::getWindowConfiguration() {
    auto window_config = app_config["window"];
    std::string title = window_config["title"].get<std::string>();
    
    int width = window_config["size"]["width"].get<int>();
    int height = window_config["size"]["height"].get<int>();

    bool isFullScreen = window_config["fullscreen"].get<bool>();

    return {title, {width, height}, isFullScreen};
}

// This is the main class function that run the whole application (Initialize, Game loop, House cleaning).
// run_for_frames decides how many frames should be run before the application automatically closes.
// if run_for_frames == 0, the application runs indefinitely till manually closed.
int our::Application::run(int run_for_frames) {

    // Set the function to call when an error occurs.
    glfwSetErrorCallback(glfw_error_callback);

    // Initialize GLFW and exit if it failed
    if(!glfwInit()){
        std::cerr << "Failed to Initialize GLFW" << std::endl;
        return -1;
    }

    configureOpenGL(); // This function sets OpenGL window hints.

    auto win_config = getWindowConfiguration();             // Returns the WindowConfiguration current struct instance.

    // Create a window with the given "WindowConfiguration" attributes.
    // If it should be fullscreen, monitor should point to one of the monitors (e.g. primary monitor), otherwise it should be null
    GLFWmonitor* monitor = win_config.isFullscreen ? glfwGetPrimaryMonitor() : nullptr;
    // The last parameter "share" can be used to share the resources (OpenGL objects) between multiple windows.
    window = glfwCreateWindow(win_config.size.x, win_config.size.y, win_config.title.c_str(), monitor, nullptr);
    if(!window) {
        std::cerr << "Failed to Create Window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);         // Tell GLFW to make the context of our window the main context on the current thread.

    gladLoadGL(glfwGetProcAddress);         // Load the OpenGL functions from the driver

    // Print information about the OpenGL context
    std::cout << "VENDOR          : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "RENDERER        : " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "VERSION         : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL VERSION    : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

#if defined(ENABLE_OPENGL_DEBUG_MESSAGES)
    // if we have OpenGL debug messages enabled, set the message callback
    glDebugMessageCallback(opengl_callback, nullptr);
    // Then enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    // Then make the output synchronized to the OpenGL commands.
    // This will make sure that OpenGL and the main thread are synchronized such that message callback is called as soon
    // as the command causing it is called. This is useful for debugging but slows down the code execution.
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

    setupCallbacks();
    keyboard.enable(window);
    mouse.enable(window);

    // Start the ImGui context and set dark style (just my preference :D)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // fonts we use in writing in game window
    ImFont *font = io.Fonts->AddFontFromFileTTF("assets\\fonts\\Ruda-Bold.ttf", 30.0f);
    ImFont *fontBlopy = io.Fonts->AddFontFromFileTTF("assets\\fonts\\fontBlopy.ttf", 60.0f);
    ImFont *fontGameOver = io.Fonts->AddFontFromFileTTF("assets\\fonts\\game-over.ttf", 100.0f);
    ImFont *fontItalic = io.Fonts->AddFontFromFileTTF("assets\\fonts\\fontItalic.ttf", 130.0f);

    ImGui::StyleColorsDark();

    // Initialize ImGui for GLFW and OpenGL
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // This part of the code extracts the list of requested screenshots and puts them into a priority queue
    using ScreenshotRequest = std::pair<int, std::string>;
    std::priority_queue<
        ScreenshotRequest, 
        std::vector<ScreenshotRequest>, 
        std::greater<ScreenshotRequest>> requested_screenshots;
    if(auto& screenshots = app_config["screenshots"]; screenshots.is_object()) {
        auto base_path = std::filesystem::path(screenshots.value("directory", "screenshots"));
        if(auto& requests = screenshots["requests"]; requests.is_array()) {
            for(auto& item : requests){
                auto path = base_path / item.value("file", "");
                int frame = item.value("frame", 0);
                requested_screenshots.push({ frame, path.string() });
            }
        }
    }

    // If a scene change was requested, apply it
    if(nextState) {
        currentState = nextState;
        nextState = nullptr;
    }
    // Call onInitialize if the scene needs to do some custom initialization (such as file loading, object creation, etc).
    if(currentState) currentState->onInitialize();

    // The time at which the last frame started. But there was no frames yet, so we'll just pick the current time.
    double last_frame_time = glfwGetTime();
    int current_frame = 0;

    //Game loop
    while(!glfwWindowShouldClose(window)){
        if(run_for_frames != 0 && current_frame >= run_for_frames) break;
        glfwPollEvents(); // Read all the user events and call relevant callbacks.

        //change background edited by me
        // glClearColor( 0.4375, 0.6875, 0.9375, 1.0);
        //glfwSwapBuffers(window); 
        // Start a new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(currentState) currentState->onImmediateGui(); // Call to run any required Immediate GUI.

        // if the state is menu state:
        if (currentState == states["menu"])
        {
            // set window size of the menu
            ImGui::SetNextWindowSize(ImVec2(900, 700));
            // set the position of the menu
            ImGui::SetWindowPos("Main menu", ImVec2(200, 10));
            // start window GUI
            ImGui::Begin("Main menu", nullptr,ImGuiWindowFlags_NoMove);


            ImGuiStyle *style = &ImGui::GetStyle();
            style->WindowMenuButtonPosition = ImGuiDir_None;
            ImVec4 *colors = style->Colors;

            // set colors for button and 
            colors[ImGuiCol_Button] = ImVec4(149.0f / 256, 72.0f / 256, 100.0f/ 256, 1.0f);
            colors[ImGuiCol_ButtonActive] = ImVec4(168.0f / 256, 89.0f / 256, 118.0f/ 256, 1.0f);
            colors[ImGuiCol_ButtonHovered] = ImVec4(168.0f / 256, 89.0f / 256, 118.0f/ 256, 1.0f);
            colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.0f / 256, 0.0f / 256, 255.0f, 1.0f);
            colors[ImGuiCol_TitleBgActive] = ImVec4(168.0f / 256, 89.0f / 256, 118.0f/ 256, 1.0f);
            // colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            colors[ImGuiCol_ResizeGrip] = ImVec4(149.0f / 256, 72.0f / 256, 100.0f/ 256, 1.0f);
            colors[ImGuiCol_ResizeGripActive] = ImVec4(149.0f / 256, 72.0f / 256, 100.0f/ 256, 1.0f);
            colors[ImGuiCol_ResizeGripHovered] = ImVec4(149.0f / 256, 72.0f / 256, 100.0f/ 256, 1.0f);

            // set cursor position of (x, y) to write "Way To Home" in the center of the window
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Way To Home").x) * 0.35);
            ImGui::SetCursorPosY((ImGui::GetWindowSize().y - ImGui::CalcTextSize("Way To Home").y) * 0.2);

            // set font for writing the title
            ImGui::PushFont(fontBlopy);
            // write the title
            ImGui::Text("Way To Home");
            // pop the font after writing
            ImGui::PopFont();

            // set font for writing "Start" and "Exit" in buttons
            ImGui::PushFont(font);

            // set cursor position of (x, y) to write "Start" in the left of the window
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Start").x) * 0.2);
            ImGui::SetCursorPosY((ImGui::GetWindowSize().y - ImGui::CalcTextSize("Start").y) * 0.5);

            // if button for Start pressed:
            if (ImGui::Button("Start", ImVec2(200, 100)))
            {
                // calculate the current time and make it our start the time
                time(&start_time);
                // play sound for starting the game
                PlaySound("assets/sound/start.wav", NULL, SND_ASYNC);
                // change the state to play state
                changeState("play");
            }

            // set cursor position of (x, y) to write "Exit" in the right of the window
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Exit").x) * 0.6);
            ImGui::SetCursorPosY((ImGui::GetWindowSize().y - ImGui::CalcTextSize("Exit").y) * 0.5);

            // if button for Exit pressed:
            if (ImGui::Button("Exit", ImVec2(200, 100)))
            {
                // end the play
                return 0; 
            }
            // pop the font after writing
            ImGui::PopFont();
            // end the GUI
            ImGui::End();
        }

        // if the state is play state:
        else if (currentState == states["play"])
        {
            // calculate the current time and make it our end time
            time(&end_time);
            // our implementation that the game time is 60 second
            // if the difference between start and end is 60, then the game is finished
            // if the player hit the penalty, then the game is finished also
            if (abs(start_time - end_time) >= 31 || penalty){
                // if game finished with penalty:
                if(penalty)
                    // play sound for game over
                    PlaySound("assets/sound/loser.wav", NULL, SND_ASYNC);
                // if the game finished after 60 second:    
                else
                    // play sound for winning
                    PlaySound("assets/sound/winner.wav", NULL, SND_ASYNC);
                // for both scenarios we change the state to gameOver state 
                changeState("gameOver");
            }

            // set the window size for the game
            ImGui::SetNextWindowSize(ImVec2(1280, 200));
            // start window GUI
            ImGui::Begin(" ", nullptr, ImGuiWindowFlags_NoMove);
            // set the position of the game
            ImGui::SetWindowPos(" ", ImVec2(0, 0));

            ImGuiStyle *style = &ImGui::GetStyle();

            ImVec4 *colors = style->Colors;

            // set the color for window background
            colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            colors[ImGuiCol_ResizeGripActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

            // set the cursor (x, y) to write number of rewards
            ImGui::SetCursorPosX(960);
            ImGui::SetCursorPosY(60);
            
            // set the font to write rewards
            ImGui::PushFont(font);
            // first string for word "REWARD: "
            std::string string1 = "REWARD: ";
            // second string for the number of rewards
            std::string string2 = std::to_string(reward);
            // concatenate both strings
            std::string stringLine = string1 + string2;
            // write it in GUI
            ImGui::Text(stringLine.c_str());
            // pop the font after writing
            ImGui::PopFont();

            // ImGui::SetCursorPosX(960);
            // ImGui::SetCursorPosY(40);
            
            // ImGui::PushFont(font);
            // std::string string3 = "LIVE: ";
            // std::string string4 = std::to_string(penalty);
            // std::string stringLine = string3 + string4;
            // ImGui::Text(stringLine.c_str());
            // ImGui::PopFont();
            
            // end the GUI
            ImGui::End();
        }
        
        // if the state is "Losing" or "Winnig" state:
        else
        {
            // set the window size for the "Losing" or "Winnig"
            ImGui::SetNextWindowSize(ImVec2(1000, 1000));
            // start window GUI
            ImGui::Begin(" ", nullptr, ImGuiWindowFlags_NoMove);
            // set the position of the "Losing" or "Winnig"
            ImGui::SetWindowPos(" ", ImVec2(150, -150));

            ImGuiStyle *style = &ImGui::GetStyle();
            ImVec4 *colors = style->Colors;
            // set color for window background 
            colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);
            // set the window size for the "Losing" or "Winnig"
            ImGui::SetNextWindowSize(ImVec2(1500, 100));

            // set the position of (x, y) for writing "GAME OVER" in the center of the window
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("GAME OVER").x) * 0.2);
            ImGui::SetCursorPosY((ImGui::GetWindowSize().y - ImGui::CalcTextSize("GAME OVER").y) * 0.2);
            // set the font for writing "GAME OVER"
            ImGui::PushFont(fontGameOver);
            // if the game ended with penalty:
            if (penalty)
                // we write "GAME OVER" in the middle of the window
                ImGui::Text("GAME OVER");
            else
            // else if the user is the winner
            {
                 // set the position of (x, y) for writing "GOOD JOB!" in the center of the window
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("GOOD JOB!").x) * 0.5);
                ImGui::SetCursorPosY((ImGui::GetWindowSize().y - ImGui::CalcTextSize("GOOD JOB!").y) * 0.2);
                // we write "GAME OVER" in the middle of the window
                ImGui::Text("GOOD JOB!");
            }
            // pop the font after writing
            ImGui::PopFont();

             // set the position of (x, y) for writing "REWARD: " in the center of the window
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("GAME OVER").x) * 0.4);
            ImGui::PushFont(fontItalic);
            // write the "REWARD: " and the number of rewards
            std::string string1 = "REWARD: ";
            std::string string2 = std::to_string(reward);
            std::string stringLine = string1 + string2;
            ImGui::Text(stringLine.c_str());

            // pop the font after writing
            ImGui::PopFont();

            // set the cursor (x, y) to write "Restart" at the left of window
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Restart").x) * 0.3);
            ImGui::SetCursorPosY((ImGui::GetWindowSize().y - ImGui::CalcTextSize("Restart").y) * 0.5);
            
            // if the button of "Restart" pressed:
            if (ImGui::Button("Restart", ImVec2(200, 100)))
            {
                // calculate the current time and make it our start the time
                time(&start_time);
                // set penalty to false
                penalty = false;
                // set reward to 0
                reward = 0;
                // change the state to play state
                registerState<Playstate>("play");
                changeState("play");
                // play sound for starting the game
                PlaySound("assets/sound/start.wav", NULL, SND_ASYNC);
            }

            // set the cursor (x, y) to write "Exit" at the right of window
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Exit").x) * 0.6);
            ImGui::SetCursorPosY((ImGui::GetWindowSize().y - ImGui::CalcTextSize("Exit").y) * 0.5);

            // if the button of "Exit" pressed:
            if (ImGui::Button("Exit", ImVec2(200, 100)))
            {
                // end the play
                return 0;
            }
            // end the GUI
            ImGui::End();
        }


        // If ImGui is using the mouse or keyboard, then we don't want the captured events to affect our keyboard and mouse objects.
        // For example, if you're focusing on an input and writing "W", the keyboard object shouldn't record this event.
        keyboard.setEnabled(!io.WantCaptureKeyboard, window);
        mouse.setEnabled(!io.WantCaptureMouse, window);

        // Render the ImGui commands we called (this doesn't actually draw to the screen yet.
        ImGui::Render();

        // Just in case ImGui changed the OpenGL viewport (the portion of the window to which we render the geometry),
        // we set it back to cover the whole window
        auto frame_buffer_size = getFrameBufferSize();
        glViewport(0, 0, frame_buffer_size.x, frame_buffer_size.y);

        // Get the current time (the time at which we are starting the current frame).
        double current_frame_time = glfwGetTime();

        // Call onDraw, in which we will draw the current frame, and send to it the time difference between the last and current frame
        if(currentState) currentState->onDraw(current_frame_time - last_frame_time);
        last_frame_time = current_frame_time; // Then update the last frame start time (this frame is now the last frame)

#if defined(ENABLE_OPENGL_DEBUG_MESSAGES)
        // Since ImGui causes many messages to be thrown, we are temporarily disabling the debug messages till we render the ImGui
        glDisable(GL_DEBUG_OUTPUT);
        glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Render the ImGui to the framebuffer
#if defined(ENABLE_OPENGL_DEBUG_MESSAGES)
        // Re-enable the debug messages
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

        // If F12 is pressed, take a screenshot
        if(keyboard.justPressed(GLFW_KEY_F12)){
            glViewport(0, 0, frame_buffer_size.x, frame_buffer_size.y);
            std::string path = default_screenshot_filepath();
            if(our::screenshot_png(path)){
                std::cout << "Screenshot saved to: " << path << std::endl;
            } else {
                std::cerr << "Failed to save a Screenshot" << std::endl;
            }
        }
        // There are any requested screenshots, take them
        while(requested_screenshots.size()){ 
            if(const auto& request = requested_screenshots.top(); request.first == current_frame){
                if(our::screenshot_png(request.second)){
                    std::cout << "Screenshot saved to: " << request.second << std::endl;
                } else {
                    std::cerr << "Failed to save a screenshot to: " << request.second << std::endl;
                }
                requested_screenshots.pop();
            } else break;
        }

        // Swap the frame buffers
        glfwSwapBuffers(window);

        // Update the keyboard and mouse data
        keyboard.update();
        mouse.update();

        // If a scene change was requested, apply it
        while(nextState){
            // If a scene was already running, destroy it (not delete since we can go back to it later)
            if(currentState) currentState->onDestroy();
            // Switch scenes
            currentState = nextState;
            nextState = nullptr;
            // Initialize the new scene
            currentState->onInitialize();
        }

        ++current_frame;
    }

    // Call for cleaning up
    if(currentState) currentState->onDestroy();

    // Shutdown ImGui & destroy the context
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Destroy the window
    glfwDestroyWindow(window);

    // And finally terminate GLFW
    glfwTerminate();
    return 0; // Good bye
}

// Sets-up the window callback functions from GLFW to our (Mouse/Keyboard) classes.
void our::Application::setupCallbacks() {

    // We use GLFW to store a pointer to "this" window instance.
    glfwSetWindowUserPointer(window, this);
    // The pointer is then retrieved in the callback function.

    // The second parameter to "glfwSet---Callback" is a function pointer.
    // It is replaced by an inline function -lambda expression- as it is not needed to create
    // a seperate function for it.
    // In the inline function we retrieve the window instance and use it to set our (Mouse/Keyboard) classes values.

    // Keyboard callbacks
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->getKeyboard().keyEvent(key, scancode, action, mods);
            if(app->currentState) app->currentState->onKeyEvent(key, scancode, action, mods);
        }
    });

    // mouse position callbacks
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x_position, double y_position){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->getMouse().CursorMoveEvent(x_position, y_position);
            if(app->currentState) app->currentState->onCursorMoveEvent(x_position, y_position);
        }
    });

    // mouse position callbacks
    glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            if(app->currentState) app->currentState->onCursorEnterEvent(entered);
        }
    });

    // mouse button position callbacks
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if(app){
            app->getMouse().MouseButtonEvent(button, action, mods);
            if(app->currentState) app->currentState->onMouseButtonEvent(button, action, mods);
        }
    });
}
