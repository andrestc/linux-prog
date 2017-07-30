/*
    Write a set-user-ID-root program similar to the sudo(8) program. 
    This program should take command-line options and arguments as follows:
    $ ./douser [-u user ] program-file arg1 arg2 ...
    The douser program executes program-file, with the given arguments, as though it was run by user. 
    (If the –u option is omitted, then user should default to root.) 
    Before executing program-file, douser should request the password for user, 
    authenticate it against the standard password file (see Listing 8-2, on page 164), 
    and then set all of the process user and group IDs to the correct values for that user
*/

#define _BSD_SOURCE /* Get getpass() declaration from <unistd.h> */
#include <unistd.h>
#include <crypt.h>
#include <stdio.h>
#include <pwd.h>
#include <limits.h>
#include <shadow.h>
#include <errno.h>
#include "tlpi_hdr.h"
#include "error_functions.c"

/*
    authenticate prompts the user for the password
    and validates it, returning 0 on success.
*/
int 
authenticate(char *username) 
{
    struct spwd *spwd;
    struct passwd *pwd;
    char *encrypted;
    Boolean authOk;

    pwd = getpwnam(username);
    if (pwd == NULL) 
        fatal("could not get password file");

    spwd = getspnam(username);
    if (spwd == NULL && errno == EACCES)
        fatal("no permission to read shadow password file");
    
    if (spwd != NULL)
        pwd->pw_passwd = spwd->sp_pwdp;
    
    encrypted = crypt(getpass("Password:"), pwd->pw_passwd);

    return strcmp(encrypted, pwd->pw_passwd);
}

int
main(int argc, char **argv)
{
    int c, status;
    char *username = NULL;
    struct passwd *pwd;
    pid_t pid;

    if (argc < 2) {
        printf("Usage: %s [-u user] cmd args...\n", argv[0]);
        exit(1);
    }

    while ((c = getopt (argc, argv, "u:")) != -1)
    switch (c) {
        case 'u':
            username = optarg;
            break;
        case '?':
        if (optopt == 'u')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
            fprintf (stderr,
                    "Unknown option character `\\x%x'.\n",
                    optopt);
        return 1;
    }
    if (username == NULL) {
        username = "root\0";
    }
    if (authenticate(username) != 0) {
        printf("Incorrect password\n");
        exit(EXIT_FAILURE);
    }

    pwd = getpwnam(username);
    if (pwd == NULL)
        fatal("unable to retrieve user info");

    if (setuid(pwd->pw_uid) != 0)   
        errExit("setuid");
    
    pid = fork();
    if (pid == -1) {
        errExit("fork");
    } else if (pid != 0) {
        execl("/usr/bin/whoami", "whoami", (char *) NULL);
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}