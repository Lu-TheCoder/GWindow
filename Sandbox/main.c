#include <stdio.h>
#include <GWindow.h>
#include "Renderer/renderer.h"

int main() {


    //Creates a Graphics Window Instance with the specified properties
    GWindow* window = GWindow_create(800, 600, "Lu's Graphics Engine (Metal)", 0);

    renderer_initialize();

    //GLFW inspired Loop for rendering and handling events
     while(!GWindow_should_close(window)){
        
        //listens for OS events
        GWindow_poll_events();
    }

    renderer_shutdown();

    //deallocates resources
    GWindow_destroy(window);

    
    return 0;
}