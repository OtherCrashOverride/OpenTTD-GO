#include "../stdafx.h"
#include "../openttd.h"
#include "../gfx_func.h"
#include "../blitter/factory.hpp"
#include "ogs_v.h"

#include "../safeguards.h"

#include <go2/display.h>
#include <go2/input.h>
#include <drm/drm_fourcc.h>

static go2_display_t* display;
static go2_presenter_t* presenter;
static go2_surface_t* surface;
static go2_input_t* go2input;
static go2_gamepad_state_t gamepadState;
static go2_gamepad_state_t previousState;
static int mouse_x;
static int mouse_y;

/** Factory for the null video driver. */
static FVideoDriver_OGS iFVideoDriver_OGS;


void VideoDriver_OGS::PollEvent()
{
    go2_input_gamepad_read(go2input, &gamepadState);

    if (gamepadState.buttons.f1)
    {
        HandleExitGameRequest();
    }


    int dw = go2_display_height_get(display);
    int dh = go2_display_width_get(display);

    if (gamepadState.dpad.left)
    {
        --mouse_x;
        if (mouse_x < 0) mouse_x = 0;
    }
    if (gamepadState.dpad.right)
    {
        ++mouse_x;
        if (mouse_x > dw) mouse_x = dw;
    }

    if (gamepadState.dpad.up)
    {
        --mouse_y;
        if (mouse_y < 0) mouse_y = 0;
    }
    if (gamepadState.dpad.down)
    {
        ++mouse_y;
        if (mouse_y > dh) mouse_y = dh;
    }


    const float TRIM = 0.35f;
    const float ACCEL = 5.0f;

    if (gamepadState.thumb.x < -TRIM ||
        gamepadState.thumb.x > TRIM)
    {
        mouse_x += gamepadState.thumb.x * ACCEL;
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_x > dw) mouse_x = dw;
    }
    if (gamepadState.thumb.y < -TRIM ||
        gamepadState.thumb.y > TRIM)
    {
        mouse_y += gamepadState.thumb.y * ACCEL;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_y > dh) mouse_y = dh;
    }


    if (!previousState.buttons.a && gamepadState.buttons.a) // pressed
    {
        _left_button_down = true;
    }
    else if (previousState.buttons.a && !gamepadState.buttons.a) // released
    {
        _left_button_down = false;
        _left_button_clicked = false;
    }
    
    if (!previousState.buttons.b && gamepadState.buttons.b) // pressed
    {
        _right_button_down = true;
        _right_button_clicked = true;
    }
    else if (previousState.buttons.b && !gamepadState.buttons.b) // released
    {
        _right_button_down = false;
    }
    

    _cursor.UpdateCursorPosition(mouse_x, mouse_y, false);
    HandleMouseEvents();

    previousState = gamepadState;
}

const char *VideoDriver_OGS::Start(const char * const *param)
{
    display = go2_display_create();
    int dw = go2_display_height_get(display);
    int dh = go2_display_width_get(display);

    _num_resolutions = 1;
    _resolutions[0].width = dw;
    _resolutions[0].height = dh;

    go2input = go2_input_create();

    int bpp = BlitterFactory::GetCurrentBlitter()->GetScreenDepth();
    printf("OGS: w=%d, h=%d, bpp=%d\n", dw, dh, bpp);

    uint32_t drmfmt = 0;

    switch(bpp)
    {
        case 8:
            return "OGS: Unsupported bpp (8).";
            break;

        case 16:
            drmfmt = DRM_FORMAT_RGB565;
            break;

        case 32:
            drmfmt = DRM_FORMAT_XRGB8888;
            break;
    }

    presenter = go2_presenter_create(display, DRM_FORMAT_RGB565, 0xff080808);
    surface = go2_surface_create(display, dw, dh, drmfmt);
    
	_screen.width = dw;
	_screen.height = dh;
	_screen.pitch = go2_surface_stride_get(surface) / (bpp / 8);
	_screen.dst_ptr = go2_surface_map(surface);
    _fullscreen = true;

    ScreenSizeChanged();

    memset(_screen.dst_ptr, 0xff, _screen.height * go2_surface_stride_get(surface));

    Blitter *blitter = BlitterFactory::GetCurrentBlitter();
	blitter->PostResize();


	GameSizeChanged();
    MarkWholeScreenDirty();

    mouse_x = dw / 2;
    mouse_y = dh / 2;

    _cursor.pos.x = mouse_x;
	_cursor.pos.y = mouse_y;
    HandleMouseEvents();

	return nullptr;
}

void VideoDriver_OGS::Stop()
{

}

void VideoDriver_OGS::MakeDirty(int left, int top, int width, int height)
{
    //printf("OGS: MakeDirty - l=%d, t=%d, w=%d, h=%d\n", left, top, width, height);
}

void VideoDriver_OGS::MainLoop()
{
    int dw = go2_display_height_get(display);
    int dh = go2_display_width_get(display);

	while(!_exit_game) {
        PollEvent();
		GameLoop();
		UpdateWindows();

        go2_presenter_post(presenter,
                           surface,
                           0, 0, dw, dh,
                           0, 0, dh, dw,
                           GO2_ROTATION_DEGREES_270);   
	}
}

bool VideoDriver_OGS::ChangeResolution(int w, int h)
{
    printf("OGS: ChangeResolution - w=%d, h=%d\n", w, h);
    return true;
}

bool VideoDriver_OGS::ToggleFullscreen(bool fs)
{
    printf("OGS: ToggleFullscreen - fs=%s\n", fs ? "true" : "false");
    return true;
}
