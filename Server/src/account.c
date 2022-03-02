#include "../include/account.h"

int account_load(struct client *cli, char *datapath, char *uid, char *paswd) {
  FILE *fp = NULL;
  char ch = '\0';
  size_t bufsize = 0;
  char *uid_buf = NULL;
  char *linebuf = NULL;
  fpos_t fpos;

  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i]) {
      if (clients[i]->uid == atoi(uid)) {
        printf("Client already logged in\n");
        return EXIT_FAILURE;
      }
    }
  }

  if (!(fp = fopen(datapath, "r")))
    ERROR("Cannot open account data file!\n");

  while (!feof(fp)) { // while not at the end of file
    fsetpos(fp, &fpos);
    fgetpos(fp, &fpos);

    for (bufsize = 0; (ch = fgetc(fp)) != '\n'; bufsize++)
      ;

    fsetpos(fp, &fpos);

    linebuf = xcalloc(bufsize, sizeof(char));
    getline(&linebuf, &bufsize, fp);
    linebuf[strcspn(linebuf, "\n")] = '\0';

    fgetpos(fp, &fpos);

    if (strcmp(linebuf, uid) == 0) {
      printf("Found Your UID! %s\n", linebuf);

      uid_buf = xmalloc(sizeof(linebuf));
      strncpy(uid_buf, linebuf, strlen(linebuf));

      free(linebuf);
      fgetpos(fp, &fpos);
      for (bufsize = 0; (ch = fgetc(fp)) != '\n'; bufsize++)
        ;
      fsetpos(fp, &fpos);

      linebuf = xcalloc(bufsize, sizeof(char));
      getline(&linebuf, &bufsize, fp);

      linebuf[strcspn(linebuf, "\n")] = '\0';

      if (strcmp(linebuf, paswd) == 0) {
        cli->uid = atoi(uid_buf);
        printf("Your uid %d\n", cli->uid);
        printf("Found your password, %s\n", linebuf);
        return FOUND_UID_AND_PASS;
      } else {
        free(uid_buf);
        printf("You entered the wrong password\n");
        return EXIT_FAILURE;
      }
    } else {
      printf("UID not found yet, %s\n", linebuf);
    }
  }
  return EXIT_FAILURE;
}
