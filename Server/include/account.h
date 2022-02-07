#pragma once

#include "../../include/common.h"
#include "../server.h"
#include <stdio.h>
#include <pthread.h>

#define FOUND_UID_AND_PASS 0

int account_load(struct client *cli, char *datapath, char* uid, char *paswd);