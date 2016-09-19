#define LOGASMJS
#include "front_JS_natif.hpp"




extern "C" void run_main() {// int argc, char** argv

    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 16;
    info.width = 1024;
    info.height = 768;
    info.rdp5_performanceflags =   PERF_DISABLE_WALLPAPER
                                 | PERF_DISABLE_FULLWINDOWDRAG
                                 | PERF_DISABLE_MENUANIMATIONS;

    front.setClientInfo(info);
}
