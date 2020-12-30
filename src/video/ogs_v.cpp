#include "../stdafx.h"
#include "../openttd.h"
#include "../gfx_func.h"
#include "../blitter/factory.hpp"
#include "ogs_v.h"

#include "../safeguards.h"

#include <go2/display.h>
#include <drm/drm_fourcc.h>

static go2_display_t* display;
static go2_presenter_t* presenter;
static go2_surface_t* surface;
static Palette _local_palette;

/** Factory for the null video driver. */
static FVideoDriver_OGS iFVideoDriver_OGS;


const char *VideoDriver_OGS::Start(const char * const *param)
{
    display = go2_display_create();
    int dw = go2_display_height_get(display);
    int dh = go2_display_width_get(display);

    _num_resolutions = 1;
    _resolutions[0].width = dw;
    _resolutions[0].height = dh;


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
