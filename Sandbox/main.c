#include <stdio.h>
#include <GWindow.h>

int main() {

    GWindow* window = GWindow_create(800, 600, "Lu's Graphics Engine (Metal)", 0);

     while(!GWindow_should_close(window)){
        
        GWindow_poll_events();
        
    }

    GWindow_destroy(window);
    
    printf("Hellow !\n");

    return 0;
}