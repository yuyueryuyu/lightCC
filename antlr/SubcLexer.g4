lexer grammar SubcLexer;


INT     : 'int';
FLOAT   : 'float';
IF      : 'if';
ELSE    : 'else';
WHILE   : 'while';
RETURN  : 'return';
INPUT   : 'input';
PRINT   : 'print';
VOID    : 'void';

ADD     : '+';
SUB     : '-';
MUL     : '*';
DIV     : '/';
MOD     : '%';

EQ      : '==';
ASG     : '=';

LT      : '<';
LE      : '<=';

GT      : '>';
GE      : '>=';

NE      : '!=';

ANDAND  : '&&';
OROR    : '||';

LPA     : '(';
RPA     : ')';
LBK     : '[';
RBK     : ']';
LBR     : '{';
RBR     : '}';
CMA     : ',';
SCO     : ';';

NUM     : ('+' | '-')? [0-9]+;
FLO     : ('+' | '-')? [0-9]* '.' [0-9]+ | [0-9]+ '.' [0-9]*;
ID      : [a-zA-Z][a-zA-Z0-9]*;
WS      : [ \t\n\r\f]+ -> skip ;