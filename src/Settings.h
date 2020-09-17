#pragma once
#include "definitions.h"
#include "input/ControllerBindings.h"
#include "input/KeyBindings.h"
#include <filesystem>
#include <string>

namespace Shiro {
    struct Settings {
        Settings();

        bool init(const int argc, const char* const argv[]);
        void read(const std::string filename);
        void write() const;

        KeyBindings keyBindings;
        ControllerBindings controllerBindings;
        float videoScale;
        int videoStretch;
        int fullscreen;
        int vsync;
        int frameDelay;
        int vsyncTimestep;
#ifdef ENABLE_OPENGL_INTERPOLATION
        int interpolate;
#endif
        int masterVolume;
        int sfxVolume;
        int musicVolume;
        std::filesystem::path configurationPath;
        std::filesystem::path basePath;
        std::string playerName;
    };
}