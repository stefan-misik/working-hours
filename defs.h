#ifndef DEFS_H
#define	DEFS_H

#include "win_common.h"

/* Stringification */
#define DEFS_STRINGIFY(s) DEFS_STRINGIFYX(s)
#define DEFS_STRINGIFYX(s) #s

/* Project version*/
#define PROJECT_VER_1           2
#define PROJECT_VER_2           0
#define PROJECT_VER_3           2
#ifdef PROJECT_COMMITS
    #define PROJECT_VER_4       PROJECT_COMMITS
#else
    #define PROJECT_VER_4       0
#endif
#ifndef PROJECT_LAST_COMMIT
    #define PROJECT_LAST_COMMIT 0
#endif

#ifndef PROJECT_LAST_RELEASE
    #define PROJECT_LAST_RELEASE         ""
#endif

#ifndef PROJECT_GIT
    #define PROJECT_GIT         ""
#endif

#define PROJECT_VER             "v" DEFS_STRINGIFY(PROJECT_VER_1) "."     \
                                    DEFS_STRINGIFY(PROJECT_VER_2) "."     \
                                    DEFS_STRINGIFY(PROJECT_VER_3) "."     \
                                    DEFS_STRINGIFY(PROJECT_VER_4)
#define PROJECT_NAME            "Working-hours"
#define PROJECT_AUTHOR          "Stefan Misik"
#define PROJECT_DESC            "Working Hours: estimate time spent working"
#define PROJECT_CPYR            "Copyright (C) 2018 " PROJECT_AUTHOR

#define PROJECT_WEB "https://github.com/stefan-misik/working-hours/releases/latest"

#define PROJECT_MAIL_USER "mail"
#define PROJECT_MAIL_SERVER "stefanmisik.com"

extern const TCHAR lpProjectName[];
extern const TCHAR lpProjectDesc[];
extern const TCHAR lpProjectAuthor[];
extern const TCHAR lpProjectVer[];
extern const TCHAR lpProjectLastRelease[];
extern const TCHAR lpProjectGit[];
extern const TCHAR lpProjectWeb[];
extern const TCHAR lpProjectMail[];
extern const TCHAR lpProjectLic[];

#endif	/* DEFS_H */
