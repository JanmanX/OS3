#pragma once

#include <stdint.h>

/*
 * A file explorer window over the mounted FAT volume.
 *
 * Click a folder to open it and the Up button to go back. Folders with more
 * entries than fit on screen are paged with Prev and Next. Clicking a file
 * reads the whole of it and shows its first bytes below the list.
 */

/* Adds the window to the compositor, which has to be running. Without a
 * mounted volume the window says so instead of listing anything. */
void explorer_init(void);
