#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

unsigned int get_file_size(FILE *file) {
    if(!file) {
        printf("Failed to get file pointer in %s", __func__);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    unsigned int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(size < 1) {
        printf("Failed to get file size\n");
        exit(1);
    }
    else if(size == 1) {
        printf("File contains only EOF\n");
        exit(2);
    }

    return size;
}

char* get_file_contents(const char* path) {
    FILE* file = fopen(path, "r");
    if(!file) {
        printf("Failed to create file object in %s", __func__);
        exit(0);
    }
    
    unsigned int size = get_file_size(file);
    char* contents = malloc(size + 1);
    fread(contents, 1, size, file);
    contents[size] = '\0';

    return contents;
}

typedef struct Error{
    enum ErrorType {
        ERROR_NONE,
        ERROR_ARGUMENTS,
        ERROR_SYNTAX
    } type;
    char* msg;
} Error;

void print_error(Error err) {
    printf("ERROR: ");
    switch(err.type) {
        case ERROR_NONE:
            break;
        case ERROR_ARGUMENTS:
            printf("Invalid arguments");
            break;
        case ERROR_SYNTAX:
            printf("Invalid syntax");
            break;
    }
    putchar('\n');
    printf("     :%s", err.msg);
}

#define ERROR_PREP(n, t, m)     \
    (n).type = t;               \
    (n).msg = m;

Error ok = {ERROR_NONE, NULL};

typedef enum TokenType {
    INST,
    NUM,
    VAR
} Type;

typedef enum Inst_type {
    PUSH,
    POP,
    ADD,
    MUL,
    SUB,
    PRINT,
    HLT
}Inst_type;

typedef struct Token {
    Type token_type;
    char* beg;
    char* end;
    union {
        Inst_type inst_type;
        int value;
    } val;
    struct Token* next_token;
} Token;

typedef struct Binding {
    char* id;
    int value;
} Binding;

int num_bindings;
Binding* binds;

const char* ws = " \r\n";
const char* delims = " \r\n#";

Error lex(char* src, Token* token) {
    Error err = ok;
    if(!src || !token) {
        ERROR_PREP(err, ERROR_ARGUMENTS, "Cannot lex empty source\n");
        return err;
    }
    token->beg = src;
    token->beg += strspn(token->beg, ws);
    
    token->end = token->beg;
    if(*token->end == '\0') {
        return err;
    }
    
    token->end += strcspn(token->beg, delims);
    if(token->beg == token->end) {
        token->end += 1;
    }

    return err;
}

Token* create_token() {
    Token* token = malloc(sizeof(Token));
    assert(token && "Failed to allocate memory for token");
    memset(token, 0, sizeof(Token));
    return token;
}

void print_tokens(Token* root) {
    size_t num_tokens = 1;
    while(root) {
        printf("Token %zu: %.*s\n", num_tokens, (int)(root->end - root->beg), root->beg);
        root = root->next_token;
        ++num_tokens;
    }
}

unsigned int num_tokens(Token* root) {
    unsigned int num_tokens = 0;
    while(root) {
            ++num_tokens; 
            root = root->next_token;
        }
    return num_tokens;
}

void print_usage(char* argv) {
    printf("USAGE: %s <path_to_file>\n", argv);
}

int token_str_cmp(char* str, Token* tok) {
    if(!str || !tok) {
        return 0;
    }
    char* beg = tok->beg;
    while(beg < tok->end) {
        if(*beg != *str) {
            return 0;
        }
        ++beg;
        ++str;
    }
    return 1;
}

Token* tokenize(Token* root, int token_num) {
    Token* temp = root;
    Token* tokens = malloc(sizeof(Token) * token_num);
    for(int i = 0; i < token_num; ++i) {
        memcpy(&tokens[i], temp, sizeof(Token));
        if(token_str_cmp("push", &tokens[i])) {
            tokens[i].token_type = INST;
            tokens[i].val.inst_type = PUSH;
        }
        else if(token_str_cmp("pop", &tokens[i])) {
            tokens[i].token_type = INST;
            tokens[i].val.inst_type = POP;
        }
        else if(token_str_cmp("hlt", &tokens[i])) {
            tokens[i].token_type = INST;
            tokens[i].val.inst_type = HLT;
        }
        else if(token_str_cmp("add", &tokens[i])) {
            tokens[i].token_type = INST;
            tokens[i].val.inst_type = ADD;
        }
        else if(token_str_cmp("mul", &tokens[i])) {
            tokens[i].token_type = INST;
            tokens[i].val.inst_type = MUL;
        }
        else if(token_str_cmp("sub", &tokens[i])) {
            tokens[i].token_type = INST;
            tokens[i].val.inst_type = SUB;
        }
        else if(token_str_cmp("print", &tokens[i])) {
            tokens[i].token_type = INST;
            tokens[i].val.inst_type = PRINT;
        }
        else if(token_str_cmp("var", &tokens[i])) {
            tokens[i].token_type = VAR;
            tokens[i].val.value = atoi(temp->next_token->next_token->next_token->beg);
        }
        else if(token_str_cmp("#", &tokens[i])) {
            tokens[i].token_type = NUM;
            //Get the integer value of the next token which will be after "#"
            tokens[i].val.value = atoi(temp->next_token->beg);
        }
        temp = temp->next_token;
    }
    return tokens;
}
void parse(Token* tokens, int token_num, int** stack) {
    int push_indx = 0;
    for(int i = 0; i < token_num; ++i) {
        if(tokens[i].token_type == INST) {
            switch(tokens[i].val.inst_type) {
                //Push an integer towards the tail of the stack
                case PUSH:
                    if(*stack == NULL) {
                        *stack = malloc(sizeof(int) * 1);
                        if(!stack) {
                            printf("Failed to allocate memory for stack\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    else {
                        *stack = realloc(*stack, sizeof(int) * (push_indx + 1));
                        if(!stack) {
                            printf("Failed to reallocate memory for new stack item\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                    (*stack)[push_indx] = tokens[i+1].val.value;
                    push_indx++;
                    break;
               
                //Pop the tail value
                case POP:
                    if(push_indx > 0) {
                        push_indx--;
                        if(push_indx > 0) {
                            *stack = realloc(*stack, sizeof(int) * (push_indx - 1));
                            if(!stack) {
                                printf("Memory reallocation for popping stack failed\n");
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                    else {
                        free(*stack);
                        *stack = NULL;
                    }
                    break;

                case ADD: {
                    if(!stack || push_indx < 1) { break; } 
                    int add = (*stack)[0];
                    if(push_indx > 1) {
                        for(int x = 1; x < push_indx; ++x) {
                            add += (*stack)[x];
                        }
                    }
                    printf("%d\n", add);
                    break;
                }
                case SUB: {
                    if(!stack || push_indx < 1) { break; }
                    int sub = (*stack)[0];
                    if(push_indx > 1) {
                        for(int x = 1; x < push_indx; ++x) {
                            sub -= (*stack)[x];
                        }
                    }
                    printf("%d\n", sub);
                }
                case MUL: {
                    if(!stack || push_indx < 1) { break; }         
                    int mul = (*stack)[0];
                    if(push_indx > 1) {
                        for(int x = 1; x < push_indx; ++x) {
                            mul *= (*stack)[x];
                        }
                    }
                    printf("%d\n", mul);
                }
                case PRINT:
                    printf("[ ");
                    for(int i = 0; i < push_indx; ++i) {
                        if(i == push_indx - 1) {                            
                            printf("%d ]\n", (*stack)[i]);
                            break;
                        }
                        else {
                            printf("%d, ", (*stack)[i]);
                        }
                    }
                    break;
                case HLT:
                    free(*stack);
                    exit(EXIT_SUCCESS);
            } 

        }

        else if(tokens[i].token_type == VAR) {
            printf("Found var: %d", tokens[i].val.value);
        }
        
    }
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        print_usage(argv[0]);
        exit(1);
    }
    //get file contents
    char* path = argv[1];
    char* contents = get_file_contents(path);

    //lex
    Error err = ok;

    Token* tokens = NULL;
    Token* token_it = tokens;

    Token current_token;
    current_token.beg = contents;
    current_token.end = contents;
    current_token.next_token = NULL;

    while((err = lex(current_token.end, &current_token)).type == ERROR_NONE) {
        size_t token_size = current_token.end - current_token.beg;
        if(token_size == 0) { break; }
    
        if(tokens) {
            token_it->next_token = create_token();
            memcpy(token_it->next_token, &current_token, sizeof(Token));
            token_it = token_it->next_token;
        }
        else {
            tokens = create_token();
            memcpy(tokens, &current_token, sizeof(Token));
            token_it = tokens;
        }
    }
   
    //Tokenize
    unsigned int token_count = num_tokens(tokens);
    Token* token_arr = tokenize(tokens, token_count);

    //parse
    int* stack = NULL;
    parse(token_arr, token_count, &stack);

    return EXIT_SUCCESS;
}
