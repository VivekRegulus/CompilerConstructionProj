#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lexer.h"

/* ========================================================= */
/*                    TWIN BUFFER CODE                       */
/* ========================================================= */

twinBuffer initBaseBuffer(FILE *fp)
{
    twinBuffer B = malloc(sizeof(struct TwinBufferStruct));
    B->fp = fp;
    B->currentBuff = 1;
    B->forward = 0;

    B->bytesRead1 = fread(B->buf1, 1, BUFFER_SIZE, fp);
    B->bytesRead2 = 0;

    B->eof_reached = (B->bytesRead1 < BUFFER_SIZE);
    return B;
}

char getNextChar(twinBuffer B)
{
    if (B->currentBuff == 1) {
        if (B->forward < B->bytesRead1)
            return B->buf1[B->forward++];

        B->bytesRead2 = fread(B->buf2, 1, BUFFER_SIZE, B->fp);
        B->currentBuff = 2;
        B->forward = 0;

        if (B->bytesRead2 == 0)
            return EOF;

        return B->buf2[B->forward++];
    }
    else {
        if (B->forward < B->bytesRead2)
            return B->buf2[B->forward++];

        B->bytesRead1 = fread(B->buf1, 1, BUFFER_SIZE, B->fp);
        B->currentBuff = 1;
        B->forward = 0;

        if (B->bytesRead1 == 0)
            return EOF;

        return B->buf1[B->forward++];
    }
}

void retractChar(twinBuffer B)
{
    if (B->forward > 0)
        B->forward--;
    else {
        B->currentBuff = (B->currentBuff == 1) ? 2 : 1;
        B->forward = BUFFER_SIZE - 1;
    }
}

/* ========================================================= */
/*                    HELPER FUNCTIONS                       */
/* ========================================================= */

bool isKeyword(const char *str, TokenType *outType)
{
    if (strcmp(str, "with") == 0)        { *outType = TK_WITH; return true; }
    if (strcmp(str, "parameters") == 0)  { *outType = TK_PARAMETERS; return true; }
    if (strcmp(str, "end") == 0)         { *outType = TK_END; return true; }
    if (strcmp(str, "while") == 0)       { *outType = TK_WHILE; return true; }
    if (strcmp(str, "if") == 0)          { *outType = TK_IF; return true; }
    if (strcmp(str, "then") == 0)        { *outType = TK_THEN; return true; }
    if (strcmp(str, "endif") == 0)       { *outType = TK_ENDIF; return true; }
    if (strcmp(str, "read") == 0)        { *outType = TK_READ; return true; }
    if (strcmp(str, "write") == 0)       { *outType = TK_WRITE; return true; }
    if (strcmp(str, "return") == 0)      { *outType = TK_RETURN; return true; }
    if (strcmp(str, "int") == 0)         { *outType = TK_INT; return true; }
    if (strcmp(str, "real") == 0)        { *outType = TK_REAL; return true; }
    return false;
}

/* ========================================================= */
/*                  DFA-BASED LEXER                          */
/* ========================================================= */

int currentLineNumber = 1;

tokenInfo getNextToken(twinBuffer B)
{
    tokenInfo ti = malloc(sizeof(token_info));
    ti->lexeme[0] = '\0';
    ti->lineNo = currentLineNumber;

    int lex_len = 0;
    char c;

    typedef enum {
        ST_START,
        ST_ID,
        ST_FUNID,
        ST_RUID,
        ST_NUM,
        ST_RNUM,
        ST_LT,
        ST_GT,
        ST_EQ,
        ST_NE,
        ST_ASSIGNOP,
        ST_AND,
        ST_OR,
        ST_COMMENT,
        ST_DONE
    } DFAState;

    DFAState state = ST_START;

    while (state != ST_DONE) {
        c = getNextChar(B);

        switch (state) {

        case ST_START:
            if (c == ' ' || c == '\t' || c == '\r')
                break;

            if (c == '\n') {
                currentLineNumber++;
                break;
            }

            if (c == EOF) {
                ti->tokenType = TK_EOF;
                strcpy(ti->lexeme, "EOF");
                state = ST_DONE;
                break;
            }

            ti->lexeme[lex_len++] = c;
            ti->lexeme[lex_len] = '\0';

            if (isdigit(c))                state = ST_NUM;
            else if (c >= 'a' && c <= 'z') state = ST_ID;
            else if (c == '_')             state = ST_FUNID;
            else if (c == '#')             state = ST_RUID;
            else if (c == '%')             state = ST_COMMENT;
            else if (c == '<')             state = ST_LT;
            else if (c == '>')             state = ST_GT;
            else if (c == '=')             state = ST_EQ;
            else if (c == '!')             state = ST_NE;
            else if (c == '&')             state = ST_AND;
            else if (c == '@')             state = ST_OR;

            else if (c == '+') { ti->tokenType = TK_PLUS;  state = ST_DONE; }
            else if (c == '-') { ti->tokenType = TK_MINUS; state = ST_DONE; }
            else if (c == '*') { ti->tokenType = TK_MUL;   state = ST_DONE; }
            else if (c == '/') { ti->tokenType = TK_DIV;   state = ST_DONE; }
            else {
                fprintf(stderr,
                        "Line %d : Error: Unknown symbol <%c>\n",
                        currentLineNumber, c);
                ti->tokenType = TK_ERROR;
                state = ST_DONE;
            }
            break;

        case ST_ID:
            if (c >= 'a' && c <= 'z') {
                ti->lexeme[lex_len++] = c;
                ti->lexeme[lex_len] = '\0';
            } else {
                retractChar(B);
                TokenType kw;
                if (isKeyword(ti->lexeme, &kw))
                    ti->tokenType = kw;
                else
                    ti->tokenType = TK_FIELDID;
                state = ST_DONE;
            }
            break;

        case ST_FUNID:
            if (isalnum(c)) {
                ti->lexeme[lex_len++] = c;
                ti->lexeme[lex_len] = '\0';
            } else {
                retractChar(B);
                if (strcmp(ti->lexeme, "_main") == 0)
                    ti->tokenType = TK_MAIN;
                else
                    ti->tokenType = TK_FUNID;
                state = ST_DONE;
            }
            break;

        case ST_RUID:
            if (c >= 'a' && c <= 'z') {
                ti->lexeme[lex_len++] = c;
                ti->lexeme[lex_len] = '\0';
            } else {
                retractChar(B);
                ti->tokenType = TK_RUID;
                state = ST_DONE;
            }
            break;

        case ST_NUM:
            if (isdigit(c)) {
                ti->lexeme[lex_len++] = c;
                ti->lexeme[lex_len] = '\0';
            } else if (c == '.') {
                ti->lexeme[lex_len++] = c;
                ti->lexeme[lex_len] = '\0';
                state = ST_RNUM;
            } else {
                retractChar(B);
                ti->tokenType = TK_NUM;
                state = ST_DONE;
            }
            break;

        case ST_RNUM:
            if (isdigit(c)) {
                ti->lexeme[lex_len++] = c;
                ti->lexeme[lex_len] = '\0';
            } else {
                retractChar(B);
                ti->tokenType = TK_RNUM;
                state = ST_DONE;
            }
            break;

        case ST_COMMENT:
            while (c != '\n' && c != EOF)
                c = getNextChar(B);
            if (c == '\n')
                currentLineNumber++;
            lex_len = 0;
            state = ST_START;
            break;

        default:
            ti->tokenType = TK_ERROR;
            state = ST_DONE;
        }
    }

    ti->lineNo = currentLineNumber;
    return ti;
}
