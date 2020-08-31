/// TEST PLOTS
/*c
t = time()

plot_title("Triangle wave")
plot_yaxis("", -2, 2)
plot_xaxis("", -10, 10)
plot((x + t)/abs(x + t) * sin(x + t)/abs(sin(x + t)) * (fmod(abs(x + t), pi) - pi/2))

plot_title("Sawtooth wave")
plot_yaxis("", -2, 2)
plot_xaxis("", -10, 10)
plot(cos(x + t)*sin(x + t)*abs(cos(x + t)*sin(x + t))**((cos(x + t)**4096 - sin(x + t)**2)**2 - 1))

plot_title("Square wave")
plot_yaxis("", -2, 2)
plot_xaxis("", -10, 10)
plot(sin(x + t) * abs(sin(x + t))**(cos(x + t)**4096 - 1))
*/

struct Number
{
    bool is_float;
    
    I64 integer;
    F64 floating;
};

enum CALC_NODE_KIND
{
    CalcNode_Invalid = 0,
    
    CalcNode_Add,
    CalcNode_Sub,
    CalcNode_Mul,
    CalcNode_Div,
    CalcNode_Mod,
    CalcNode_Pow,
    CalcNode_BitAnd,
    CalcNode_BitOr,
    CalcNode_BitXor,
    CalcNode_BitLShift,
    CalcNode_BitRShift,
    CalcNode_And,
    CalcNode_Or,
    
    CalcNode_Neg,
    CalcNode_BitNot,
    CalcNode_Not,
    
    CalcNode_IsEqual,
    CalcNode_IsLess,
    CalcNode_IsLessOrEQ,
    CalcNode_IsGreater,
    CalcNode_IsGreaterOrEQ,
    
    CalcNode_Call,
    CalcNode_Variable,
    CalcNode_String,
    CalcNode_Number,
    CalcNode_Subscript,
    
    CALC_NODE_KIND_COUNT
};

#define CALC_FUNC_MAX_ARG_COUNT 8

struct Calc_Node
{
    U8 kind;
    
    union
    {
        struct
        {
            Calc_Node* left;
            Calc_Node* right;
        };
        
        Calc_Node* operand;
        
        struct
        {
            String name;
            Calc_Node* args[CALC_FUNC_MAX_ARG_COUNT];
            U32 arg_count;
        } call;
        
        struct
        {
            String name;
            bool is_global;
        } var;
        
        String string;
        Number number;
    };
};

struct Calc_Var
{
    String name;
    Number value;
};

struct Calc_Plot
{
    I64 pos;
    String title;
    String x_label;
    String y_label;
    F64 min_x;
    F64 max_x;
    F64 min_y;
    F64 max_y;
    bool is_active;
    Calc_Node* funcs[CALC_FUNC_MAX_ARG_COUNT];
    U32 func_count;
};


Calc_Var CalcVariables[256] = {
    {CONST_STRING("x"),   {}},
    {CONST_STRING("pi"),  {true, 3, 3.1415926535}},
    {CONST_STRING("tau"), {true, 6, 6.2831853071}},
    {CONST_STRING("e"),   {true, 2, 2.7182818284}},
};

#define CALC_RESERVED_VAR_NAMES_COUNT 4
#define CALC_X_VAR_INDEX 0

U32 CalcVariableCount       = 0;
F64 CalcTime                = 0;

#define CALC_FUNC_LIST()                                                 \
CALC_FUNC(Int, 1,                                                    \
{                                                          \
Number result;                                         \
result.is_float = false;                               \
if (args[0].is_float)                                  \
{                                                      \
result.integer  =      (I64)args[0].floating;      \
result.floating = (F64)(I64)args[0].floating;      \
}                                                      \
else                                                   \
{                                                      \
result.integer  = (I64)args[0].integer;            \
result.floating = (F64)args[0].integer;            \
}                                                      \
return result;                                         \
})                                                         \
CALC_FUNC(Float, 1,                                                  \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
if (args[0].is_float)                                  \
{                                                      \
result.integer  = (I64)args[0].floating;           \
result.floating = (F64)args[0].floating;           \
}                                                      \
else                                                   \
{                                                      \
result.integer  = (I64)args[0].integer;            \
result.floating = (F64)args[0].integer;            \
}                                                      \
return result;                                         \
})                                                         \
CALC_FUNC(Pow, 2,                                                    \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = pow((F64)CALC_GET_ARG(0),            \
(F64)CALC_GET_ARG(1));           \
return result;                                         \
})                                                         \
CALC_FUNC(Sqrt, 1,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = sqrt((F64)CALC_GET_ARG(0));          \
return result;                                         \
})                                                         \
CALC_FUNC(Cbrt, 1,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = cbrt((F64)CALC_GET_ARG(0));          \
return result;                                         \
})                                                         \
CALC_FUNC(Abs, 1,                                                    \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = abs((F64)CALC_GET_ARG(0));           \
return result;                                         \
})                                                         \
CALC_FUNC(Time, 0,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = CalcTime;                            \
return result;                                         \
})                                                         \
CALC_FUNC(Sin, 1,                                                    \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = sin((F64)CALC_GET_ARG(0));           \
return result;                                         \
})                                                         \
CALC_FUNC(Cos, 1,                                                    \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = cos((F64)CALC_GET_ARG(0));           \
return result;                                         \
})                                                         \
CALC_FUNC(Tan, 1,                                                    \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = tan((F64)CALC_GET_ARG(0));           \
return result;                                         \
})                                                         \
CALC_FUNC(ASin, 1,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = asin((F64)CALC_GET_ARG(0));          \
return result;                                         \
})                                                         \
CALC_FUNC(ACos, 1,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = acos((F64)CALC_GET_ARG(0));          \
return result;                                         \
})                                                         \
CALC_FUNC(ATan, 1,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = atan((F64)CALC_GET_ARG(0));          \
return result;                                         \
})                                                         \
CALC_FUNC(ATan2, 2,                                                  \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = atan2((F64)CALC_GET_ARG(0),          \
(F64)CALC_GET_ARG(1));         \
return result;                                         \
})                                                         \
CALC_FUNC(Sinh, 1,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = sinh((F64)CALC_GET_ARG(0));          \
return result;                                         \
})                                                         \
CALC_FUNC(Cosh, 1,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = cosh((F64)CALC_GET_ARG(0));          \
return result;                                         \
})                                                         \
CALC_FUNC(Tanh, 1,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = tanh((F64)CALC_GET_ARG(0));          \
return result;                                         \
})                                                         \
CALC_FUNC(ASinh, 1,                                                  \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = asinh((F64)CALC_GET_ARG(0));         \
return result;                                         \
})                                                         \
CALC_FUNC(ACosh, 1,                                                  \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = acosh((F64)CALC_GET_ARG(0));         \
return result;                                         \
})                                                         \
CALC_FUNC(ATanh, 1,                                                  \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = atanh((F64)CALC_GET_ARG(0));         \
return result;                                         \
})                                                         \
CALC_FUNC(Exp, 1,                                                    \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = exp((F64)CALC_GET_ARG(0));           \
return result;                                         \
})                                                         \
CALC_FUNC(Log, 1,                                                    \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = log((F64)CALC_GET_ARG(0));           \
return result;                                         \
})                                                         \
CALC_FUNC(Log10, 1,                                                  \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = log10((F64)CALC_GET_ARG(0));         \
return result;                                         \
})                                                         \
CALC_FUNC(Log2, 1,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = log2((F64)CALC_GET_ARG(0));          \
return result;                                         \
})                                                         \
CALC_FUNC(Exp2, 1,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = exp2((F64)CALC_GET_ARG(0));          \
return result;                                         \
})                                                         \
CALC_FUNC(Ceil, 1,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = ceil((F64)CALC_GET_ARG(0));          \
return result;                                         \
})                                                         \
CALC_FUNC(Floor, 1,                                                  \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = floor((F64)CALC_GET_ARG(0));         \
return result;                                         \
})                                                         \
CALC_FUNC(FMod, 2,                                                   \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = fmod((F64)CALC_GET_ARG(0),           \
(F64)CALC_GET_ARG(1));          \
return result;                                         \
})                                                         \
CALC_FUNC(Trunc, 1,                                                  \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = trunc((F64)CALC_GET_ARG(0));         \
return result;                                         \
})                                                         \
CALC_FUNC(Round, 1,                                                  \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = round((F64)CALC_GET_ARG(0));         \
return result;                                         \
})                                                         \
CALC_FUNC(IsNan, 1,                                                  \
{                                                          \
Number result;                                         \
result.is_float = false;                               \
result.integer  = isnan((F64)CALC_GET_ARG(0));         \
result.floating = (F64)result.integer;                 \
return result;                                         \
})                                                         \
CALC_FUNC(IsInf, 1,                                                  \
{                                                          \
Number result;                                         \
result.is_float = false;                               \
result.integer  = isinf((F64)CALC_GET_ARG(0));         \
result.floating = (F64)result.integer;                 \
return result;                                         \
})                                                         \
CALC_FUNC(Nan, 0,                                                    \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = NAN;                                 \
return result;                                         \
})                                                         \
CALC_FUNC(Inf, 0,                                                    \
{                                                          \
Number result;                                         \
result.is_float = true;                                \
result.floating = INFINITY;                            \
return result;                                         \
})                                                         \
CALC_FUNC(max, 2,                                                    \
{                                                          \
Number result;                                         \
if (args[0].is_float || args[1].is_float)              \
{                                                      \
result.is_float = true;                            \
result.floating = fmax((F64)CALC_GET_ARG(0),       \
(F64)CALC_GET_ARG(1));      \
}                                                      \
else                                                   \
{                                                      \
result.is_float = false;                           \
result.integer  = MAX((I64)CALC_GET_ARG(0),        \
(I64)CALC_GET_ARG(1));       \
result.floating = (F64)result.integer;             \
}                                                      \
return result;                                         \
})                                                         \
CALC_FUNC(min, 2,                                                    \
{                                                          \
Number result;                                         \
if (args[0].is_float || args[1].is_float)              \
{                                                      \
result.is_float = true;                            \
result.floating = fmin((F64)CALC_GET_ARG(0),       \
(F64)CALC_GET_ARG(1));      \
}                                                      \
else                                                   \
{                                                      \
result.is_float = false;                           \
result.integer  = MIN((I64)CALC_GET_ARG(0),        \
(I64)CALC_GET_ARG(1));       \
result.floating = (F64)result.integer;             \
}                                                      \
return result;                                         \
})                                                         \


#define CALC_FUNC(name, arg_count, body) Number name (Number args[CALC_FUNC_MAX_ARG_COUNT]) body
#define CALC_GET_ARG(n) (args[(n)].is_float ? args[(n)].floating : args[(n)].integer)
CALC_FUNC_LIST()
#undef CALC_FUNC

Calc_Node*
AddCalcNode(Memory_Arena* arena, U8 kind)
{
    Calc_Node* node = (Calc_Node*)Arena_Allocate(arena, sizeof(Calc_Node), alignof(Calc_Node));
    ZeroStruct(node);
    
    node->kind = kind;
    
    return node;
}

bool ParsePrimaryExpr(Memory_Arena* arena, String* string, Calc_Node** result);
bool ParseLogicalOrExpr(Memory_Arena* arena, String* string, Calc_Node** result);

bool
ParsePostExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    while (!encountered_errors)
    {
        EatAllWhitespace(string);
        
        if (string->size <= 1) break;
        else
        {
            if (string->data[0] == '*' && string->data[1] == '*')
            {
                Advance(string, 2);
                
                Calc_Node* base = *result;
                
                *result = AddCalcNode(arena, CalcNode_Pow);
                (*result)->left = base;
                
                if (!ParsePrimaryExpr(arena, string, &(*result)->right))
                {
                    encountered_errors = true;
                }
            }
            
            else if (string->data[0] == '[')
            {
                Advance(string, 1);
                
                Calc_Node* pointer = *result;
                
                *result = AddCalcNode(arena, CalcNode_Subscript);
                (*result)->left = pointer;
                
                if (!ParsePrimaryExpr(arena, string, &(*result)->right)) encountered_errors = true;
                else
                {
                    EatAllWhitespace(string);
                    
                    if (string->size != 0 && string->data[0] == ']') Advance(string, 1);
                    else
                    {
                        encountered_errors = true;
                    }
                }
            }
            
            else if (string->data[0] == '(')
            {
                Advance(string, 1);
                
                if ((*result)->kind != CalcNode_Variable || (*result)->var.is_global) encountered_errors = true;
                else
                {
                    String name = (*result)->var.name;
                    
                    *result = AddCalcNode(arena, CalcNode_Call);
                    (*result)->call.name = name;
                    
                    EatAllWhitespace(string);
                    
                    if (string->size != 0 && string->data[0] != ')')
                        while (!encountered_errors)
                    {
                        if ((*result)->call.arg_count >= CALC_FUNC_MAX_ARG_COUNT) encountered_errors = true;
                        else
                        {
                            if (!ParseLogicalOrExpr(arena, string, &(*result)->call.args[(*result)->call.arg_count++]))
                            {
                                encountered_errors = true;
                            }
                            
                            else
                            {
                                EatAllWhitespace(string);
                                
                                if (string->size != 0 && string->data[0] == ',') Advance(string, 1);
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }
                    
                    EatAllWhitespace(string);
                    if (string->size != 0 && string->data[0] == ')') Advance(string, 1);
                    else
                    {
                        encountered_errors = true;
                    }
                }
            }
            
            else break;
        }
    }
    
    return !encountered_errors;
}

bool ParseLogicalOrExpr(Memory_Arena* arena, String* string, Calc_Node** result);

bool
ParsePrimaryExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    EatAllWhitespace(string);
    
    if (string->size == 0) encountered_errors = true;
    else
    {
        if (IsAlpha(string->data[0]) || string->data[0] == '_' || string->data[0] == '@')
        {
            bool is_global = false;
            
            if (string->data[0] == '@')
            {
                is_global = true;
                
                Advance(string, 1);
            }
            
            if (!IsAlpha(string->data[0]) && string->data[0] != '_') encountered_errors = true;
            else
            {
                String identifier;
                identifier.data = string->data;
                identifier.size = 0;
                
                while (string->size != 0 &&
                       (IsAlpha(string->data[0]) ||
                        IsDigit(string->data[0]) ||
                        string->data[0] == '_'))
                {
                    identifier.size += 1;
                    Advance(string, 1);
                }
                
                if (identifier.size == 0) encountered_errors = true;
                else
                {
                    if (StringCompare(identifier, CONST_STRING("true"), false) ||
                        StringCompare(identifier, CONST_STRING("false"), false))
                    {
                        *result = AddCalcNode(arena, CalcNode_Number);
                        (*result)->number.is_float = false;
                        (*result)->number.integer  = StringCompare(identifier, CONST_STRING("true"));
                        (*result)->number.floating = (F64)(*result)->number.integer;
                    }
                    
                    else
                    {
                        *result = AddCalcNode(arena, CalcNode_Variable);
                        (*result)->var.name      = identifier;
                        (*result)->var.is_global = is_global;
                    }
                }
            }
        }
        
        else if (string->data[0] == '"')
        {
            Advance(string, 1);
            
            *result = AddCalcNode(arena, CalcNode_String);
            (*result)->string.data = string->data;
            
            while (string->size != 0 && string->data[0] != '"')
            {
                Advance(string, 1);
            }
            
            if (string->size == 0 || string->data[0] != '"') encountered_errors = true;
            else
            {
                (*result)->string.size = string->data - (*result)->string.data;
                Advance(string, 1);
            }
        }
        
        else if (IsDigit(string->data[0]) || string->data[0] == '.')
        {
            *result = AddCalcNode(arena, CalcNode_Number);
            
            Number* number = &(*result)->number;
            number->is_float = false;
            number->integer  = 0;
            number->floating = 0;
            
            bool is_hex    = false;
            bool is_binary = false;
            
            if (string->data[0] == '0' && string->size > 1)
            {
                if      (string->data[1] == 'x') is_hex    = true;
                else if (string->data[1] == 'b') is_binary = true;
                
                if (is_hex || is_binary) Advance(string, 2);
            }
            
            else if (string->data[0] == '.')
            {
                number->is_float = true;
                Advance(string, 1);
            }
            
            if (string->size == 0) encountered_errors = true;
            else
            {
                U8 base = (is_hex ? 16 : (is_binary ? 2 : 10));
                
                F64 flt_place = 1;
                while (!encountered_errors && string->size != 0)
                {
                    U8 digit = 0;
                    
                    if (number->is_float) flt_place /= 10;
                    
                    if (IsDigit(string->data[0]))
                    {
                        digit = string->data[0] - '0';
                        Advance(string, 1);
                        
                        if (digit >= base) encountered_errors = true;
                    }
                    
                    else if (is_hex && ToUpper(string->data[0]) >= 'A' && ToUpper(string->data[0]) <= 'F')
                    {
                        digit = (ToUpper(string->data[0]) - 'A') + 10;
                        Advance(string, 1);
                    }
                    
                    else if (!number->is_float && string->data[0] == '.')
                    {
                        number->is_float = true;
                        Advance(string, 1);
                        
                        continue;
                    }
                    
                    else break;
                    
                    number->integer  = number->integer * base + digit;
                    number->floating = number->floating * (number->is_float ? 1 : base) + digit * flt_place;
                }
            }
        }
        
        else if (string->data[0] == '(')
        {
            Advance(string, 1);
            
            if (!ParseLogicalOrExpr(arena, string, result)) encountered_errors = true;
            else
            {
                if (string->size != 0 && string->data[0] == ')') Advance(string, 1);
                else encountered_errors = true;
            }
        }
        
        else encountered_errors = true;
    }
    
    if (!encountered_errors)
    {
        if (!ParsePostExpr(arena, string, result))
        {
            encountered_errors = true;
        }
    }
    
    return !encountered_errors;
}

bool
ParseUnaryExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    Calc_Node** current = result;
    while (!encountered_errors)
    {
        EatAllWhitespace(string);
        
        if (string->size >= 1 && string->data[0] == '+') Advance(string, 1);
        
        else if (string->size >= 1 && string->data[0] == '-')
        {
            Advance(string, 1);
            
            *current = AddCalcNode(arena, CalcNode_Neg);
            current = &(*current)->operand;
        }
        
        else if (string->size >= 1 && string->data[0] == '~')
        {
            Advance(string, 1);
            
            *current = AddCalcNode(arena, CalcNode_BitNot);
            current = &(*current)->operand;
        }
        
        else if (string->size >= 1 && string->data[0] == '!')
        {
            Advance(string, 1);
            
            *current = AddCalcNode(arena, CalcNode_Not);
            current = &(*current)->operand;
        }
        
        else
        {
            if (ParsePrimaryExpr(arena, string, current)) break;
            else
            {
                encountered_errors = true;
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseMulLevelExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    Calc_Node** current = result;
    while (!encountered_errors)
    {
        if (!ParseUnaryExpr(arena, string, (*current != 0 ? &(*current)->right : current))) encountered_errors = true;
        else
        {
            EatAllWhitespace(string);
            
            U8 kind = CalcNode_Invalid;
            
            if (string->size >= 1                            &&
                !(string->size >= 2 && string->data[1] == '=') &&
                !(string->size >= 3 && string->data[0] == string->data[1] && string->data[2] == '='))
            {
                if      (string->data[0] == '*') kind = CalcNode_Mul;
                else if (string->data[0] == '/') kind = CalcNode_Div;
                else if (string->data[0] == '%') kind = CalcNode_Mod;
                else if (string->data[0] == '^') kind = CalcNode_BitXor;
                
                else if (string->data[0] == '&' &&
                         (string->size < 2 || string->data[1] != '&'))
                {
                    kind = CalcNode_BitAnd;
                }
                
                else if (string->size >= 2 && string->data[0] == string->data[1])
                {
                    if      (string->data[0] == '<') kind = CalcNode_BitLShift;
                    else if (string->data[0] == '>') kind = CalcNode_BitRShift;
                }
            }
            
            if (kind == CalcNode_Invalid) break;
            else
            {
                if (kind == CalcNode_BitLShift || kind == CalcNode_BitRShift) Advance(string, 2);
                else                                                          Advance(string, 1);
                
                Calc_Node* lhs = *current;
                
                *current = AddCalcNode(arena, kind);
                (*current)->left = lhs;
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseAddLevelExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    Calc_Node** current = result;
    while (!encountered_errors)
    {
        if (!ParseMulLevelExpr(arena, string, (*current != 0 ? &(*current)->right : current))) encountered_errors = true;
        else
        {
            EatAllWhitespace(string);
            
            U8 kind = CalcNode_Invalid;
            
            if (string->size >= 1 &&
                !(string->size >= 2 && string->data[1] == '='))
            {
                
                if      (string->data[0] == '+')                           kind = CalcNode_Add;
                else if (string->data[0] == '-')                           kind = CalcNode_Sub;
                else if (string->data[0] == '|' && string->data[1] != '|') kind = CalcNode_BitOr;
            }
            
            if (kind == CalcNode_Invalid) break;
            else
            {
                Advance(string, 1);
                
                Calc_Node* lhs = *current;
                
                *current = AddCalcNode(arena, kind);
                (*current)->left = lhs;
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseComparisonExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    Calc_Node** current = result;
    while (!encountered_errors)
    {
        if (!ParseAddLevelExpr(arena, string, (*current != 0 ? &(*current)->right : current))) encountered_errors = true;
        else
        {
            EatAllWhitespace(string);
            
            U8 kind = CalcNode_Invalid;
            
            if (string->size >= 1)
            {
                if (string->size > 1 && string->data[1] == '=')
                {
                    if      (string->data[0] == '=') kind = CalcNode_IsEqual;
                    else if (string->data[0] == '<') kind = CalcNode_IsLessOrEQ;
                    else if (string->data[0] == '>') kind = CalcNode_IsGreaterOrEQ;
                }
                
                else
                {
                    if      (string->data[0] == '<') kind = CalcNode_IsLess;
                    else if (string->data[0] == '>') kind = CalcNode_IsGreater;
                }
            }
            
            if (kind == CalcNode_Invalid) break;
            else
            {
                if (kind == CalcNode_IsLess || kind == CalcNode_IsGreater) Advance(string, 1);
                else                                                       Advance(string, 2);
                
                Calc_Node* lhs = *current;
                
                *current = AddCalcNode(arena, kind);
                (*current)->left = lhs;
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseLogicalAndExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    Calc_Node** current = result;
    while (!encountered_errors)
    {
        if (!ParseComparisonExpr(arena, string, (*current != 0 ? &(*current)->right : current))) encountered_errors = true;
        else
        {
            EatAllWhitespace(string);
            
            if (string->size > 1       &&
                string->data[0] == '&' &&
                string->data[1] == '&' &&
                (string->size < 3 || string->data[2] != '='))
            {
                Advance(string, 2);
                
                Calc_Node* lhs = *current;
                
                *current = AddCalcNode(arena, CalcNode_And);
                (*current)->left = lhs;
            }
            
            else break;
        }
    }
    
    return !encountered_errors;
}

bool
ParseLogicalOrExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    Calc_Node** current = result;
    while (!encountered_errors)
    {
        if (!ParseLogicalAndExpr(arena, string, (*current != 0 ? &(*current)->right : current))) encountered_errors = true;
        else
        {
            EatAllWhitespace(string);
            
            if (string->size > 1       &&
                string->data[0] == '|' &&
                string->data[1] == '|' &&
                (string->size < 3 || string->data[2] != '='))
            {
                Advance(string, 2);
                
                Calc_Node* lhs = *current;
                
                *current = AddCalcNode(arena, CalcNode_Or);
                (*current)->left = lhs;
            }
            
            else break;
        }
    }
    
    return !encountered_errors;
}

bool
EvalCalcNode(Calc_Node* node, Number* number)
{
    bool encountered_errors = false;
    
    if (node->kind == CalcNode_Number) *number = node->number;
    
    else if (node->kind == CalcNode_String) encountered_errors = true;
    
    else if (node->kind == CalcNode_Variable)
    {
        if (node->var.is_global)
        {
            NOT_IMPLEMENTED;
        }
        
        else
        {
            I64 var_index = -1;
            for (U32 i = 0; i < CalcVariableCount; ++i)
            {
                if (StringCompare(node->var.name, CalcVariables[i].name))
                {
                    var_index = i;
                    break;
                }
            }
            
            if (var_index == -1) encountered_errors = true;
            else
            {
                *number = CalcVariables[var_index].value;
            }
        }
    }
    
    else if (node->kind == CalcNode_Add        ||
             node->kind == CalcNode_Sub        ||
             node->kind == CalcNode_Mul        ||
             node->kind == CalcNode_Div        ||
             node->kind == CalcNode_Pow        ||
             node->kind == CalcNode_IsEqual    ||
             node->kind == CalcNode_IsLess     ||
             node->kind == CalcNode_IsLessOrEQ ||
             node->kind == CalcNode_IsGreater  ||
             node->kind == CalcNode_IsGreaterOrEQ)
    {
        Number lhs, rhs;
        if (!EvalCalcNode(node->left, &lhs) || !EvalCalcNode(node->right, &rhs)) encountered_errors = true;
        else
        {
            number->is_float = (lhs.is_float || rhs.is_float);
            
            if (node->kind == CalcNode_Add)
            {
                number->integer  = lhs.integer  + rhs.integer;
                number->floating = lhs.floating + rhs.floating;
            }
            
            else if (node->kind == CalcNode_Sub)
            {
                number->integer  = lhs.integer  - rhs.integer;
                number->floating = lhs.floating - rhs.floating;
            }
            
            else if (node->kind == CalcNode_Mul)
            {
                number->integer  = lhs.integer  * rhs.integer;
                number->floating = lhs.floating * rhs.floating;
            }
            
            else if (node->kind == CalcNode_Div)
            {
                if (rhs.is_float && rhs.floating == 0 || !rhs.is_float && rhs.integer == 0)
                {
                    number->is_float = true;
                    number->floating = NAN;
                }
                
                else
                {
                    number->integer  = lhs.integer  / rhs.integer;
                    number->floating = lhs.floating / rhs.floating;
                }
            }
            
            else if (node->kind == CalcNode_Pow)
            {
                number->is_float = true;
                
                number->floating = pow(lhs.floating, rhs.floating);
            }
            
            else
            {
                number->is_float = false;
                
                F64 lhs_flt = (F64)(lhs.is_float ? lhs.floating : lhs.integer);
                F64 rhs_flt = (F64)(rhs.is_float ? rhs.floating : rhs.integer);
                
                if (node->kind == CalcNode_IsEqual)
                {
                    
                    if (lhs.is_float || rhs.is_float)
                    {
                        number->integer  = (lhs_flt == rhs_flt);
                        number->floating = (lhs_flt == rhs_flt);
                    }
                    
                    else
                    {
                        number->integer  = (lhs.integer == rhs.integer);
                        number->floating = (lhs.integer == rhs.integer);
                    }
                }
                
                else if (node->kind == CalcNode_IsLess)
                {
                    if (lhs.is_float || rhs.is_float)
                    {
                        number->integer  = (lhs_flt < rhs_flt);
                        number->floating = (lhs_flt < rhs_flt);
                    }
                    
                    else
                    {
                        number->integer  = (lhs.integer < rhs.integer);
                        number->floating = (lhs.integer < rhs.integer);
                    }
                }
                
                else if (node->kind == CalcNode_IsLessOrEQ)
                {
                    if (lhs.is_float || rhs.is_float)
                    {
                        number->integer  = (lhs_flt <= rhs_flt);
                        number->floating = (lhs_flt <= rhs_flt);
                    }
                    
                    else
                    {
                        number->integer  = (lhs.integer <= rhs.integer);
                        number->floating = (lhs.integer <= rhs.integer);
                    }
                }
                
                else if (node->kind == CalcNode_IsGreater)
                {
                    if (lhs.is_float || rhs.is_float)
                    {
                        number->integer  = (lhs_flt > rhs_flt);
                        number->floating = (lhs_flt > rhs_flt);
                    }
                    
                    else
                    {
                        number->integer  = (lhs.integer > rhs.integer);
                        number->floating = (lhs.integer > rhs.integer);
                    }
                }
                
                else
                {
                    if (lhs.is_float || rhs.is_float)
                    {
                        number->integer  = (lhs_flt >= rhs_flt);
                        number->floating = (lhs_flt >= rhs_flt);
                    }
                    
                    else
                    {
                        number->integer  = (lhs.integer >= rhs.integer);
                        number->floating = (lhs.integer >= rhs.integer);
                    }
                }
            }
        }
    }
    
    else if (node->kind == CalcNode_BitAnd     ||
             node->kind == CalcNode_BitOr      ||
             node->kind == CalcNode_BitXor     ||
             node->kind == CalcNode_BitLShift  ||
             node->kind == CalcNode_BitRShift  ||
             node->kind == CalcNode_And        ||
             node->kind == CalcNode_Or)
    {
        Number lhs, rhs;
        if (!EvalCalcNode(node->left, &lhs) || !EvalCalcNode(node->right, &rhs)) encountered_errors = true;
        else
        {
            if (lhs.is_float || rhs.is_float) encountered_errors = true;
            else
            {
                number->is_float = false;
                
                if (node->kind == CalcNode_BitAnd)
                {
                    number->integer  = lhs.integer & rhs.integer;
                    number->floating = (F64)number->integer;
                }
                
                else if (node->kind == CalcNode_BitOr)
                {
                    number->integer  = lhs.integer | rhs.integer;
                    number->floating = (F64)number->integer;
                }
                
                else if (node->kind == CalcNode_BitXor)
                {
                    number->integer  = lhs.integer ^ rhs.integer;
                    number->floating = (F64)number->integer;
                }
                
                else if (node->kind == CalcNode_BitLShift)
                {
                    number->integer  = lhs.integer << rhs.integer;
                    number->floating = (F64)number->integer;
                }
                
                else if (node->kind == CalcNode_BitRShift)
                {
                    number->integer  = lhs.integer >> rhs.integer;
                    number->floating = (F64)number->integer;
                }
                
                else if (node->kind == CalcNode_And)
                {
                    number->integer  = lhs.integer && rhs.integer;
                    number->floating = (F64)number->integer;
                }
                
                else if (node->kind == CalcNode_Or)
                {
                    number->integer  = lhs.integer || rhs.integer;
                    number->floating = (F64)number->integer;
                }
            }
        }
    }
    
    else if (node->kind == CalcNode_Neg    ||
             node->kind == CalcNode_BitNot ||
             node->kind == CalcNode_Not)
    {
        Number op;
        if (!EvalCalcNode(node->operand, &op)) encountered_errors = true;
        else
        {
            if (node->kind == CalcNode_Neg)
            {
                number->is_float = op.is_float;
                
                if (op.is_float)
                {
                    number->floating = -op.floating;
                }
                
                else
                {
                    number->integer  = -op.integer;
                    number->floating = (F64)number->integer;
                }
            }
            
            else
            {
                if (op.is_float) encountered_errors = true;
                else
                {
                    number->is_float = false;
                    
                    if (node->kind == CalcNode_BitNot)
                    {
                        number->integer  = ~op.integer;
                        number->floating = (F64)number->integer;
                    }
                    
                    else
                    {
                        number->integer  = !op.integer;
                        number->floating = (F64)number->integer;
                    }
                }
            }
        }
    }
    
    else if (node->kind == CalcNode_Call)
    {
        Number args[CALC_FUNC_MAX_ARG_COUNT] = {};
        U32 arg_count                        = 0;
        
        for (U32 i = 0; !encountered_errors  && i < node->call.arg_count; ++i)
        {
            if (EvalCalcNode(node->call.args[i], &args[i])) ++arg_count;
            else
            {
                encountered_errors = true;
            }
        }
        
        if (!encountered_errors) do
        {
            if (StringCompare(node->call.name, CONST_STRING("plot"), false)) encountered_errors = true;
            
#define CALC_FUNC(func_name, req_arg_count, body)                                   \
else if (StringCompare(node->call.name, CONST_STRING(#func_name), false))       \
{                                                                               \
if (arg_count != (req_arg_count)) { encountered_errors = true; break; }     \
*number = func_name(args);                                                  \
}
            
            CALC_FUNC_LIST()
                
#undef CALC_FUNC
            
            
            
            else encountered_errors = true;
            
        } while (0);
    }
    
    else encountered_errors = true;
    
    return !encountered_errors;
}

void
RenderCalcComment(Application_Links* app, View_ID view, Text_Layout_ID text_layout_id, Face_ID face_id, Frame_Info frame_info,
                  I64 base_pos, I64 cursor_pos, String string)
{
    Memory_Arena arena = {};
    
    Face_Metrics metrics = get_face_metrics(app, face_id);
    U8* start            = string.data;
    
    CalcTime += frame_info.literal_dt;
    
    char char_buffer[256] = {};
    
    Calc_Plot plot_buffer[256] = {};
    U32 plot_count = 0;
    
    Zero(&CalcVariables[CALC_RESERVED_VAR_NAMES_COUNT], sizeof(CalcVariables) - CALC_RESERVED_VAR_NAMES_COUNT);
    CalcVariableCount = CALC_RESERVED_VAR_NAMES_COUNT;
    
    ZeroStruct(&CalcVariables[CALC_X_VAR_INDEX].value);
    
    ARGB_Color alarm_color = pack_color(V4f32(1.0f, 0.2f, 0.2f, 1.0f));
    
    bool encountered_errors = false;
    
    while (!encountered_errors)
    {
        while (string.size != 0 && (string.data[0] == ' ' ||
                                    string.data[0] == '\t' ||
                                    string.data[0] == '\v' ||
                                    string.data[0] == '\r' ||
                                    string.data[0] == '\n'))
        {
            Advance(&string, 1);
        }
        
        if (string.size == 0) break;
        
        bool is_assignment        = false;
        bool is_pure_assignment   = false;
        Calc_Node* assignment_lhs = 0;
        
        Calc_Node* current = 0;
        if (!ParseLogicalOrExpr(&arena, &string, &current)) encountered_errors = true;
        else
        {
            EatAllWhitespace(&string);
            
            if (string.size != 0 && string.data[0] == '=' ||
                string.size >= 2 && string.data[1] == '=' ||
                string.size >= 3 && string.data[2] == '=')
            {
                U8 kind = CalcNode_Invalid;
                
                if (string.size >= 2 && string.data[1] == '=')
                {
                    if      (string.data[0] == '+') kind = CalcNode_Add;
                    else if (string.data[0] == '-') kind = CalcNode_Sub;
                    else if (string.data[0] == '*') kind = CalcNode_Mul;
                    else if (string.data[0] == '/') kind = CalcNode_Div;
                    else if (string.data[0] == '%') kind = CalcNode_Mod;
                    else if (string.data[0] == '&') kind = CalcNode_BitAnd;
                    else if (string.data[0] == '|') kind = CalcNode_BitOr;
                    else if (string.data[0] == '~') kind = CalcNode_BitNot;
                    else if (string.data[0] == '^') kind = CalcNode_BitXor;
                }
                
                else if (string.size >= 3 && string.data[0] == string.data[1])
                {
                    if      (string.data[0] == '&') kind = CalcNode_And;
                    else if (string.data[0] == '|') kind = CalcNode_Or;
                    else if (string.data[0] == '<') kind = CalcNode_BitLShift;
                    else if (string.data[0] == '>') kind = CalcNode_BitRShift;
                }
                
                if (kind == CalcNode_Invalid && string.data[0] != '=') encountered_errors = true;
                else
                {
                    Advance(&string, (kind == CalcNode_Invalid ? 1 : 2));
                    
                    is_assignment  = true;
                    assignment_lhs = current;
                    
                    if (kind != CalcNode_Invalid)
                    {
                        Calc_Node* lhs = current;
                        
                        current = AddCalcNode(&arena, kind);
                        current->left = lhs;
                        
                        if (!ParseLogicalOrExpr(&arena, &string, &current->right))
                        {
                            encountered_errors = true;
                        }
                    }
                    
                    else
                    {
                        is_pure_assignment = true;
                        
                        current = 0;
                        if (!ParseLogicalOrExpr(&arena, &string, &current))
                        {
                            encountered_errors = true;
                        }
                    }
                }
            }
            
            EatAllWhitespace(&string);
            
            if (string.size != 0 && string.data[0] != '\r' && string.data[0] != '\n')
            {
                encountered_errors = true;
            }
        }
        
        I64 pos = base_pos + (string.data - start);
        Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(pos));
        Vec2_f32 p           = view_relative_xy_of_pos(app, view, cursor.line, pos);
        I64 line_start       = view_pos_at_relative_xy(app, view, cursor.line, {0, p.y});
        I64 line_end         = view_pos_at_relative_xy(app, view, cursor.line, {max_f32, p.y});
        
        bool is_active = (cursor_pos >= line_start && cursor_pos <= line_end);
        
        if (!encountered_errors)
        {
            if (current != 0)
            {
                Number number           = {};
                bool should_draw_number = false;
                
                if (is_assignment)
                {
                    if (assignment_lhs->kind != CalcNode_Variable || assignment_lhs->var.is_global) encountered_errors = true;
                    else
                    {
                        I64 var_index = -1;
                        for (U32 i = 0; i < CalcVariableCount; ++i)
                        {
                            if (StringCompare(assignment_lhs->var.name, CalcVariables[i].name))
                            {
                                var_index = i;
                                break;
                            }
                        }
                        
                        bool is_decl = (var_index == -1);
                        if (var_index == -1)
                        {
                            if (CalcVariableCount == ArrayCount(CalcVariables)) encountered_errors = true;
                            else
                            {
                                var_index                     = CalcVariableCount;
                                CalcVariables[var_index].name = assignment_lhs->var.name;
                                
                                CalcVariableCount += 1;
                            }
                        }
                        
                        if (!encountered_errors)
                        {
                            if (is_decl && !is_pure_assignment) encountered_errors = true;
                            else
                            {
                                if (var_index < CALC_RESERVED_VAR_NAMES_COUNT) encountered_errors = true;
                                else
                                {
                                    if (!EvalCalcNode(current, &number)) encountered_errors = true;
                                    else
                                    {
                                        should_draw_number             = true;
                                        CalcVariables[var_index].value = number;
                                    }
                                }
                            }
                        }
                    }
                }
                
                else
                {
                    if (current->kind == CalcNode_Call)
                    {
                        if (StringCompare(current->call.name, CONST_STRING("plot_title")))
                        {
                            if (current->call.arg_count == 1 && current->call.args[0]->kind == CalcNode_String)
                            {
                                plot_buffer[plot_count].title = current->call.args[0]->string;
                                
                                plot_buffer[plot_count].is_active = (plot_buffer[plot_count].is_active || is_active);
                            }
                            
                            else encountered_errors = true;
                        }
                        
                        else if (StringCompare(current->call.name, CONST_STRING("plot_xaxis")))
                        {
                            if (current->call.arg_count != 3) encountered_errors = true;
                            else
                            {
                                Number min_x  = {};
                                Number max_x = {};
                                if (current->call.args[0]->kind == CalcNode_String &&
                                    EvalCalcNode(current->call.args[1], &min_x)    &&
                                    EvalCalcNode(current->call.args[2], &max_x))
                                {
                                    plot_buffer[plot_count].x_label = current->call.args[0]->string;
                                    
                                    plot_buffer[plot_count].min_x   = (F64)(min_x.is_float
                                                                            ? min_x.floating
                                                                            : min_x.integer);
                                    
                                    plot_buffer[plot_count].max_x   = (F64)(max_x.is_float
                                                                            ? max_x.floating
                                                                            : max_x.integer);
                                    
                                    if (plot_buffer[plot_count].min_x >= plot_buffer[plot_count].max_x)
                                    {
                                        encountered_errors = true;
                                    }
                                    
                                    plot_buffer[plot_count].is_active = (plot_buffer[plot_count].is_active || is_active);
                                }
                                
                                else encountered_errors = true;
                            }
                        }
                        
                        else if (StringCompare(current->call.name, CONST_STRING("plot_yaxis")))
                        {
                            if (current->call.arg_count != 3) encountered_errors = true;
                            else
                            {
                                Number min_y  = {};
                                Number max_y = {};
                                if (current->call.args[0]->kind == CalcNode_String &&
                                    EvalCalcNode(current->call.args[1], &min_y)    &&
                                    EvalCalcNode(current->call.args[2], &max_y))
                                {
                                    plot_buffer[plot_count].y_label = current->call.args[0]->string;
                                    
                                    plot_buffer[plot_count].min_y   = (F64)(min_y.is_float
                                                                            ? min_y.floating
                                                                            : min_y.integer);
                                    
                                    plot_buffer[plot_count].max_y   = (F64)(max_y.is_float
                                                                            ? max_y.floating
                                                                            : max_y.integer);
                                    
                                    if (plot_buffer[plot_count].min_y >= plot_buffer[plot_count].max_y)
                                    {
                                        encountered_errors = true;
                                    }
                                    
                                    plot_buffer[plot_count].is_active = (plot_buffer[plot_count].is_active || is_active);
                                }
                                
                                else encountered_errors = true;
                            }
                        }
                        
                        else if (StringCompare(current->call.name, CONST_STRING("plot")))
                        {
                            if (current->call.arg_count == 0) encountered_errors = true;
                            else
                            {
                                Copy(current->call.args, plot_buffer[plot_count].funcs,
                                     sizeof(Calc_Node*) * ArrayCount(current->call.args));
                                
                                plot_buffer[plot_count].func_count = current->call.arg_count;
                                
                                for (U32 i = 0; i < plot_buffer[plot_count].func_count; ++i)
                                {
                                    Number y_val;
                                    if (!EvalCalcNode(plot_buffer[plot_count].funcs[i], &y_val))
                                    {
                                        encountered_errors = true;
                                        break;
                                    }
                                }
                                
                                plot_buffer[plot_count].is_active = (plot_buffer[plot_count].is_active || is_active);
                                
                                plot_buffer[plot_count].pos = line_end;
                                
                                if (plot_count == ArrayCount(plot_buffer)) encountered_errors = true;
                                else
                                {
                                    plot_count += 1;
                                }
                            }
                        }
                        
                        else
                        {
                            should_draw_number = true;
                            if (!EvalCalcNode(current, &number))
                            {
                                encountered_errors = true;
                            }
                        }
                    }
                    
                    else
                    {
                        should_draw_number = true;
                        if (!EvalCalcNode(current, &number))
                        {
                            encountered_errors = true;
                        }
                    }
                }
                
                if (!encountered_errors && should_draw_number)
                {
                    String_Const_u8 disp_string;
                    disp_string.str = (U8*)char_buffer;
                    
                    if (number.is_float)
                    {
                        disp_string.size = snprintf(char_buffer, ArrayCount(char_buffer), "= %g", number.floating);
                    }
                    
                    else
                    {
                        disp_string.size = snprintf(char_buffer, ArrayCount(char_buffer), "= %lld", number.integer);
                    }
                    
                    Rect_f32 origin_rect = text_layout_character_on_screen(app, text_layout_id, line_end);
                    
                    draw_string(app, face_id, disp_string,
                                V2f32(origin_rect.x0 + metrics.max_advance, origin_rect.y0),
                                (is_active ? alarm_color : fcolor_resolve(fcolor_id(defcolor_comment))));
                }
            }
        }
    }
    
    I64 pos = base_pos + (string.data - start);
    Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(pos));
    Vec2_f32 p           = view_relative_xy_of_pos(app, view, cursor.line, pos);
    I64 line_end         = view_pos_at_relative_xy(app, view, cursor.line, {max_f32, p.y});
    
    ARGB_Color func_color_cycle[] = {
        0xff03d3fc,
        0xff22b80b,
        0xfff0bb0c,
        0xfff0500c,
    };
    
    if (encountered_errors)
    {
        Rect_f32 origin_rect = text_layout_character_on_screen(app, text_layout_id, line_end);
        
        draw_string(app, face_id, string_u8_litexpr("X"),
                    V2f32(origin_rect.x0 + metrics.max_advance, origin_rect.y0),
                    alarm_color);
    }
    
    else
    {
        Rect_f32 screen_rect = view_get_screen_rect(app, view);
        
        F32 aspect = 4.0f / 3.0f;
        F32 height = screen_rect.y1 / 5;
        F32 width  = height * aspect;
        
        F32 title_height = 1.2f * metrics.line_height;
        
        F32 start_x = screen_rect.x1 - (width + title_height);
        
        F32 start_y = text_layout_character_on_screen(app, text_layout_id, base_pos).y0;
        if (start_y >= height + title_height) start_y -= height / 2 + title_height;
        else                                  start_y += title_height;
        
        for (U32 plot_index = 0; !encountered_errors && plot_index < plot_count; ++plot_index)
        {
            Calc_Plot* plot = &plot_buffer[plot_index];
            
            if (plot->title.size)
            {
                draw_string_oriented(app, face_id, fcolor_resolve(fcolor_id(defcolor_comment)),
                                     {plot->title.data, plot->title.size}, V2f32(start_x, start_y),
                                     0, V2f32(1, 0));
            }
            
            start_y += title_height;
            
            Rect_f32 window = {};
            window.x0 = start_x;
            window.x1 = start_x + width;
            window.y0 = start_y;
            window.y1 = start_y + height;
            
            F32 clip_padding = 0.1f;
            
            Rect_f32 clip = window;
            clip.x0 += clip_padding;
            clip.x1 -= clip_padding;
            clip.y0 += clip_padding;
            clip.y1 -= clip_padding;
            
            Rect_f32 prev_clip = draw_set_clip(app, clip);
            
            draw_rectangle(app, window, 0, fcolor_resolve(fcolor_id(defcolor_back)));
            
            F32 axis_thickness    = 2.0f;
            ARGB_Color axis_color = fcolor_resolve(fcolor_id(defcolor_margin_hover));
            
            if (plot->min_x == plot->max_x)
            {
                plot->min_x = -1;
                plot->max_x = +1;
            }
            
            if (plot->min_y == plot->max_y)
            {
                plot->min_y = -1;
                plot->max_y = +1;
            }
            
            F32 y_offset = (F32)(plot->max_y * (height / ABS(plot->max_y - plot->min_y)));
            
            Rect_f32 x_axis = {};
            x_axis.x0 = window.x0;
            x_axis.x1 = window.x1;
            x_axis.y0 = window.y0 + y_offset - axis_thickness / 2;
            x_axis.y1 = window.y0 + y_offset + axis_thickness / 2;
            
            draw_rectangle(app, x_axis, 0, axis_color);
            
            draw_string_oriented(app, face_id, fcolor_resolve(fcolor_id(defcolor_comment)),
                                 {plot->x_label.data, plot->x_label.size},
                                 V2f32(window.x1 - 2 * metrics.max_advance,
                                       x_axis.y1 + 0.5f * metrics.max_advance),
                                 0, V2f32(1, 0));
            
            F32 x_offset = (F32)(plot->max_x * (width / ABS(plot->max_x - plot->min_x)));
            
            Rect_f32 y_axis = {};
            y_axis.x0 = window.x1 - x_offset - axis_thickness / 2;
            y_axis.x1 = window.x1 - x_offset + axis_thickness / 2;
            y_axis.y0 = window.y0;
            y_axis.y1 = window.y1;
            
            draw_rectangle(app, y_axis, 0, axis_color);
            
            draw_string_oriented(app, face_id, fcolor_resolve(fcolor_id(defcolor_comment)),
                                 {plot->y_label.data, plot->y_label.size},
                                 V2f32(y_axis.x0 - 1.5f * metrics.max_advance,
                                       window.y0 + metrics.max_advance),
                                 0, V2f32(1, 0));
            
            for (U32 i = 0; !encountered_errors && i < plot->func_count; ++i)
            {
                Number* x_val = &CalcVariables[CALC_X_VAR_INDEX].value;
                x_val->is_float = true;
                
                F32 point_thickness   = 2;
                ARGB_Color func_color = func_color_cycle[i % ArrayCount(func_color_cycle)];
                
                Vec2_f32 origin = V2f32(window.x1 - x_offset, window.y0 + y_offset);
                
                F32 x_adj  = (F32)(width / ABS(plot->max_x - plot->min_x));
                F32 y_adj  = (F32)(height / ABS(plot->max_y - plot->min_y));
                F32 x_step = (F32)(ABS(plot->max_y - plot->min_y) / 100);
                
                for (U32 j = 0; !encountered_errors; ++j)
                {
                    x_val->floating = (F32)(plot->min_x + j * x_step);
                    
                    Number y_val = {};
                    if (EvalCalcNode(plot->funcs[i], &y_val))
                    {
                        F64 x = (F64)(x_val->is_float ? x_val->floating : x_val->integer);
                        F64 y = (F64)(y_val.is_float  ? y_val.floating  : y_val.integer);
                        
                        if (x > plot->max_x) break;
                        
                        Rect_f32 point = {};
                        point.x0 = origin.x + (F32)(x * x_adj - point_thickness / 2);
                        point.x1 = origin.x + (F32)(x * x_adj + point_thickness / 2);
                        point.y0 = origin.y - (F32)(y * y_adj + point_thickness / 2);
                        point.y1 = origin.y - (F32)(y * y_adj - point_thickness / 2);
                        
                        draw_rectangle(app, point, 2.0f, func_color);
                    }
                }
                
                if (encountered_errors)
                {
                    Rect_f32 origin_rect = text_layout_character_on_screen(app, text_layout_id, plot->pos);
                    
                    draw_string(app, face_id, string_u8_litexpr("X"),
                                V2f32(origin_rect.x0 + metrics.max_advance, origin_rect.y0),
                                alarm_color);
                    break;
                }
            }
            
            start_y += height + title_height;
            
            draw_set_clip(app, prev_clip);
            
            draw_rectangle_outline(app, window, 4.0f, 3.0f, (plot->is_active
                                                             ? alarm_color
                                                             : fcolor_resolve(fcolor_id(defcolor_margin))));
        }
    }
    
    Arena_FreeAll(&arena);
}