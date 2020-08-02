#ifndef VIDEO_H
#define VIDEO_H

/**
 * Consider to typedef the video pointers, void* is too generic,
 * either that or create a structure to represent it
 */


/**
 * Initializes video memory
 */
void video_init();

/**
 * Creates new video memory and returns it, we are responsible for reeing with video_free
 */
void* video_new();
void video_free(void* ptr);

/**
 * Saves the current video memory into the provided pointer
 */
void video_save(void* ptr);


/**
 * Restores the provided video memory pointer back into screen memory
 */
void video_restore(void* ptr);

/**
 * Resets the cursor back to the top left of the terminal
 */
void video_reset_cursor();

#endif