%option noyywrap
%{
#include <stdio.h>
%}
%%
[a-z]+ { puts(yytext); }
[ \n]+ ;
. ;
%%
int main(void) { return yylex(); }
