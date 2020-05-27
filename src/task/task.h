#ifndef TASK_H
#define TASK_H

typedef void(*USER_MODE_FUNCTION)();
void user_mode_enter(USER_MODE_FUNCTION);

#endif