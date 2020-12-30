#pragma once

#include "video_driver.hpp"

/** The null video driver. */
class VideoDriver_OGS : public VideoDriver {
private:
	uint ticks; ///< Amount of ticks to run.

	void PollEvent();

public:
	const char *Start(const char * const *param) override;

	void Stop() override;

	void MakeDirty(int left, int top, int width, int height) override;

	void MainLoop() override;

	bool ChangeResolution(int w, int h) override;

	bool ToggleFullscreen(bool fullscreen) override;
	const char *GetName() const override { return "null"; }
	bool HasGUI() const override { return true; }


};

/** Factory the null video driver. */
class FVideoDriver_OGS : public DriverFactoryBase {
public:
	FVideoDriver_OGS() : DriverFactoryBase(Driver::DT_VIDEO, 5, "ogs", "ODROID-GO Super Video Driver") {}
	Driver *CreateInstance() const override { return new VideoDriver_OGS(); }
};
