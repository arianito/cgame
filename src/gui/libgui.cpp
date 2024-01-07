
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3

#include "libgui.h"
#include <stddef.h>
#include <stdbool.h>
#include "gui/cimgui_impl.h"

CIMGUI_API void gui_init(void *context)
{
    igCreateContext(NULL);
    
    ImGuiIO* io = igGetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)context, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    igStyleColorsDark(NULL);
}
CIMGUI_API void gui_begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
}
CIMGUI_API void gui_end()
{
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

CIMGUI_API void gui_destroy()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(NULL);
}