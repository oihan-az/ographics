#include <oplatform/runtime.hpp>
#include <oplatform/window.hpp>

int main()
{
    oplatform::Runtime runtime;

    oplatform::WindowDesc windowDesc;
    windowDesc.width = 1280;
    windowDesc.height = 720;
    windowDesc.title = "window example";
    windowDesc.resizable = true;
    windowDesc.fullscreen = false;

    oplatform::Window window(windowDesc);
    while (!window.should_close())
    {
        window.poll_events();
    }

    return 0;
}