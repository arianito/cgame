#include "anim.h"
#include "anim_editor.h"

#include "gui/libgui.h"
#include "math/color.h"
#include "engine/game.h"
#include "engine/input.h"

#define COLOR_BG color_hex(color(0.1, 0.1, 0.1, 1.0))

#define MODE_NONE 0
#define MODE_DRAG_POINTS 1

typedef struct
{
    ImGuiID id;
    ImDrawList *drawList;
    ImVec2 scroll;
    ImVec2 scale;
    struct
    {
        bool dragging;
        ImVec2 lastScroll;
        ImVec2 lastMouse;
    } dragData;
    //
    float maxDuration;
    //
    int i0;
    int j0;
    int k0;
    float min0;
    float max0;
    int mode;
    ImVec2 prevMouse;
    float prevValue[6];
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

    igPushStyleColor_U32(ImGuiCol_WindowBg, COLOR_BG);
    igPushStyleColor_U32(ImGuiCol_Border, 0);
    igPushStyleVar_Float(ImGuiStyleVar_WindowRounding, 0.0f);

    igBegin(name, NULL, flags);
    if (igGetCurrentWindow()->ID != self.id)
    {
        self = (AnimContext){0};
        self.drawList = igGetWindowDrawList();
        self.scroll = imvec2f(100, 0);
        self.scale = imvec2f(1, 1);
        self.dragData.dragging = false;
        self.mode = 0;
        self.i0 = -1;
        self.j0 = -1;
        self.k0 = -1;
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
    float fontHeight = igGetTextLineHeight();

    // begin
    begin(name, height);
    ImVec2 offset;
    igGetWindowPos(&offset);
    ImVec2 size;
    igGetWindowSize(&size);
    ImVec2 mouse;
    igGetMousePos(&mouse);

    // handle clip
    igPushClipRect(offset, imvec2f(offset.x + size.x, offset.y + size.y), false);

    // handle zoom
    if (input->wheel.y != 0 && igIsWindowHovered(0))
    {
        if (!igIsKeyDown_Nil(ImGuiKey_LeftCtrl))
        {
            float pz = self.scale.x;
            self.scale.x *= 1 + (input->wheel.y * gtime->delta * 4.0);
            float ofc = self.scale.x - pz;
            float m = (mouse.x - self.scroll.x) / self.scale.x;
            self.scroll.x -= m * ofc;
        }
        else
        {
            float pz = self.scale.y;
            self.scale.y *= 1 + (input->wheel.y * gtime->delta * 4.0);
            float ofc = self.scale.y - pz;
            float m = (mouse.y - self.scroll.y - offset.y - size.y / 2) / self.scale.y;
            self.scroll.y -= m * ofc;
        }
    }

    // handle pan
    if (igIsWindowHovered(0) && !self.dragData.dragging && igIsMouseDown_Nil(ImGuiMouseButton_Middle))
    {
        self.dragData.dragging = true;
        self.dragData.lastScroll = self.scroll;
        self.dragData.lastMouse = mouse;
    }
    if (self.dragData.dragging && igIsMouseDown_Nil(ImGuiMouseButton_Middle))
    {
        self.scroll.x = self.dragData.lastScroll.x + (mouse.x - self.dragData.lastMouse.x);
        self.scroll.y = self.dragData.lastScroll.y + (mouse.y - self.dragData.lastMouse.y);
    }
    if (igIsMouseReleased_Nil(ImGuiMouseButton_Middle))
    {
        self.dragData.dragging = false;
    }

    float scalePx = UNIT * self.scale.x;
    float scalePy = UNIT * self.scale.y;
    int nStepX = self.scale.x < 1 ? 4 : floof(maxf(self.scale.x, 1)) * 10;
    int nStepY = self.scale.y < 1 ? 4 : floof(maxf(self.scale.y, 1)) * 10;

    // draw grid
    {
        const int ED_COLOR_TEXT = (color_hex(color_alpha(color_white, 0.75)));
        const int ED_COLOR_LIGHT = (color_hex(color_alpha(color_white, 0.25)));
        const int ED_COLOR_DARK = (color_hex(color_alpha(color_white, 0.1)));
        const float a1 = 20;
        const float a2 = 10;
        const float pad = 4;

#define to_screen_x(ft) (offset.x + self.scroll.x + ((ft) * scalePx))
#define to_screen_y(ft) (offset.y + size.y / 2 + self.scroll.y - (ft * scalePy))

        // draw loop
        {
            float x = offset.x + self.scroll.x;
            float y = offset.y;

            ImDrawList_AddRectFilled(
                self.drawList,
                imvec2f(x, y),
                imvec2f(x + (self.maxDuration * scalePx), y + size.y),
                color_hex(color_alpha(color_blue, 0.06)),
                0,
                0);
        }
        // draw horizontal
        {
            char buff[10];
            ImVec2 textSize;
            float x = offset.x;
            float y = offset.y;

            float ssx = repeatf(self.scroll.x, scalePx);
            float psx = floof(self.scroll.x / scalePx);
            int n = size.x / scalePx + 1; //(size.x / scalePx + 1);
            for (int i = 0, k = 0; i <= n; i++, k += nStepX)
            {

                float ox = offset.x + ssx + (i * scalePx);

                for (int j = 1; j < nStepX; j++)
                {
                    float h = j % 2 == 0 ? a1 : a2;
                    float ox2 = ox + j * (scalePx / (float)(nStepX));
                    ImDrawList_AddLine(
                        self.drawList,
                        imvec2f(ox2, y),
                        imvec2f(ox2, y + h),
                        ED_COLOR_DARK, 2);
                }

                ImDrawList_AddLine(self.drawList, imvec2f(ox, y), imvec2f(ox, y + a2), ED_COLOR_LIGHT, 2);

                float v = (i - psx);
                int n = snprintf(buff, 10, "%.0f", v);
                igCalcTextSize(&textSize, buff, buff + n, 0, 0);

                ImDrawList_AddLine(
                    self.drawList,
                    imvec2f(ox, y + header_height + a2),
                    imvec2f(ox, y + size.y),
                    ED_COLOR_DARK, 1);

                ImDrawList_AddText_Vec2(
                    self.drawList,
                    imvec2f(ox - textSize.x / 2, y + a2 + pad),
                    ED_COLOR_TEXT, buff, buff + n);
            }
        }
        // draw vertical
        {
            char buff[10];
            ImVec2 textSize;
            float x = offset.x + size.x;
            float y = offset.y;
            float ssy = repeatf(self.scroll.y, scalePy);
            float psy = floof(self.scroll.y / scalePy);
            int n = (size.y / scalePy + 1);

            for (int i = -n; i <= n; i++)
            {
                float oy = offset.y + size.y / 2 + ssy + (i * scalePy);
                for (int j = 1; j < nStepY; j++)
                {
                    float w = j % 2 == 0 ? a1 : a2;
                    float oy2 = oy + j * (scalePy / (float)(nStepY));
                    ImDrawList_AddLine(self.drawList, imvec2f(x - w, oy2), imvec2f(x, oy2), ED_COLOR_DARK, 2);
                }
                ImDrawList_AddLine(self.drawList, imvec2f(x - a2, oy), imvec2f(x, oy), ED_COLOR_LIGHT, 2);

                float v = (psy - i);
                int n = snprintf(buff, 10, "%.0f", v);
                igCalcTextSize(&textSize, buff, buff + n, 0, 0);

                ImDrawList_AddText_Vec2(
                    self.drawList,
                    imvec2f(x - a2 - textSize.x - pad, oy - textSize.y / 2),
                    ED_COLOR_TEXT, buff, buff + n);
            }
        }
    }

    // draw cursor
    {
        const int ED_COLOR_CURSOR = (color_hex(color_red));

        float x = offset.x;
        float y = offset.y;
        float ox = to_screen_x(context->time);

        ImDrawList_AddLine(self.drawList, imvec2f(ox, y), imvec2f(ox, y + size.y), ED_COLOR_CURSOR, 4);
    }

    // render
    {
        bool normalize = 1;

        Anim *anim = context->anim;
        float rad = 8;
        float rads[] = {rad / 2, rad, rad / 2};

#define convert(v) ({                                                  \
    if (normalize)                                                     \
        v.y = ((v.y - seq->min0) / (seq->max0 - seq->min0) - 0.5) * 2; \
    v.x = to_screen_x(v.x);                                            \
    v.y = to_screen_y(v.y);                                            \
})
        for (int i = 0; i < anim->length; i++)
        {
            AnimSequence *seq = &anim->data[i];
            Color c = anim_property_colors[seq->type];
            c.alpha = 0.5;

            if (self.i0 == i)
                c.alpha = 1.0;
            if (self.mode == MODE_NONE)
            {
                seq->min0 = MAX_FLOAT;
                seq->max0 = MIN_FLOAT;
                for (int j = 0; j < seq->length; j++)
                {
                    KeyFrame *kf = &seq->frames[j];
                    seq->min0 = minf(kf->value, seq->min0);
                    seq->max0 = maxf(kf->value, seq->max0);
                }
            }

            KeyFrame pkf0 = {0, 0, {0, 0, seq->frames[0].cubic[0], seq->frames[0].cubic[1]}};
            for (int j = 0; j < seq->length; j++)
            {
                KeyFrame *kf = &seq->frames[j];
                self.maxDuration = maxf(self.maxDuration, kf->t);
                {
                    KeyFrame *pkf = j > 0 ? &seq->frames[j - 1] : &pkf0;

                    ImVec2 qs[4];
                    anim_control_points(pkf, kf, qs);

                    for (int k = 0; k < 4; k++)
                        convert(qs[k]);

                    if (j > 0)
                        ImDrawList_AddBezierCubic(self.drawList, qs[0], qs[1], qs[2], qs[3], color_hex(c), 1, 16);
                }
                {
                    ImVec2 qs[3] = {
                        {kf->t + kf->cubic[0], kf->value + kf->cubic[1]},
                        {kf->t, kf->value},
                        {kf->t + kf->cubic[2], kf->value + kf->cubic[3]},
                    };

                    for (int k = 0; k < 3; k++)
                    {
                        convert(qs[k]);

                        if (i == self.i0 && j == self.j0 && k == self.k0)
                            ImDrawList_AddCircleFilled(self.drawList, qs[k], rads[k], color_hex(c), 4);
                        else
                            ImDrawList_AddCircle(self.drawList, qs[k], rads[k], color_hex(c), 4, 1);
                    }

                    ImDrawList_AddLine(self.drawList, qs[0], qs[1], color_hex(c), 1);
                    ImDrawList_AddLine(self.drawList, qs[1], qs[2], color_hex(c), 1);
                }
            }
        }

        if (self.mode == MODE_NONE && igIsMouseClicked_Bool(ImGuiMouseButton_Left, false))
        {
            bool brk = false;
            for (int i = 0; i < anim->length && !brk; i++)
            {
                AnimSequence *seq = &anim->data[i];
                for (int j = 0; j < seq->length && !brk; j++)
                {
                    KeyFrame *kf = &seq->frames[j];
                    ImVec2 qs[3] = {
                        {kf->t + kf->cubic[0], kf->value + kf->cubic[1]},
                        {kf->t, kf->value},
                        {kf->t + kf->cubic[2], kf->value + kf->cubic[3]},
                    };

                    for (int k = 0; k < 3 && !brk; k++)
                    {
                        convert(qs[k]);
                        if (vec2_distance(vec2im(mouse), vec2im(qs[k])) <= (rads[k] * 2) &&
                            !(self.i0 == i &&
                              self.j0 == j &&
                              self.k0 == k))
                        {
                            self.i0 = i;
                            self.j0 = j;
                            self.k0 = k;

                            brk = true;
                            break;
                        }
                    }
                }
            }
        }

        if (self.mode == MODE_NONE && igIsKeyPressed_Bool(ImGuiKey_G, false))
        {

            AnimSequence *seq = &anim->data[self.i0];
            KeyFrame *kf = &seq->frames[self.j0];

            self.min0 = seq->min0;
            self.max0 = seq->max0;

            self.mode = MODE_DRAG_POINTS;
            self.prevMouse = mouse;

            self.prevValue[0] = kf->cubic[0];
            self.prevValue[1] = kf->cubic[1];
            self.prevValue[2] = kf->cubic[2];
            self.prevValue[3] = kf->cubic[3];

            self.prevValue[4] = kf->t;
            self.prevValue[5] = kf->value;
        }

        if (self.mode == MODE_DRAG_POINTS)
        {
            float d = normalize ? ((self.max0 - self.min0) / 2) : 1;
            float diff = (mouse.y - self.prevMouse.y) / scalePy;
            AnimSequence *seq = &anim->data[self.i0];
            KeyFrame *k = &seq->frames[self.j0];
            float *x = NULL, *y = NULL;
            float *x1 = NULL, *y1 = NULL;

            float px = 0, py = 0;
            float px1 = 0, py1 = 0;
            bool sym = 1;

            if (igIsKeyDown_Nil(ImGuiKey_LeftAlt))
                sym = 0;

            if (self.k0 == 1)
            {
                x = &k->t;
                y = &k->value;
                x1 = &k->t;
                y1 = &k->value;
                px = self.prevValue[4];
                py = self.prevValue[5];
            }
            else if (self.k0 == 0)
            {
                x = &k->cubic[0];
                y = &k->cubic[1];
                px = self.prevValue[0];
                py = self.prevValue[1];

                x1 = &k->cubic[2];
                y1 = &k->cubic[3];
                px1 = self.prevValue[2];
                py1 = self.prevValue[3];
            }
            else if (self.k0 == 2)
            {
                x = &k->cubic[2];
                y = &k->cubic[3];
                px = self.prevValue[2];
                py = self.prevValue[3];

                x1 = &k->cubic[0];
                y1 = &k->cubic[1];
                px1 = self.prevValue[0];
                py1 = self.prevValue[1];
            }

            *x = px + (mouse.x - self.prevMouse.x) / scalePx;
            *y = py - diff * d;

            if (sym)
            {
                *x1 = -*x;
                *y1 = -*y;
            }
            else
            {
                *x1 = px1;
                *y1 = py1;
            }

            // input_infinite_y();
            if (igIsMouseReleased_Nil(ImGuiMouseButton_Left))
            {
                // self.maxDuration = MIN_FLOAT;
                self.mode = MODE_NONE;
            }
        }
    }
    // playback
    {
        context->time += gtime->delta;
        if (context->time > self.maxDuration)
            context->time = 0;
    }

    // release clip
    igPopClipRect();

    end();
    return true;
}

/*




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
        float rr = 10;

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
                    maxDuration = maxf(maxDuration, kf->t);
                }
                float dts = absf(max - min);
                // draw
                {

                    float yval = anim_iterpolate(it, context->time);
                    float oy = (y + height / 2) - (((yval - min) / dts - 0.5f) * gh);

                    ImVec2 sz;
                    char buff[20];
                    int n = snprintf(buff, 20, "%.2f", yval);
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
                    if (it == self.active && j == self.i0)
                    {
                        ImDrawList_AddCircle(self.drawList, pt, rad * 2, ED_COLOR_RED, 8, 4);
                    }

                    if (self.mode == 0 &&
                        igIsMouseHoveringRect(imvec2f(pt.x - rad, pt.y - rad), imvec2f(pt.x + rad, pt.y + rad), true) &&
                        igIsMouseDown_Nil(ImGuiMouseButton_Left))
                    {
                        self.mode = 1;
                        self.i0 = j;
                        self.active = it;
                        self.prevMouse = mouse;
                        self.prevValue = imvec2f(kf->t, kf->value);
                    }
                    if (j > 0)
                    {
                        KeyFrame *pkf = &it->frames[j - 1];

                        ImVec2 qs[4];
                        anim_control_points(pkf, kf, qs);

                        ImVec2 a1 = qs[1];
                        ImVec2 a2 = qs[2];
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

                        if (self.mode == 0 && igIsMouseDown_Nil(ImGuiMouseButton_Left))
                        {

                            float jmpX = kf->t - pkf->t;
                            float jmpY = kf->value - pkf->value;
                            self.jmpX = jmpX;
                            self.jmpY = jmpY;
                            if (igIsMouseHoveringRect(imvec2f(qs[1].x - rr, qs[1].y - rr), imvec2f(qs[1].x + rr, qs[1].y + rr), true))
                            {

                                self.mode = 4;
                                self.i0 = j - 1;
                                self.active = it;

                                self.prevMouse = mouse;
                                self.prevValue = a1;
                            }
                            else if (igIsMouseHoveringRect(imvec2f(qs[2].x - rr, qs[2].y - rr), imvec2f(qs[2].x + rr, qs[2].y + rr), true))
                            {
                                self.mode = 5;
                                self.i0 = j;
                                self.active = it;
                                self.prevMouse = mouse;
                                self.prevValue = a2;
                            }
                        }
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
                    if (it == self.active && j == self.i0)
                    {
                        ImDrawList_AddCircle(self.drawList, imvec2f(ox, y + height / 2), rad * 2, ED_COLOR_RED, 8, 4);
                    }
                    maxDuration = maxf(maxDuration, kf->t);

                    if (self.mode == 0 &&
                        igIsMouseHoveringRect(imvec2f(ox - rr, y + height / 2 - rr), imvec2f(ox + rr, y + height / 2 + rr), true) &&
                        igIsMouseDown_Nil(ImGuiMouseButton_Left))
                    {
                        self.mode = 1;
                        self.i0 = j;
                        self.active = it;

                        self.prevMouse = mouse;
                        self.prevValue = imvec2f(kf->t, kf->value);
                    }
                }
            }

            // seq
            bool cls = self.scroll.x >= sidebar_width;
            float wd = cls ? sidebar_width : (fontHeight + pad);
            ImDrawList_AddRectFilled(self.drawList, imvec2f(x, y), imvec2f(x + wd, y + height), ED_COLOR_PILL, 0, 0);
            ImDrawList_AddText_Vec2(self.drawList, imvec2f(x + pad, y + height / 2 - fontHeight / 2), ED_COLOR_TEXT,
                                    cstr(it->name), cls ? cend(it->name) : (cstr(it->name) + 1));

            if (self.mode == 0 && igIsMouseClicked_Bool(ImGuiMouseButton_Left, false) && igIsMouseHoveringRect(imvec2f(x, y), imvec2f(x + wd, y + height), true))
            {
                it->state0 ^= ANIM_SEQ_STATE_EXPAND;
                self.mode = 3;
            }
            y += height + gap;
        }

        if (igGetCurrentWindow()->ID != self.id)
        {
            self.scroll.x = self.sidebarWidth + pad * 2 + 30;
        }
    }
    {

        const int ED_COLOR_RED = (color_hex(color_alpha(color_red, 0.25)));
        float x = offset.x + self.scroll.x;
        float y = offset.y;

        ImDrawList_AddRectFilled(self.drawList, imvec2f(x, y), imvec2f(x + (maxDuration * scalePx), y + header_height), ED_COLOR_RED, 0, 0);
    }
    if (self.mode == 1)
    {
        int nStepY = self.active->scale0 < 1 ? 4 : floof(maxf(self.active->scale0, 1)) * 10;
        KeyFrame *it = &self.active->frames[self.i0];
        it->t = self.prevValue.x + ((mouse.x - self.prevMouse.x)) / scalePx;
        it->value -= input->delta.y * gtime->delta * 20;

        input_infinite_y();
        if (igIsKeyDown_Nil(ImGuiKey_LeftCtrl))
        {
            it->t = snapf(it->t, 1.0 / nStepX);
        }
        if (self.i0 < self.active->length - 1)
        {
            KeyFrame *nx = &self.active->frames[self.i0 + 1];
            it->t = minf(it->t, nx->t);
        }
        if (self.i0 > 0)
        {
            KeyFrame *pv = &self.active->frames[self.i0 - 1];
            it->t = maxf(it->t, pv->t);
        }

        if (igIsMouseReleased_Nil(ImGuiMouseButton_Left))
        {
            self.mode = 0;
        }
    }

    // jump time
    if (self.mode == 0 && igIsWindowHovered(0) && igIsMouseDragging(ImGuiMouseButton_Left, 1))
    {
        self.mode = 2;
    }

    if (self.mode == 2)
    {

        context->time = (mouse.x - self.scroll.x) / scalePx;

        for (int i = 0; i < context->anim->length; i++)
        {
            AnimSequence *it = &context->anim->data[i];
            KeyFrame *k = anim_find(it, context->time, 1.0 / nStepX);
            if (k != NULL)
            {
                context->time = k->t;
                break;
            }
        }

        if (igIsKeyDown_Nil(ImGuiKey_LeftCtrl))
        {
            context->time = snapf(context->time, 1.0 / nStepX);
        }

        // context->time = maxf(context->time, 0);
        if (igIsMouseReleased_Nil(ImGuiMouseButton_Left))
        {
            self.mode = 0;
        }
    }
    if (self.mode == 4)
    {
        float mx = (mouse.x - self.scroll.x) / scalePx;
        float my = (mouse.y - self.scroll.y) / self.active->scale0;
        if (igIsKeyDown_Nil(ImGuiKey_LeftCtrl))
        {
            mx = snapf(mx, 1.0 / nStepX);
            my = snapf(my, 1.0 / nStepX);
        }
        KeyFrame *it = &self.active->frames[self.i0];


        mx = maxf(mx, it->t);

        it->cubic[2] = 1 - (mx - it->t) / self.jmpX;

        if (igIsMouseReleased_Nil(ImGuiMouseButton_Left))
        {
            self.mode = 0;
        }
    }
    if (self.mode == 5)
    {
        // float mx = (mouse.x - self.scroll.x) / scalePx;
        // if (igIsKeyDown_Nil(ImGuiKey_LeftCtrl))
        // {
        //     mx = snapf(mx, 1.0 / nStepX);
        // }
        // self.selected->cubic[0] = -(mx - self.selected->t) / self.jmpX;
        if (igIsMouseReleased_Nil(ImGuiMouseButton_Left))
        {
            self.mode = 0;
        }
    }

    if (self.mode == 3)
    {
        self.mode = 0;
    }
*/