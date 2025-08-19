typedef enum Type {
    DEFINE,
    STRUCT,
    ENUM,
    FUNC,
    GLOBAL_VAR,
} Type;

typedef struct TypedVar {
    char        *type;
    char        *ident;
} TypedVar;

typedef struct TypedStruct {
    char        *ident;
    TypedVar    *vars;
    long long   len;
} TypedStruct;

typedef struct TypedEnum {
    char        *ident;
    char        **types;
    long long   len;
} TypedEnum;

typedef struct TypedFunc {
    char        *ret_type;
    char        *ident;

    TypedVar    *args;
    long long   len;
} TypedFunc;

typedef struct Expr {
    Type type;

    union {
        TypedVar        *var;
        TypedFunc       *tf;
        TypedStruct     *ts;
        TypedEnum       *te;
    };
} Expr;

int main(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
    }

    return 0;
}
