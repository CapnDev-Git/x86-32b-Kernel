#ifndef SOUND_H
#define SOUND_H

#include <kstd.h>
#include <stddef.h>
#include <string.h>

struct melody *load_sound(const char *path);

void clear_sound(struct melody *melody);

#endif /* !SOUND_H_ */
