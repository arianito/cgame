#include "anim.h"
#include "anim_editor.h"

#include "gui/libgui.h"
#include "math/color.h"
#include "engine/game.h"
#include "engine/input.h"

#define COLOR_BG color_hex(color(0.1, 0.1, 0.1, 0.9))
#define COLOR_LINE color_hex(color(0.5, 0.5, 0.5, 0.9))

#define MODE_NONE 0
#define MODE_DRAG_POINTS 1
#define MODE_DRAG_TIME 2
#define MODE_DRAG_BEZIER 3
#define MODE_TEMP 99

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
    char dir;
    bool play;
    bool smooth;
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
        self.play = 0;
        self.smooth = 0;
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

    const float status_height = 32;
    const float header_height = 32;
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

    int nStepX = floof(maxf(self.scale.x, 1)) * 10;
    if (self.scale.x < 0.25)
        nStepX = 2;
    else if (self.scale.x < 1)
        nStepX = 4;

    int nStepY = floof(maxf(self.scale.y, 1)) * 10;
    if (self.scale.y < 0.25)
        nStepY = 2;
    else if (self.scale.y < 1)
        nStepY = 4;

    // draw grid
    {
        const int ED_COLOR_TEXT = (color_hex(color_alpha(color_white, 0.75)));
        const int ED_COLOR_LIGHT = (color_hex(color_alpha(color_white, 0.25)));
        const int ED_COLOR_DARK = (color_hex(color_alpha(color_white, 0.1)));
        const float a1 = 20;
        const float a2 = 10;
        const float pad = 4;

#define to_screen_x(ft) (offset.x + self.scroll.x + ((ft) * scalePx))
#define to_screen_y(ft) (offset.y + self.scroll.y + size.y / 2 - (ft * scalePy))

#define to_world_x(sc) (((sc)-self.scroll.x - offset.x) / scalePx)
#define to_world_y(sc) (((-sc) + offset.y + self.scroll.y + size.y / 2) / scalePy)

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
            for (int i = -n, k = 0; i <= n; i++, k += nStepX)
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

                ImDrawList_AddLine(
                    self.drawList,
                    imvec2f(ox, y + header_height + a2),
                    imvec2f(ox, y + size.y),
                    ED_COLOR_DARK, 1);

                if (self.scale.x > 0.5)
                {

                    float v = (i - psx);
                    int n = snprintf(buff, 10, "%.0f", v);
                    igCalcTextSize(&textSize, buff, buff + n, 0, 0);

                    ImDrawList_AddText_Vec2(
                        self.drawList,
                        imvec2f(ox - textSize.x / 2, y + a2 + pad),
                        ED_COLOR_TEXT, buff, buff + n);
                }
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
                if (self.scale.y > 0.5)
                {
                    if (self.i0 == -1)
                    {
                        float v = (psy - i);
                        int n = snprintf(buff, 10, "%.0f", v);
                        igCalcTextSize(&textSize, buff, buff + n, 0, 0);

                        ImDrawList_AddText_Vec2(
                            self.drawList,
                            imvec2f(x - a2 - textSize.x - pad, oy - textSize.y / 2),
                            ED_COLOR_TEXT, buff, buff + n);
                    }
                    else if (i == -1 || i == 1)
                    {

                        float oy = offset.y + size.y / 2 + self.scroll.y + (i * scalePy);
                        AnimSequence *seq = &context->anim->sequences->vector[self.i0];
                        float val = 0;
                        if (i == -1)
                            val = seq->max0;
                        else if (i == 1)
                            val = seq->min0;

                        int n = snprintf(buff, 10, "%.5f", val);
                        igCalcTextSize(&textSize, buff, buff + n, 0, 0);

                        ImDrawList_AddText_Vec2(
                            self.drawList,
                            imvec2f(x - a2 - textSize.x - pad, oy - textSize.y / 2),
                            ED_COLOR_TEXT, buff, buff + n);
                    }
                }
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
        float rads[] = {rad * 0.5, rad * 0.5, rad * 0.75};

#define convert(v) ({                                                  \
    if (normalize)                                                     \
        v.y = ((v.y - seq->min0) / (seq->max0 - seq->min0) - 0.5) * 2; \
    v.x = to_screen_x(v.x);                                            \
    v.y = to_screen_y(v.y);                                            \
})

        for (int i = 0; i < anim->sequences->length; i++)
        {
            AnimSequence *seq = &anim->sequences->vector[i];
            Color c = anim_property_colors[seq->type];
            c.alpha = 0.5;

            if (self.i0 == i)
                c.alpha = 1.0;
            if (self.mode == MODE_NONE)
            {
                seq->min0 = MAX_FLOAT;
                seq->max0 = MIN_FLOAT;
                for (int j = 0; j < seq->frames->length; j++)
                {
                    KeyFrame *kf = &seq->frames->vector[j];
                    seq->min0 = minf(kf->value, seq->min0);
                    seq->max0 = maxf(kf->value, seq->max0);
                }
            }

            KeyFrame pkf0 = {0, 0, {0, 0, seq->frames->vector[0].cubic[0], seq->frames->vector[0].cubic[1]}};
            for (int j = 0; j < seq->frames->length; j++)
            {
                KeyFrame *kf = &seq->frames->vector[j];
                self.maxDuration = maxf(self.maxDuration, kf->t);
                {
                    KeyFrame *pkf = j > 0 ? &seq->frames->vector[j - 1] : &pkf0;

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
                        {kf->t + kf->cubic[2], kf->value + kf->cubic[3]},
                        {kf->t, kf->value},
                    };
                    for (int k = 0; k < 3; k++)
                    {
                        convert(qs[k]);
                        float cof = minf(maxf(10 / vec2_distance(vec2im(mouse), vec2im(qs[k])), 1), 2);
                        if (self.i0 == i && j == self.j0 && k == self.k0)
                            ImDrawList_AddCircleFilled(self.drawList, qs[k], rads[k] * 2, color_hex(c), 4);
                        else if (self.i0 == i || k == 2)
                            ImDrawList_AddCircle(self.drawList, qs[k], rads[k] * cof, color_hex(c), 4, 1);
                    }

                    if (self.i0 == i)
                    {

                        ImDrawList_AddLine(self.drawList, qs[0], qs[2], color_hex(c), 1);
                        ImDrawList_AddLine(self.drawList, qs[1], qs[2], color_hex(c), 1);
                    }
                }
            }
        }

        {
            ImDrawList_AddRectFilled(self.drawList, imvec2f(offset.x, offset.y + size.y - status_height), imvec2f(offset.x + size.x, offset.y + size.y), COLOR_BG, 0, 0);
            if (self.i0 != -1 && self.j0 != -1 && self.k0 != -1)
            {
                AnimSequence *seq = &anim->sequences->vector[self.i0];
                KeyFrame *k = &seq->frames->vector[self.j0];

                float x, y;

                if (self.k0 == 0)
                {
                    x = k->t + k->cubic[0];
                    y = k->value + k->cubic[1];
                }
                else if (self.k0 == 1)
                {
                    x = k->t + k->cubic[2];
                    y = k->value + k->cubic[3];
                }
                else if (self.k0 == 2)
                {
                    x = k->t;
                    y = k->value;
                }

                char buff[100];
                ImVec2 textSize;
                int n = snprintf(buff, 100, "x: %.5f, y: %.5f", x, y);
                igCalcTextSize(&textSize, buff, buff + n, 0, 0);
                ImDrawList_AddText_Vec2(
                    self.drawList,
                    imvec2f(offset.x + size.x - textSize.x - 24, offset.y + size.y - status_height / 2 - textSize.y / 2),
                    COLOR_LINE, buff, buff + n);
            }
            {
                float pad = 4;
                float tri = 3;
                float x = offset.x;
                float y = offset.y + size.y - status_height;
                float r = status_height / 2 - pad;
                // playback
                Color c = color_gray;
                Color c2 = color_white;

                ImVec2 cen = {x + r + pad, y + status_height / 2};
                ImDrawList_AddCircleFilled(self.drawList, cen, r, color_hex(c), 16);
                ImDrawList_AddTriangleFilled(self.drawList, imvec2f(cen.x - tri, cen.y - tri), imvec2f(cen.x - tri, cen.y + tri), imvec2f(cen.x + tri, cen.y), color_hex(c2));

                if ((igIsMouseClicked_Bool(ImGuiMouseButton_Left, 0) &&
                     vec2_distance(vec2im(mouse), vec2im(cen)) <= r))
                {
                    self.play ^= 1;
                    self.mode = MODE_TEMP;
                }
                if (igIsKeyPressed_Bool(ImGuiKey_Space, 0))
                {
                    self.play ^= 1;
                }
            }
        }
        if (igIsMouseDoubleClicked_Nil(ImGuiMouseButton_Left) && self.i0  != 0)
        {
            self.mode == MODE_TEMP;
        }
        if (self.mode == MODE_NONE && igIsMouseClicked_Bool(ImGuiMouseButton_Left, false))
        {
            bool brk = false;
            for (int i = 0; i < anim->sequences->length && !brk; i++)
            {
                AnimSequence *seq = &anim->sequences->vector[i];
                for (int j = 0; j < seq->frames->length && !brk; j++)
                {
                    KeyFrame *kf = &seq->frames->vector[j];
                    bool skip = (near0f(kf->cubic[0]) && near0f(kf->cubic[1]) && near0f(kf->cubic[2]) && near0f(kf->cubic[3]));
                    ImVec2 qs[3] = {
                        {kf->t + kf->cubic[0], kf->value + kf->cubic[1]},
                        {kf->t + kf->cubic[2], kf->value + kf->cubic[3]},
                        {kf->t, kf->value},
                    };

                    for (int k = skip ? 2 : 0; k < 3 && !brk; k++)
                    {
                        convert(qs[k]);
                        float cof = minf(maxf(50 / vec2_distance(vec2im(mouse), vec2im(qs[k])), 1), 2);
                        if (vec2_distance(vec2im(mouse), vec2im(qs[k])) <= (rads[k] * cof))
                        {
                            if (k != 2 && self.i0 != i)
                                continue;
                            self.mode = MODE_TEMP;
                            if (!(self.i0 == i &&
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
            if (!brk)
            {
                // self.i0 = -1;
                self.j0 = -1;
                self.k0 = -1;
            }
        }
        else if (self.mode == MODE_NONE && igIsWindowHovered(0) && igIsMouseDragging(ImGuiMouseButton_Left, 1.0 / nStepX))
        {
            self.mode = MODE_DRAG_TIME;
        }
        if (self.i0 != -1 && self.j0 != -1 && self.k0 != -1 && self.mode == MODE_NONE)
        {
            bool s_pressed = igIsKeyPressed_Bool(ImGuiKey_S, false);
            bool g_pressed = igIsKeyPressed_Bool(ImGuiKey_G, false);

            if (s_pressed || g_pressed)
            {

                AnimSequence *seq = &anim->sequences->vector[self.i0];
                KeyFrame *kf = &seq->frames->vector[self.j0];
                self.min0 = seq->min0;
                self.max0 = seq->max0;
                self.mode = MODE_DRAG_POINTS;
                self.prevMouse = mouse;
                self.dir = 'a';
                self.prevValue[0] = kf->cubic[0];
                self.prevValue[1] = kf->cubic[1];
                self.prevValue[2] = kf->cubic[2];
                self.prevValue[3] = kf->cubic[3];
                self.prevValue[4] = kf->t;
                self.prevValue[5] = kf->value;
                self.smooth = 0;
            }

            if (s_pressed)
            {
                self.k0 = 1;
                self.smooth = 1;
            }
        }

        bool should_release = igIsMouseReleased_Nil(ImGuiMouseButton_Left) || igIsKeyPressed_Bool(ImGuiKey_Enter, 0);

        if (self.mode == MODE_DRAG_TIME)
        {

            context->time = (mouse.x - self.scroll.x) / scalePx;

            if (igIsKeyDown_Nil(ImGuiKey_LeftCtrl))
            {
                context->time = snapf(context->time, 1.0 / nStepX);
            }

            for (int i = 0; i < context->anim->sequences->length; i++)
            {
                AnimSequence *it = &context->anim->sequences->vector[i];
                KeyFrame *k = anim_find(it, context->time, 0.5 / nStepX);
                if (k != NULL)
                {
                    context->time = k->t;
                    break;
                }
            }

            if (should_release)
            {
                self.mode = MODE_NONE;
            }
        }
        else if (self.mode == MODE_DRAG_POINTS)
        {
            AnimSequence *seq = &anim->sequences->vector[self.i0];
            KeyFrame *k = &seq->frames->vector[self.j0];
            float *x = NULL, *y = NULL;
            float *x1 = NULL, *y1 = NULL;

            float px = 0, py = 0;
            float px1 = 0, py1 = 0;
            bool sym = 1;
            bool alg = 0;

            float ff = 0;

            if (igIsKeyPressed_Bool(ImGuiKey_X, false))
                self.dir = self.dir != 'x' ? 'x' : 'a';
            if (igIsKeyPressed_Bool(ImGuiKey_Y, false))
                self.dir = self.dir != 'y' ? 'y' : 'a';

            if (igIsKeyDown_Nil(ImGuiKey_LeftAlt))
                sym = 0;
            if (igIsKeyDown_Nil(ImGuiKey_LeftShift))
                alg = 1;

            if (self.smooth)
            {
                sym = 1;
                alg = 1;
            }
            if (self.k0 == 0)
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
            else if (self.k0 == 1)
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
            else if (self.k0 == 2)
            {
                x = &k->t;
                y = &k->value;
                px = self.prevValue[4];
                py = self.prevValue[5];
            }

            float dx = to_world_x(mouse.x);
            float dy = to_world_y(mouse.y);

            if (igIsKeyDown_Nil(ImGuiKey_LeftCtrl))
            {
                dx = snapf(dx, 0.5 / nStepX);
                dy = snapf(dy, 1.0 / nStepY);

                for (int i = 0; i < context->anim->sequences->length; i++)
                {
                    if (i == self.i0)
                        continue;
                    AnimSequence *it = &context->anim->sequences->vector[i];
                    KeyFrame *k = anim_find(it, dx, 0.5 / nStepX);
                    if (k != NULL)
                    {
                        dx = k->t;
                        break;
                    }
                }
            }

            if (normalize)
            {
                dy = ((dy / 2) + 0.5) * (self.max0 - self.min0) + self.min0;
            }

            if (self.prevMouse.x != mouse.x && self.prevMouse.y != mouse.y)
            {
                *x = dx;
                *y = dy;

                if (self.k0 != 2)
                {
                    *x -= k->t;
                    *y -= k->value;
                }

                if (self.dir == 'x')
                {
                    *y = py;
                }
                else if (self.dir == 'y')
                {
                    *x = px;
                }
            }
            float fy = 1.0 / nStepY;
            float fx = 1.0 / nStepX;
            if (igIsKeyDown_Nil(ImGuiKey_LeftShift))
            {
                fy *= 10;
                fx *= 10;
            }
            if (igIsKeyPressed_Bool(ImGuiKey_UpArrow, 1))
            {
                *y = snapf(*y, fy);
                *y += fy;
            }
            else if (igIsKeyPressed_Bool(ImGuiKey_DownArrow, 1))
            {
                *y = snapf(*y, fy);
                *y -= fy;
            }

            if (igIsKeyPressed_Bool(ImGuiKey_RightArrow, 1))
            {
                *y = snapf(*y, fx);
                *x += fx;
            }
            else if (igIsKeyPressed_Bool(ImGuiKey_LeftArrow, 1))
            {
                *x = snapf(*x, fx);
                *x -= fx;
            }

            ImVec2 cp = {*x, *y};
            if (self.k0 != 2)
            {
                cp.x += k->t;
                cp.y += k->value;
            }
            convert(cp);

            ImDrawList_AddLine(self.drawList, imvec2f(offset.x, cp.y), imvec2f(offset.x + size.x, cp.y), COLOR_LINE, 1);
            ImDrawList_AddLine(self.drawList, imvec2f(cp.x, offset.y), imvec2f(cp.x, offset.y + size.y), COLOR_LINE, 1);

            if (self.k0 != 2)
            {
                if (sym)
                {
                    if (alg)
                    {
                        *x1 = -*x;
                        *y1 = -*y;
                    }
                    else
                    {
                        float xx = *x;
                        float yy = *y;

                        Vec2 v = vec2_rotate(vec2(xx, yy), 180);
                        float sc = sqrf(px1 * px1 + py1 * py1) / sqrf(px * px + py * py);
                        *x1 = v.x * sc;
                        *y1 = v.y * sc;
                    }
                }
                else
                {
                    *x1 = px1;
                    *y1 = py1;
                }
            }

            if (igIsKeyPressed_Bool(ImGuiKey_Escape, 0))
            {
                *x = px;
                *y = py;
                if (x1 != NULL)
                {
                    *x1 = px1;
                    *y1 = py1;
                }
                self.mode = MODE_NONE;
            }
            if (should_release)
            {
                self.maxDuration = MIN_FLOAT;
                for (int i = 0; i < anim->sequences->length; i++)
                {
                    AnimSequence *seq = &anim->sequences->vector[i];
                    for (int j = 0; j < seq->frames->length; j++)
                    {
                        KeyFrame *kf = &seq->frames->vector[j];
                        self.maxDuration = maxf(self.maxDuration, kf->t);
                    }
                }
                self.mode = MODE_NONE;
            }
        }
        else if (self.mode == MODE_TEMP)
        {
            if (should_release)
            {
                self.mode = MODE_NONE;
            }
        }
    }
    // playback
    {
        if (self.play && self.mode != MODE_DRAG_TIME)
        {
            context->time += gtime->delta;
            if (context->time > self.maxDuration)
                context->time = 0;
        }
    }

    // release clip
    igPopClipRect();

    end();
    return true;
}
