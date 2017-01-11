#include "netclipboard.h"

#ifdef _WIN32
void sleep(int t)
{
	Sleep(t * 1000);
}
#endif

int get_line(char *prmpt, char *buff, size_t sz)
{
    int ch, extra;

    if (prmpt != NULL)
    {
        printf("%s", prmpt);
        fflush(stdout);
    }
    if (fgets(buff, sz, stdin) == NULL)
        return -1;

    if (buff[strlen(buff)-1] != '\n')
    {
        extra = 0;
        while(((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? -2 : 0;
    }

    buff[strlen(buff)-1] = '\0';
    return 0;
}
