%{
int yylex(void) { return 0; }
void yyerror(const char *s) { (void)s; }
%}
%%
input: %empty;
%%
int main(void) { return yyparse(); }
