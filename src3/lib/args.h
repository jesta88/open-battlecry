#pragma once

void args_init(int argc, char** argv);
int args_check_next(int last, const char* arg);
int args_check(const char* arg);