#ifndef _yy_defines_h_
#define _yy_defines_h_

#define WORD 257
#define NOTOKEN 258
#define EXIT 259
#define GREAT 260
#define NEWLINE 261
#define LESS 262
#define APPEND 263
#define ERROR 264
#define APPEND_ERROR 265
#define PIPE 266
#define BACKGROUND 267
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union YYSTYPE	{
		char   *string_val;
	} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
extern YYSTYPE yylval;

#endif /* _yy_defines_h_ */
