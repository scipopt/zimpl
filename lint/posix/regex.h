/*TK21May2007*/

typedef struct { int a; } regex_t;
typedef struct { int b; } regmatch_t;
extern int regcomp(regex_t *preg, const char *regex, int cflags);
extern int regexec(const  regex_t  *preg,  const  char *string, size_t nmatch,
                   regmatch_t pmatch[], int eflags);
extern size_t regerror(int errcode, const regex_t *preg, char *errbuf,  size_t errbuf_size);
extern void regfree(regex_t *preg);

