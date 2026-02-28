/*
 * Group Number: 1
 * Group Members:
 * Member 1 ID: 2026A7PS0001P Name: John Doe
 * Member 2 ID: 2026A7PS0002P Name: Jane Doe
 */

#include "lexer.h"
#include <ctype.h>

const char *TokenStr[] = {
    "TK_ASSIGNOP", "TK_FIELDID", "TK_ID", "TK_NUM", "TK_RNUM",
    "TK_FUNID", "TK_RUID", "TK_WITH", "TK_PARAMETERS", "TK_END", "TK_WHILE",
    "TK_UNION", "TK_ENDUNION", "TK_DEFINETYPE", "TK_AS", "TK_TYPE", "TK_MAIN",
    "TK_GLOBAL", "TK_PARAMETER", "TK_LIST", "TK_SQL", "TK_SQR", "TK_INPUT",
    "TK_OUTPUT", "TK_INT", "TK_REAL", "TK_COMMA", "TK_SEM", "TK_COLON", "TK_DOT",
    "TK_ENDWHILE", "TK_OP", "TK_CL", "TK_IF", "TK_THEN", "TK_ENDIF", "TK_READ",
    "TK_WRITE", "TK_RETURN", "TK_PLUS", "TK_MINUS", "TK_MUL", "TK_DIV", "TK_CALL",
    "TK_RECORD", "TK_ENDRECORD", "TK_ELSE", "TK_AND", "TK_OR", "TK_NOT", "TK_LT",
    "TK_LE", "TK_EQ", "TK_GT", "TK_GE", "TK_NE", "TK_ERROR", "TK_EOF"
}; // dont need tk_comment//

int currentLineNumber = 1;

FILE *getStream(FILE *fp) {
    return fp;
}

twinBuffer initBaseBuffer(FILE *fp) {
    twinBuffer B = (twinBuffer)malloc(sizeof(struct TwinBufferStruct));
    B->fp = fp;
    B->currentBuff = 1;
    B->forward = 0;
    B->bytesRead1 = fread(B->buf1, 1, BUFFER_SIZE, fp);
    B->bytesRead2 = 0;
    B->eof_reached = (B->bytesRead1 < BUFFER_SIZE);
    return B;
}

static char getNextChar(twinBuffer B) {
    if (B->currentBuff == 1) {
        if (B->forward < B->bytesRead1) {
            return B->buf1[B->forward++];
        } else if (B->forward == BUFFER_SIZE && !B->eof_reached) {
            B->bytesRead2 = fread(B->buf2, 1, BUFFER_SIZE, B->fp);
            if (B->bytesRead2 < BUFFER_SIZE) B->eof_reached = true;
            B->currentBuff = 2;
            B->forward = 1;
            if (B->bytesRead2 == 0) return EOF;
            return B->buf2[0];
        } else {
            return EOF;
        }
    } else {
        if (B->forward < B->bytesRead2) {
            return B->buf2[B->forward++];
        } else if (B->forward == BUFFER_SIZE && !B->eof_reached) {
            B->bytesRead1 = fread(B->buf1, 1, BUFFER_SIZE, B->fp);
            if (B->bytesRead1 < BUFFER_SIZE) B->eof_reached = true;
            B->currentBuff = 1;
            B->forward = 1;
            if (B->bytesRead1 == 0) return EOF;
            return B->buf1[0];
        } else {
            return EOF;
        }
    }
}

static void retractChar(twinBuffer B) {
    if (B->forward > 0) {
        B->forward--;
    } else {
        B->currentBuff = (B->currentBuff == 1) ? 2 : 1;
        B->forward = BUFFER_SIZE - 1;
    }
}

static bool isKeyword(const char *str, TokenType *outType) {
    if (strcmp(str, "with") == 0) { *outType = TK_WITH; return true; }
    if (strcmp(str, "parameters") == 0) { *outType = TK_PARAMETERS; return true; }
    if (strcmp(str, "end") == 0) { *outType = TK_END; return true; }
    if (strcmp(str, "while") == 0) { *outType = TK_WHILE; return true; }
    if (strcmp(str, "union") == 0) { *outType = TK_UNION; return true; }
    if (strcmp(str, "endunion") == 0) { *outType = TK_ENDUNION; return true; }
    if (strcmp(str, "definetype") == 0) { *outType = TK_DEFINETYPE; return true; }
    if (strcmp(str, "as") == 0) { *outType = TK_AS; return true; }
    if (strcmp(str, "type") == 0) { *outType = TK_TYPE; return true; }
    if (strcmp(str, "_main") == 0) { *outType = TK_MAIN; return true; }
    if (strcmp(str, "global") == 0) { *outType = TK_GLOBAL; return true; }
    if (strcmp(str, "parameter") == 0) { *outType = TK_PARAMETER; return true; }
    if (strcmp(str, "list") == 0) { *outType = TK_LIST; return true; }
    if (strcmp(str, "input") == 0) { *outType = TK_INPUT; return true; }
    if (strcmp(str, "output") == 0) { *outType = TK_OUTPUT; return true; }
    if (strcmp(str, "int") == 0) { *outType = TK_INT; return true; }
    if (strcmp(str, "real") == 0) { *outType = TK_REAL; return true; }
    if (strcmp(str, "endwhile") == 0) { *outType = TK_ENDWHILE; return true; }
    if (strcmp(str, "if") == 0) { *outType = TK_IF; return true; }
    if (strcmp(str, "then") == 0) { *outType = TK_THEN; return true; }
    if (strcmp(str, "endif") == 0) { *outType = TK_ENDIF; return true; }
    if (strcmp(str, "read") == 0) { *outType = TK_READ; return true; }
    if (strcmp(str, "write") == 0) { *outType = TK_WRITE; return true; }
    if (strcmp(str, "return") == 0) { *outType = TK_RETURN; return true; }
    if (strcmp(str, "call") == 0) { *outType = TK_CALL; return true; }
    if (strcmp(str, "record") == 0) { *outType = TK_RECORD; return true; }
    if (strcmp(str, "endrecord") == 0) { *outType = TK_ENDRECORD; return true; }
    if (strcmp(str, "else") == 0) { *outType = TK_ELSE; return true; }
    return false;
}

tokenInfo getNextToken(twinBuffer B) {
    tokenInfo ti = (tokenInfo)malloc(sizeof(token_info));
    ti->tokenType = TK_ERROR;
    ti->lexeme[0] = '\0';
    ti->lineNo = currentLineNumber;

    char c;
    while ((c = getNextChar(B)) != EOF) {
        if (c == ' ' || c == '\t' || c == '\r') {
            continue;
        } else if (c == '\n') {
            currentLineNumber++;
        } else if (c == '%') {
            while ((c = getNextChar(B)) != EOF && c != '\n');

    if (c == '\n')
        currentLineNumber++;
    // Just continue scanning
    continue;
        }
    }

    if (c == EOF) {
        ti->tokenType = TK_EOF;
        strcpy(ti->lexeme, "EOF");
        ti->lineNo = currentLineNumber;
        return ti;
    }

    ti->lineNo = currentLineNumber;
    int lex_len = 0;
    c = getNextChar(B);
    ti->lexeme[lex_len++] = c;
    ti->lexeme[lex_len] = '\0';

    if (c == '[') { ti->tokenType = TK_SQL; return ti; }
    if (c == ']') { ti->tokenType = TK_SQR; return ti; }
    if (c == ',') { ti->tokenType = TK_COMMA; return ti; }
    if (c == ';') { ti->tokenType = TK_SEM; return ti; }
    if (c == ':') { ti->tokenType = TK_COLON; return ti; }
    if (c == '.') { ti->tokenType = TK_DOT; return ti; }
    if (c == '(') { ti->tokenType = TK_OP; return ti; }
    if (c == ')') { ti->tokenType = TK_CL; return ti; }
    if (c == '+') { ti->tokenType = TK_PLUS; return ti; }
    if (c == '-') { ti->tokenType = TK_MINUS; return ti; }
    if (c == '*') { ti->tokenType = TK_MUL; return ti; }
    if (c == '/') { ti->tokenType = TK_DIV; return ti; }
    if (c == '~') { ti->tokenType = TK_NOT; return ti; }
    
    if (c == '=') {
        char next1 = getNextChar(B);
        if (next1 == '=') {
            ti->lexeme[lex_len++] = next1; ti->lexeme[lex_len] = '\0';
            ti->tokenType = TK_EQ; return ti;
        } else {
            retractChar(B);
            fprintf(stderr, "Line No %d: Error : Unknown Symbol <=>\n", currentLineNumber);
            ti->tokenType = TK_ERROR; return ti;
        }
    }
    if (c == '!') {
        char next1 = getNextChar(B);
        if (next1 == '=') {
            ti->lexeme[lex_len++] = next1; ti->lexeme[lex_len] = '\0';
            ti->tokenType = TK_NE; return ti;
        } else {
            retractChar(B);
            fprintf(stderr, "Line No %d : Error : Unknown Symbol <!>\n", currentLineNumber);
            ti->tokenType = TK_ERROR; return ti;
        }
    }
    if (c == '<') {
        char next1 = getNextChar(B);
        if (next1 == '=') {
            ti->lexeme[lex_len++] = next1; ti->lexeme[lex_len] = '\0';
            ti->tokenType = TK_LE; return ti;
        } else if (next1 == '-') {
            char next2 = getNextChar(B);
            if (next2 == '-') {
                char next3 = getNextChar(B);
                if (next3 == '-') {
                    ti->lexeme[lex_len++] = next1; ti->lexeme[lex_len++] = next2; ti->lexeme[lex_len++] = next3; ti->lexeme[lex_len] = '\0';
                    ti->tokenType = TK_ASSIGNOP; return ti;
                } else {
                    retractChar(B); retractChar(B); retractChar(B);
                    ti->tokenType = TK_LT; return ti;
                }
            } else {
                retractChar(B); retractChar(B);
                ti->tokenType = TK_LT; return ti;
            }
        } else {
            retractChar(B);
            ti->tokenType = TK_LT; return ti;
        }
    }
    if (c == '>') {
        char next1 = getNextChar(B);
        if (next1 == '=') {
            ti->lexeme[lex_len++] = next1; ti->lexeme[lex_len] = '\0';
            ti->tokenType = TK_GE; return ti;
        } else {
            retractChar(B);
            ti->tokenType = TK_GT; return ti;
        }
    }
    if (c == '&') {
        char next1 = getNextChar(B);
        if (next1 == '&') {
            char next2 = getNextChar(B);
            if (next2 == '&') {
                ti->lexeme[lex_len++] = next1; ti->lexeme[lex_len++] = next2; ti->lexeme[lex_len] = '\0';
                ti->tokenType = TK_AND; return ti;
            } else {
                retractChar(B);
                ti->lexeme[lex_len++] = next1; ti->lexeme[lex_len] = '\0';
                fprintf(stderr, "Line no: %d : Error: Unknown pattern <%s>\n", currentLineNumber, ti->lexeme);
                ti->tokenType = TK_ERROR; return ti;
            }
        } else {
            retractChar(B);
            fprintf(stderr, "Line No %d : Error: Unknown Symbol <&>\n", currentLineNumber);
            ti->tokenType = TK_ERROR; return ti;
        }
    }
    if (c == '@') {
        char next1 = getNextChar(B);
        if (next1 == '@') {
            char next2 = getNextChar(B);
            if (next2 == '@') {
                ti->lexeme[lex_len++] = next1; ti->lexeme[lex_len++] = next2; ti->lexeme[lex_len] = '\0';
                ti->tokenType = TK_OR; return ti;
            } else {
                retractChar(B);
                ti->lexeme[lex_len++] = next1; ti->lexeme[lex_len] = '\0';
                fprintf(stderr, "Line no: %d : Error: Unknown pattern <%s>\n", currentLineNumber, ti->lexeme);
                ti->tokenType = TK_ERROR; return ti;
            }
        } else {
            retractChar(B);
            fprintf(stderr, "Line No %d : Error: Unknown Symbol <@>\n", currentLineNumber);
            ti->tokenType = TK_ERROR; return ti;
        }
    }
    if (c == '|') {
        fprintf(stderr, "Line No %d : Error: Unknown Symbol <|>\n", currentLineNumber);
        ti->tokenType = TK_ERROR; return ti;
    }

    if (c == '#') {
        char next = getNextChar(B);
        if (next >= 'a' && next <= 'z') {
            while (next >= 'a' && next <= 'z') {
                if (lex_len < 63) ti->lexeme[lex_len++] = next;
                next = getNextChar(B);
            }
            retractChar(B);
            ti->lexeme[lex_len] = '\0';
            ti->tokenType = TK_RUID;
            return ti;
        } else {
            retractChar(B);
            fprintf(stderr, "Line No %d : Error: Unknown Symbol <#>\n", currentLineNumber);
            ti->tokenType = TK_ERROR; return ti;
        }
    }

    if (c == '_') {
        char next = getNextChar(B);
        if ((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z')) {
            while ((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z')) {
                if (lex_len < 63) ti->lexeme[lex_len++] = next;
                next = getNextChar(B);
            }
            while (next >= '0' && next <= '9') {
                if (lex_len < 63) ti->lexeme[lex_len++] = next;
                next = getNextChar(B);
            }
            retractChar(B);
            ti->lexeme[lex_len] = '\0';
            if (strcmp(ti->lexeme, "_main") == 0) {
                ti->tokenType = TK_MAIN;
            } else {
                if (lex_len > 30) {
                    fprintf(stderr, "Line No %d: Error :Function Identifier is longer than the prescribed length of 30 characters.\n", currentLineNumber);
                    ti->tokenType = TK_ERROR;
                } else {
                    ti->tokenType = TK_FUNID;
                }
            }
            return ti;
        } else {
            retractChar(B);
            fprintf(stderr, "Line No %d : Error: Unknown Symbol <_>\n", currentLineNumber);
            ti->tokenType = TK_ERROR; return ti;
        }
    }

    if (c >= '0' && c <= '9') {
        char next = getNextChar(B);
        while (next >= '0' && next <= '9') {
            if (lex_len < 63) ti->lexeme[lex_len++] = next;
            next = getNextChar(B);
        }
        if (next == '.') {
            char p1 = getNextChar(B);
            if (p1 >= '0' && p1 <= '9') {
                char p2 = getNextChar(B);
                if (p2 >= '0' && p2 <= '9') {
                    ti->lexeme[lex_len++] = next;
                    ti->lexeme[lex_len++] = p1;
                    ti->lexeme[lex_len++] = p2;
                    
                    char e = getNextChar(B);
                    if (e == 'E') {
                        ti->lexeme[lex_len++] = e;
                        char s = getNextChar(B);
                        if (s == '+' || s == '-') {
                            ti->lexeme[lex_len++] = s;
                            char d1 = getNextChar(B);
                            if (d1 >= '0' && d1 <= '9') {
                                char d2 = getNextChar(B);
                                if (d2 >= '0' && d2 <= '9') {
                                    ti->lexeme[lex_len++] = d1;
                                    ti->lexeme[lex_len++] = d2;
                                    ti->lexeme[lex_len] = '\0';
                                    ti->tokenType = TK_RNUM;
                                    return ti;
                                } else {
                                    retractChar(B); retractChar(B); retractChar(B); retractChar(B);
                                    ti->lexeme[lex_len - 1] = '\0'; // Remove sign
                                    fprintf(stderr, "Line no: %d : Error: Unknown pattern <%s>\n", currentLineNumber, ti->lexeme);
                                    ti->tokenType = TK_ERROR; return ti;
                                }
                            } else {
                                retractChar(B); retractChar(B); retractChar(B);
                                ti->lexeme[lex_len - 1] = '\0';
                                fprintf(stderr, "Line no: %d : Error: Unknown pattern <%s>\n", currentLineNumber, ti->lexeme);
                                ti->tokenType = TK_ERROR; return ti;
                            }
                        } else if (s >= '0' && s <= '9') {
                            char d2 = getNextChar(B);
                            if (d2 >= '0' && d2 <= '9') {
                                ti->lexeme[lex_len++] = s;
                                ti->lexeme[lex_len++] = d2;
                                ti->lexeme[lex_len] = '\0';
                                ti->tokenType = TK_RNUM;
                                return ti;
                            } else {
                                retractChar(B); retractChar(B); retractChar(B);
                                ti->lexeme[lex_len - 1] = '\0'; // remove E
                                fprintf(stderr, "Line no: %d : Error: Unknown pattern <%s>\n", currentLineNumber, ti->lexeme);
                                ti->tokenType = TK_ERROR; return ti;
                            }
                        } else {
                            retractChar(B); // retract s
                            retractChar(B); // retract E
                            ti->lexeme[lex_len - 1] = '\0';
                            fprintf(stderr, "Line no: %d : Error: Unknown pattern <%s>\n", currentLineNumber, ti->lexeme);
                            ti->tokenType = TK_ERROR; return ti;
                        }
                    } else {
                        retractChar(B);
                        ti->lexeme[lex_len] = '\0';
                        ti->tokenType = TK_RNUM;
                        return ti;
                    }
                } else {
                    ti->lexeme[lex_len++] = next;
                    ti->lexeme[lex_len++] = p1;
                    ti->lexeme[lex_len] = '\0';
                    retractChar(B); // P2
                    fprintf(stderr, "Line no: %d : Error: Unknown pattern <%s>\n", currentLineNumber, ti->lexeme);
                    ti->tokenType = TK_ERROR; return ti;
                }
            } else {
                retractChar(B);
                retractChar(B);
                ti->lexeme[lex_len] = '\0';
                ti->tokenType = TK_NUM;
                return ti;
            }
        }
        retractChar(B);
        ti->lexeme[lex_len] = '\0';
        ti->tokenType = TK_NUM;
        return ti;
    }

    if (c >= 'a' && c <= 'z') {
        char next = getNextChar(B);
        bool is_id_path = false;
        
        if (c >= 'b' && c <= 'd' && next >= '2' && next <= '7') {
             is_id_path = true;
        }

        if (is_id_path) {
             ti->lexeme[lex_len++] = next;
             next = getNextChar(B);
             
             while (next >= 'b' && next <= 'd') {
                 if (lex_len < 63) ti->lexeme[lex_len++] = next;
                 next = getNextChar(B);
             }
             while (next >= '2' && next <= '7') {
                 if (lex_len < 63) ti->lexeme[lex_len++] = next;
                 next = getNextChar(B);
             }
             retractChar(B);
             ti->lexeme[lex_len] = '\0';
             
             if (lex_len > 20) {
                 fprintf(stderr, "Line No %d: Error :Variable Identifier is longer than the prescribed length of 20 characters.\n", currentLineNumber);
                 ti->tokenType = TK_ERROR;
             } else {
                 ti->tokenType = TK_ID;
             }
             return ti;
        } else {
             while ((next >= 'a' && next <= 'z')) {
                 if (lex_len < 63) ti->lexeme[lex_len++] = next;
                 next = getNextChar(B);
             }
             retractChar(B);
             ti->lexeme[lex_len] = '\0';
             
             TokenType kwType;
             if (isKeyword(ti->lexeme, &kwType)) {
                 ti->tokenType = kwType;
                 return ti;
             }
             
             ti->tokenType = TK_FIELDID;
             return ti;
        }
    }

    ti->lexeme[lex_len] = '\0';
    fprintf(stderr, "Line No %d : Error: Unknown Symbol <%c>\n", currentLineNumber, c);
    ti->tokenType = TK_ERROR;
    return ti;
}

void removeComments(char *testcaseFile, char *cleanFile) {
    FILE *in = fopen(testcaseFile, "r");
    FILE *out = fopen(cleanFile, "w");
    if (!in || !out) {
        if (in) fclose(in);
        if (out) fclose(out);
        return;
    }

    int c;
    while ((c = fgetc(in)) != EOF) {
        if (c == '%') {
            while ((c = fgetc(in)) != EOF && c != '\n') {}
            if (c == '\n') fputc(c, out);
        } else {
            fputc(c, out);
        }
    }
    fclose(in);
    fclose(out);
}
