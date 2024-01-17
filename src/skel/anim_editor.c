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
    ImVec2 scale;
    struct
    {
        bool dragging;
        ImVec2 lastScroll;
        ImVec2 lastMouse;
    } dragData;
    float sidebarWidth;
    ImGuiID id;
} AnimContext;

static AnimContext self = {0};

#define UNIT 100

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
    if (igGetCurrentWindow()->ID != self.id)
    {
        self = (AnimContext){0};
        self.drawList = igGetWindowDrawList();
        self.sidebarWidth = MIN_FLOAT;
        self.scroll = imvec2f(0, 0);
        self.scale = imvec2f(1, 1);
        self.dragData.dragging = false;
    }
}

inline static void end()
{
    self.id = igGetCurrentWindow()->ID;
    igEnd();
    igPopStyleVar(1);
    igPopStyleColor(2);
}

bool igSequencer(const char *name, const float height, AnimSequenceContext *context)
{
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

            if (!igIsKeyDown_Nil(ImGuiKey_LeftCtrl))
            {
                float pz = self.scale.x;
                self.scale.x *= 1 + (input->wheel.y * gtime->delta * 4.0);
                float ofc = self.scale.x - pz;
                float m = (mouse.x - self.scroll.x) / self.scale.x;
                self.scroll.x -= m * ofc;
            }
        }
    }
    float scalePx = UNIT * self.scale.x;
    int nStepX = self.scale.x < 1 ? 4 : floof(maxf(self.scale.x, 1)) * 10;

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
    if (igIsWindowHovered(0) && igIsMouseDragging(ImGuiMouseButton_Left, 1))
    {
        context->time = (mouse.x - self.scroll.x) / scalePx;

        if (igIsKeyDown_Nil(ImGuiKey_LeftCtrl))
        {
            context->time = snapf(context->time, 1.0 / nStepX);
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
            for (int j = 1; j < nStepX; j++)
            {
                float h = j % 2 == 0 ? 8 : 4;
                float ox = ssx + ((i * scalePx + j * (scalePx / (float)(nStepX))));
                ImDrawList_AddLine(self.drawList, imvec2f(ox, y), imvec2f(ox, y + h), ED_COLOR_DARK, 2);
            }
            float ox = ssx + (i * scalePx);
            ImDrawList_AddLine(self.drawList, imvec2f(ox, y), imvec2f(ox, y + 4), ED_COLOR_LIGHT, 2);
            ImDrawList_AddLine(self.drawList, imvec2f(ox, y + fontHeight * 2), imvec2f(ox, y + size.y), ED_COLOR_DARK, 1);

            float v = (i - psx);
            if (v >= 0 && absf(v - context->time) > (2.0 / nStepX))
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
        char buff[20];
        int n = snprintf(buff, 20, "%.3f", context->time);
        igCalcTextSize(&sz, buff, buff + n, 0, 0);
        ImDrawList_AddText_Vec2(self.drawList, imvec2f(ox - sz.x / 2, y + 8), ED_COLOR_CURSOR, buff, buff + n);
    }
    {

        const int ED_COLOR_TEXT = (color_hex(color_alpha(color_white, 0.9)));
        const int ED_COLOR_BACKDROP = (color_hex(color(0.14, 0.14, 0.15, 0.5)));
        const int ED_COLOR_PILL = (color_hex(color(0.2, 0.2, 0.2, 0.7)));
        const int ED_COLOR_PILL_ACTIVE = (color_hex(color(0.1, 0.2, 0.3, 0.75)));
        const int ED_COLOR_KEYFRAME = (color_hex(color_green));
        const int ED_COLOR_VAL = (color_hex(color(0.8, 0.5, 0.5, 0.5)));
        const int ED_COLOR_RED = (color_hex(color_alpha(color_red, 0.7)));
        // draw key frames
        float pad = 12;
        float gap = 10;
        float x = offset.x;
        float y = offset.y + self.scroll.y + header_height + gap;

        for (int i = 0; i < context->anim->length; i++)
        {
            AnimSequence *it = &context->anim->data[i];
            // calculate text size
            ImVec2 sz;
            igCalcTextSize(&sz, cstr(it->name), cend(it->name), 0, 0);
            self.sidebarWidth = maxf(self.sidebarWidth, sz.x);
            float sidebar_width = self.sidebarWidth + pad * 2;
            float height = 0;
            // expanded
            if (it->state0 & ANIM_SEQ_STATE_EXPAND)
            {
                // draw backdrop
                height = UNIT * 2 * it->scale0;
                float gh = height - 36;
                ImDrawList_AddRectFilled(self.drawList, imvec2f(x, y), imvec2f(size.x, y + height), ED_COLOR_BACKDROP, 0, 0);

                // determine min/max
                float min = MAX_FLOAT;
                float max = MIN_FLOAT;
                for (int j = 0; j < it->length; j++)
                {
                    KeyFrame *kf = &it->frames[j];
                    min = minf(min, kf->value);
                    max = maxf(max, kf->value);
                }
                float dts = absf(max - min);
                // draw
                {

                    float yval = anim_iterpolate(it, context->time);
                    float oy = (y + height / 2) - (((yval - min) / dts - 0.5f) * gh);

                    ImVec2 sz;
                    char buff[20];
                    int n = snprintf(buff, 20, "%.3f", yval);
                    igCalcTextSize(&sz, buff, buff + n, 0, 0);
                    ImDrawList_AddText_Vec2(self.drawList, imvec2f(size.x - sz.x - 10, oy - sz.y / 2), ED_COLOR_VAL, buff, buff + n);

                    ImDrawList_AddLine(self.drawList, imvec2f(sidebar_width, oy), imvec2f(size.x - sz.x - 20, oy), ED_COLOR_VAL, 2);
                    ImDrawList_AddLine(self.drawList, imvec2f(size.x - 4, oy - 10), imvec2f(size.x - 4, oy + 10), ED_COLOR_VAL, 2);
                }

                for (int j = 0; j < it->length; j++)
                {
                    KeyFrame *kf = &it->frames[j];
                    ImVec2 pt = imvec2f(
                        self.scroll.x + (kf->t * scalePx),
                        (y + height / 2) - (((kf->value - min) / dts - 0.5f) * gh));

                    float rad = 6;
                    ImDrawList_AddCircleFilled(self.drawList, pt, rad, ED_COLOR_KEYFRAME, 4);

                    if (j > 0)
                    {
                        KeyFrame *pkf = &it->frames[j - 1];

                        ImVec2 qs[4];
                        anim_control_points(pkf, kf, &qs[0], &qs[1], &qs[2], &qs[3]);

                        for (int i = 0; i < 4; i++)
                        {
                            qs[i].x = self.scroll.x + (qs[i].x * scalePx);
                            qs[i].y = (y + height / 2) - (((qs[i].y - min) / dts - 0.5f) * gh);
                        }

                        ImDrawList_AddBezierCubic(self.drawList, qs[0], qs[1], qs[2], qs[3], ED_COLOR_KEYFRAME, 2, 10);

                        ImDrawList_AddLine(self.drawList, qs[0], qs[1], ED_COLOR_RED, 1);
                        ImDrawList_AddLine(self.drawList, qs[2], qs[3], ED_COLOR_RED, 1);
                        ImDrawList_AddCircleFilled(self.drawList, qs[1], 3, ED_COLOR_RED, 6);
                        ImDrawList_AddCircleFilled(self.drawList, qs[2], 3, ED_COLOR_RED, 6);
                    }
                }

                if (input->wheel.y != 0 &&
                    igIsKeyDown_Nil(ImGuiKey_LeftCtrl) &&
                    igIsMouseHoveringRect(imvec2f(x + sidebar_width, y), imvec2f(size.x, y + height), true))
                {
                    it->scale0 *= 1 + (input->wheel.y * gtime->delta * 4.0);
                }
            }
            else
            {
                // normal
                height = 36;
                ImDrawList_AddRectFilled(self.drawList, imvec2f(x, y), imvec2f(size.x, y + height), ED_COLOR_BACKDROP, 0, 0);

                for (int j = 0; j < it->length; j++)
                {
                    KeyFrame *kf = &it->frames[j];
                    float ox = self.scroll.x + (kf->t * scalePx);
                    float rad = 6;
                    ImDrawList_AddCircleFilled(self.drawList, imvec2f(ox, y + height / 2), rad, ED_COLOR_KEYFRAME, 4);
                }
            }

            // seq
            bool cls = self.scroll.x >= sidebar_width;
            float wd = cls ? sidebar_width : (fontHeight + pad);
            ImDrawList_AddRectFilled(self.drawList, imvec2f(x, y), imvec2f(x + wd, y + height), ED_COLOR_PILL, 0, 0);
            ImDrawList_AddText_Vec2(self.drawList, imvec2f(x + pad, y + height / 2 - fontHeight / 2), ED_COLOR_TEXT,
                                    cstr(it->name), cls ? cend(it->name) : (cstr(it->name) + 1));

            if (igIsMouseClicked_Bool(ImGuiMouseButton_Left, false) &&
                igIsMouseHoveringRect(imvec2f(x, y), imvec2f(x + wd, y + height), true))
            {
                it->state0 ^= ANIM_SEQ_STATE_EXPAND;
            }
            y += height + gap;
        }

        if (igGetCurrentWindow()->ID != self.id)
        {
            self.scroll.x = self.sidebarWidth + pad * 2 + 30;
        }
    }
    end();
    return true;
}