#include "scrollkeylistener.h"
#include "listener.h"
#include "keyboard.h"
#include "task/process.h"
#include "video/video.h"
void scrollkeylistener_keypress(char key);
void scrollkeylistener_special(enum SpecialKeys key);

static struct keyboard_listener listener = {
    .keypress = scrollkeylistener_keypress,
    .special = scrollkeylistener_special
};

void scrollkeylistener_init()
{
    keyboard_listener_register(&listener);
}


void scrollkeylistener_keypress(char key)
{

}

void scrollkeylistener_special(enum SpecialKeys key)
{
    if (key != UP_PRESSED_OR_RELEASED && key != DOWN_PRESSED_OR_RELEASED)
        return;


    // We don't care for release events.
    if (!keyboard_is_special_on(key))
    {
        return;
    }

    // Get the active process so we know which terminal to scroll
    struct process* process = process_current();
    struct terminal_properties* properties = &process->video->properties;
    if (key == DOWN_PRESSED_OR_RELEASED)
    {
        video_terminal_set_scroll(properties, properties->y_scroll+1);
        return;
    }
    
    video_terminal_set_scroll(properties, properties->y_scroll-1);
}