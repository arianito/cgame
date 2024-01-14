
#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3

#include "libgui.h"
#include <stddef.h>
#include <stdbool.h>
#include "gui/cimgui_impl.h"

extern "C" {
    #include "engine/input.h"
    #include "engine/file.h"
}


static ImFont* defaultFont;
CIMGUI_API void gui_init(void *context, const char* font)
{
    igCreateContext(NULL);
    
    ImGuiIO* io = igGetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)context, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    igStyleColorsDark(NULL);

    if(font != NULL) {
        StrView path = resolve_stack(font);
        defaultFont = ImFontAtlas_AddFontFromFileTTF(io->Fonts, path.string, 20, NULL, ImFontAtlas_GetGlyphRangesDefault(io->Fonts));
        xxfreestack(path.string);
    }
    
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

    ImGuiIO* io = igGetIO();
    if(io->WantCaptureMouse) {
        input_disable();
    }
}

CIMGUI_API void gui_destroy()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(NULL);
}