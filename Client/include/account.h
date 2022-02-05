#pragma once

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../../include/common.h"

struct account_data{
    char *usrname;
    char *ip_address;
};

void account_load(struct account_data *acc_data, char *datapath);