#include "anim.h"
#include "anim_editor.h"

#include "gui/libgui.h"
#include "math/color.h"
#include "engine/game.h"
#include "engine/input.h"

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

static AnimContext self = {NULL, {120, 0}, 1, {0}};

inline static void begin(const char *name, const float height)
{
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoScrollWithMouse |
                                   ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoFocusOnAppearing |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGuiIO *io = igGetIO();

    ImVec2 size = imvec2f(io->DisplaySize.x, height);
    igSetNextWindowSize(size, 0);
    igSetNextWindowPos(imvec2f(0, io->DisplaySize.y), 0, imvec2f(0, 1));

    igPushStyleColor_U32(ImGuiCol_WindowBg, color_hex(color(0.1, 0.1, 0.1, 1.0)));
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

bool igSequencer(const char *name, const float height, AnimSequenceContext *context)
{
    // constants
    const float seq_height = 36;
    const float UNIT = 100;
    const float header_height = 24;

    // begin
    begin(name, height);
    ImVec2 offset;
    igGetWindowPos(&offset);
    ImVec2 size;
    igGetWindowSize(&size);
    ImVec2 mouse;
    igGetMousePos(&mouse);

    // handle zoom
    if (igIsWindowHovered(0))
    {
        if (input->wheel.y != 0)
        {
            float pz = self.zoom;
            self.zoom *= 1 + (input->wheel.y * gtime->delta * 4.0);
            float ofc = self.zoom - pz;
            float m = (mouse.x - self.scroll.x) / self.zoom;
            self.scroll.x -= m * ofc;
        }
    }
    float scalePx = UNIT * self.zoom;
    int nStep = self.zoom < 1 ? 4 : floof(maxf(self.zoom, 1)) * 10;

    // handle pan
    if (igIsWindowHovered(0) && igIsMouseDown_Nil(ImGuiMouseButton_Middle))
    {
        if (!self.dragData.dragging)
        {
            self.dragData.dragging = true;
            self.dragData.lastScroll = self.scroll;
            self.dragData.lastMouse = mouse;
        }
        self.scroll.x = self.dragData.lastScroll.x + (mouse.x - self.dragData.lastMouse.x);
        self.scroll.y = self.dragData.lastScroll.y + (mouse.y - self.dragData.lastMouse.y);
    }
    
    if (igIsMouseReleased_Nil(ImGuiMouseButton_Middle))
    {
        self.dragData.dragging = false;
    }
    // jump time
    if (igIsWindowHovered(0) && igIsMouseDragging(ImGuiMouseButton_Left, 0))
    {
        context->time = (mouse.x - self.scroll.x) / scalePx;

        if (igIsKeyDown_Nil(ImGuiKey_LeftCtrl))
        {
            context->time = snapf(context->time, 1.0 / nStep);
        }
    }

    float fontHeight = igGetTextLineHeight();
    {
        const int ED_COLOR_TEXT = (color_hex(color_alpha(color_white, 0.75)));
        const int ED_COLOR_LIGHT = (color_hex(color_alpha(color_white, 0.25)));
        const int ED_COLOR_DARK = (color_hex(color_alpha(color_white, 0.1)));

        // draw grid
        float y = offset.y;
        float x = offset.x;
        float ssx = repeatf(self.scroll.x, scalePx);
        float psx = floof(self.scroll.x / scalePx);
        int n = (size.x / scalePx + 1);
        for (int i = -3; i < n; i++)
        {
            for (int j = 1; j < nStep; j++)
            {
                float h = j % 2 == 0 ? 8 : 4;
                float ox = ssx + ((i * scalePx + j * (scalePx / (float)(nStep))));
                ImDrawList_AddLine(self.drawList, imvec2f(ox, y), imvec2f(ox, y + h), ED_COLOR_DARK, 2);
            }
            float ox = ssx + (i * scalePx);
            ImDrawList_AddLine(self.drawList, imvec2f(ox, y), imvec2f(ox, y + 4), ED_COLOR_LIGHT, 2);
            ImDrawList_AddLine(self.drawList, imvec2f(ox, y), imvec2f(ox, y + size.y), ED_COLOR_DARK, 1);

            float v = (i - psx);
            if (v >= 0 && absf(v - context->time) > (2.0 / nStep))
            {
                ImVec2 sz;
                char buff[10];
                int n = snprintf(buff, 10, "%.0f", v);
                igCalcTextSize(&sz, buff, buff + n, 0, 0);
                ImDrawList_AddText_Vec2(self.drawList, imvec2f(ox - sz.x / 2, y + 8), ED_COLOR_TEXT, buff, buff + n);
            }
        }
    }
    {
        const int ED_COLOR_CURSOR = (color_hex(color_red));
        float x = offset.x;
        float y = offset.y;
        float ox = self.scroll.x + (context->time * scalePx);
        ImDrawList_AddLine(self.drawList, imvec2f(ox, y + header_height + 6), imvec2f(ox, y + size.y), ED_COLOR_CURSOR, 3);
        ImDrawList_AddLine(self.drawList, imvec2f(ox - 10, y + 2), imvec2f(ox + 10, y + 2), ED_COLOR_CURSOR, 3);




        ImVec2 sz;
        char buff[10];
        int n = snprintf(buff, 10, "%.3f", context->time);
        igCalcTextSize(&sz, buff, buff + n, 0, 0);
        ImDrawList_AddText_Vec2(self.drawList, imvec2f(ox - sz.x / 2, y + 8), ED_COLOR_CURSOR, buff, buff + n);
    }
    {
        
        const int ED_COLOR_TEXT = (color_hex(color_alpha(color_white, 0.9)));
        const int ED_COLOR_BACKDROP = (color_hex(color(0.14, 0.14, 0.15, 0.75)));
        const int ED_COLOR_PILL = (color_hex(color(0.32, 0.35, 0.39, 0.75)));
        const int ED_COLOR_KEYFRAME = (color_hex(color_green));
        // draw key frames
        float pad = 24;
        float gap = 10;
        float x = offset.x;
        float y = offset.y + self.scroll.y;
        y += header_height + gap;
        for (int i = 0; i < context->anim->length; i++)
        {
            AnimSequence *it = &context->anim->data[i];
            // backdrop

            ImDrawList_AddRectFilled(self.drawList, imvec2f(x, y), imvec2f(size.x, y + seq_height), ED_COLOR_BACKDROP, 0, 0);
            for (int j = 0; j < it->length; j++)
            {
                KeyFrame *kf = &it->frames[j];
                float ox = self.scroll.x + (kf->t * scalePx);
                float rad = 5;
                ImDrawList_AddCircle(self.drawList, imvec2f(ox, y + seq_height / 2), rad, ED_COLOR_KEYFRAME, 8, 4);
            }

            // seq
            ImVec2 sz;
            igCalcTextSize(&sz, cstr(it->name), cend(it->name), 0, 0);
            ImDrawList_AddRectFilled(self.drawList, imvec2f(x, y), imvec2f(x + sz.x + pad * 2, y + seq_height), ED_COLOR_PILL, seq_height, 0);
            ImDrawList_AddText_Vec2(self.drawList, imvec2f(x + pad, y + seq_height / 2 - fontHeight / 2), ED_COLOR_TEXT, cstr(it->name), cend(it->name));

            y += seq_height + gap;
        }
    }
    end();
    return true;
}