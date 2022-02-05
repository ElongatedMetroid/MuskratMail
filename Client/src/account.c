#include "../include/account.h"

void account_load(struct account_data *acc_data, char *datapath){
    FILE *fp = NULL;
    char ch = '\0';
    size_t bufsize = 0;
    char *linebuf = NULL;

    if(!(fp = fopen(datapath, "r")))
        ERROR("Cannot open account data file!\n");

    for(bufsize = 0; (ch = fgetc(fp)) != '\n'; bufsize++);

    rewind(fp);

    linebuf = xcalloc(bufsize, sizeof(char));

    getline(&linebuf, &bufsize, fp);
    linebuf[strcspn(linebuf, "\n")] = '\0';
    acc_data->usrname = xcalloc(bufsize, sizeof(char));
    strncpy(acc_data->usrname, linebuf, strlen(linebuf));

    DEBUG("Got usrname, %s\n", acc_data->usrname);

    getline(&linebuf, &bufsize, fp);
    linebuf[strcspn(linebuf, "\n")] = '\0';
    acc_data->ip_address = xcalloc(bufsize, sizeof(char));
    strncpy(acc_data->ip_address, linebuf, strlen(linebuf));

    DEBUG("Got ip address, %s\n\n", acc_data->ip_address);

    free(linebuf);
}