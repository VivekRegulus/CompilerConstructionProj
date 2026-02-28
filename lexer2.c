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

        /* ---------------- START STATE ---------------- */
        case ST_START:
            if (c == ' ' || c == '\t' || c == '\r')
                break;

            if (c == '\n') {
                currentLineNumber++;
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
            else if (c == '(') { ti->tokenType = TK_OP;    state = ST_DONE; }
            else if (c == ')') { ti->tokenType = TK_CL;    state = ST_DONE; }
            else if (c == '[') { ti->tokenType = TK_SQL;   state = ST_DONE; }
            else if (c == ']') { ti->tokenType = TK_SQR;   state = ST_DONE; }
            else if (c == ';') { ti->tokenType = TK_SEM;   state = ST_DONE; }
            else if (c == ',') { ti->tokenType = TK_COMMA; state = ST_DONE; }
            else {
                fprintf(stderr,
                        "Line %d : Error: Unknown symbol <%c>\n",
                        currentLineNumber, c);
                ti->tokenType = TK_ERROR;
                state = ST_DONE;
            }
            break;

        /* ---------------- IDENTIFIER / KEYWORD ---------------- */
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

        /* ---------------- FUNCTION IDENTIFIER ---------------- */
        case ST_FUNID:
            if (isalnum(c)) {
                ti->lexeme[lex_len++] = c;
                ti->lexeme[lex_len] = '\0';
            } else {
                retractChar(B);
                if (strcmp(ti->lexeme, "_main") == 0)
                    ti->tokenType = TK_MAIN;
                else if (lex_len > 30) {
                    fprintf(stderr,
                            "Line %d: Function identifier too long\n",
                            currentLineNumber);
                    ti->tokenType = TK_ERROR;
                } else
                    ti->tokenType = TK_FUNID;
                state = ST_DONE;
            }
            break;

        /* ---------------- RUID ---------------- */
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

        /* ---------------- INTEGER ---------------- */
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

        /* ---------------- REAL NUMBER ---------------- */
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

        /* ---------------- <, <=, <--- ---------------- */
        case ST_LT:
            if (c == '=') {
                ti->lexeme[lex_len++] = c;
                ti->tokenType = TK_LE;
                state = ST_DONE;
            } else if (c == '-') {
                ti->lexeme[lex_len++] = c;
                state = ST_ASSIGNOP;
            } else {
                retractChar(B);
                ti->tokenType = TK_LT;
                state = ST_DONE;
            }
            break;

        case ST_ASSIGNOP:
            if (c == '-' && getNextChar(B) == '-') {
                ti->lexeme[lex_len++] = '-';
                ti->lexeme[lex_len++] = '-';
                ti->tokenType = TK_ASSIGNOP;
            } else {
                fprintf(stderr,
                        "Line %d : Error: Invalid assignment operator\n",
                        currentLineNumber);
                ti->tokenType = TK_ERROR;
            }
            state = ST_DONE;
            break;

        /* ---------------- >, >= ---------------- */
        case ST_GT:
            if (c == '=') {
                ti->lexeme[lex_len++] = c;
                ti->tokenType = TK_GE;
            } else {
                retractChar(B);
                ti->tokenType = TK_GT;
            }
            state = ST_DONE;
            break;

        /* ---------------- == ---------------- */
        case ST_EQ:
            if (c == '=') {
                ti->lexeme[lex_len++] = c;
                ti->tokenType = TK_EQ;
            } else {
                retractChar(B);
                fprintf(stderr,
                        "Line %d : Error: Invalid '='\n",
                        currentLineNumber);
                ti->tokenType = TK_ERROR;
            }
            state = ST_DONE;
            break;

        /* ---------------- != ---------------- */
        case ST_NE:
            if (c == '=') {
                ti->lexeme[lex_len++] = c;
                ti->tokenType = TK_NE;
            } else {
                retractChar(B);
                fprintf(stderr,
                        "Line %d : Error: Invalid '!'\n",
                        currentLineNumber);
                ti->tokenType = TK_ERROR;
            }
            state = ST_DONE;
            break;

        /* ---------------- &&& ---------------- */
        case ST_AND:
            if (c == '&' && getNextChar(B) == '&')
                ti->tokenType = TK_AND;
            else {
                fprintf(stderr,
                        "Line %d : Error: Invalid AND operator\n",
                        currentLineNumber);
                ti->tokenType = TK_ERROR;
            }
            state = ST_DONE;
            break;

        /* ---------------- @@@ ---------------- */
        case ST_OR:
            if (c == '@' && getNextChar(B) == '@')
                ti->tokenType = TK_OR;
            else {
                fprintf(stderr,
                        "Line %d : Error: Invalid OR operator\n",
                        currentLineNumber);
                ti->tokenType = TK_ERROR;
            }
            state = ST_DONE;
            break;

        /* ---------------- COMMENT (SKIPPED) ---------------- */
        case ST_COMMENT:
            while (c != '\n' && c != EOF)
                c = getNextChar(B);

            if (c == '\n')
                currentLineNumber++;

            lex_len = 0;
            state = ST_START;
            break;
        }
    }

    ti->lineNo = currentLineNumber;
    return ti;
}
