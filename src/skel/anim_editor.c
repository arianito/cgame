#include "anim.h"
#include "anim_editor.h"

#include "gui/libgui.h"
#include "math/color.h"

typedef struct
{
    ImDrawList *drawList;
    ImVec2 scroll;
    float zoom;
    struct
    {
        bool dragging;
        ImVec2 lastScroll;
        ImVec2 lastMouse;
    } dragData;

} AnimContext;

static AnimContext self = {NULL, {0}, 0.5, {0}};

#define ED_COLOR_BG (color_hex(color(0.1, 0.1, 0.1, 1.0)))
#define ED_COLOR_INFO (color_hex(color(0.15, 0.15, 0.15, 1.0)))
#define ED_COLOR_INFO_TEXT (color_hex(color(0.9, 0.9, 0.9, 1.0)))
#define ED_COLOR_TIMELINE_HEADER (color_hex(color(0.1, 0.1, 0.1, 1.0)))

#define ED_COLOR_TIMELINE_HEADER_TEXT (color_hex(color(0.5, 0.5, 0.5, 1.0)))
#define ED_COLOR_TIMELINE_HEADER_TEXT2 (color_hex(color(0.5, 0.5, 0.5, 0.5)))
#define ED_COLOR_TIMELINE_KEYFRAME (color_hex(color_red))

inline static void begin(const char *name, const float height)
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoFocusOnAppearing |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGuiIO *io = igGetIO();
    // igSetNextWindowSize(io->DisplaySize, 0);
    // igSetNextWindowPos((ImVec2){0, 0}, 0, (ImVec2){0, 0});
    igSetNextWindowSize((ImVec2){io->DisplaySize.x, height}, 0);
    igSetNextWindowPos((ImVec2){0, io->DisplaySize.y}, 0, (ImVec2){0, 1});

    igPushStyleColor_U32(ImGuiCol_WindowBg, ED_COLOR_BG);
    igPushStyleColor_U32(ImGuiCol_Border, 0);
    igPushStyleVar_Float(ImGuiStyleVar_WindowRounding, 0.0f);
    igBegin(name, NULL, flags);

    self.drawList = igGetWindowDrawList();
}

inline static void end()
{
    igEnd();
    igPopStyleVar(1);
    igPopStyleColor(2);
}

bool igSequencer(const char *name, const float height, Anim *anim)
{
    begin(name, height);

    ImVec2 offset;
    igGetWindowPos(&offset);
    ImVec2 size;
    igGetWindowSize(&size);
    ImVec2 mouse;
    igGetMousePos(&mouse);

    if (igIsWindowHovered(ImGuiHoveredFlags_None) && igIsMouseDragging(ImGuiMouseButton_Right, 1))
    {
        if (!self.dragData.dragging)
        {
            self.dragData.dragging = true;
            self.dragData.lastScroll = self.scroll;
            self.dragData.lastMouse = mouse;
        }
        self.scroll.x = self.dragData.lastScroll.x + (mouse.x - self.dragData.lastMouse.x);
    }

    if (igIsMouseReleased_Nil(ImGuiMouseButton_Right))
    {
        self.dragData.dragging = false;
    }

    float x = offset.x;
    float y = offset.y;
    float info_width = 150;
    float info_height = 28;
    float pad = 4;
    float gap = 8;
    float header_height = 32;
    self.zoom = 1;

    ImDrawList_AddRectFilled(self.drawList, imvec2f(x, y), imvec2f(x + size.x, y + header_height), ED_COLOR_TIMELINE_HEADER, 0, 0);

    float ox;
    float scalePx = 100;
    scalePx *= self.zoom;
    float ssx = repeatf(self.scroll.x, scalePx);
    int psx = floori(self.scroll.x / scalePx);
    int n1 = info_width / scalePx + 1;
    int n2 = size.x / scalePx + 1;
    for (int i = -n1; i < n2; i++)
    {
        ox = info_width + ssx + ((i * scalePx + scalePx / 2));
        ImDrawList_AddLine(self.drawList, imvec2f(ox, y), imvec2f(ox, y + 8), ED_COLOR_TIMELINE_HEADER_TEXT2, 2);

        ox = info_width + ssx + (i * scalePx);
        ImDrawList_AddLine(self.drawList, imvec2f(ox, y), imvec2f(ox, y + 4), ED_COLOR_TIMELINE_HEADER_TEXT, 2);

        float v = i - psx;
        if (1)
        {
            char buff[100];
            int n = snprintf(buff, 100, "%.1f", v);
            ImVec2 sz;
            igCalcTextSize(&sz, buff, buff + n, 0, 0);
            ImDrawList_AddText_Vec2(self.drawList, imvec2f(ox - sz.x / 2, y + 8), ED_COLOR_INFO_TEXT, buff, buff + n);
        }
    }
    y += header_height + gap;
    {
        float fontHeight = igGetTextLineHeight();
        for (int i = 0; i < anim->length; i++)
        {
            AnimSequence *it = &anim->data[i];

            ImVec2 textSize;
            ImDrawList_AddRectFilled(self.drawList, imvec2f(x, y), imvec2f(x + info_width, y + info_height), ED_COLOR_INFO, 0, 0);
            ImDrawList_AddText_Vec2(self.drawList, imvec2f(x + pad, y + info_height / 2 - fontHeight / 2), ED_COLOR_INFO_TEXT, cstr(it->name), cend(it->name));

            for (int j = 0; j < it->length; j++)
            {
                KeyFrame *kf = &it->frames[j];
                float ox = info_width + self.scroll.x + (kf->t * scalePx);
                float rad = 5;
                ImDrawList_AddCircle(self.drawList, imvec2f(ox, y + info_height / 2), rad, ED_COLOR_TIMELINE_KEYFRAME, 8, 2);
            }
            y += info_height + gap;
        }
    }
    end();
    return true;
}