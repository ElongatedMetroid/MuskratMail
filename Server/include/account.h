#pragma once

#include "../../include/common.h"
#include "../server.h"
#include <pthread.h>
#include <stdio.h>

#define FOUND_UID_AND_PASS 0

struct client;

int account_load(struct client *cli, char *datapath, char *uid, char *paswd);
