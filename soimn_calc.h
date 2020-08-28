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
            Calc_Node** args;
            U32 arg_count;
        } call;
        
        struct
        {
            String name;
            bool is_global;
        } var;
        
        Number number;
    };
};

struct Calc_Var
{
    String name;
    Number value;
};

Calc_Var CalcVariables[256] = {};
U32 CalcVariableCount       = 0;
F64 CalcTime                = 0;

#define CALC_FUNC_LIST()                                                                                \
CALC_FUNC(Sqrt, 1,                                                                                  \
{                                                                                         \
Number result;                                                                        \
result.is_float = true;                                                               \
result.floating = sqrt((F64)(args[0].is_float ? args[0].floating : args[0].integer)); \
return result;                                                                        \
})                                                                                        \
CALC_FUNC(Time, 0,                                                                                  \
{                                                                                         \
Number result;                                                                        \
result.is_float = true;                                                               \
result.floating = CalcTime;                                                           \
return result;                                                                        \
})                                                                                        \
CALC_FUNC(Sin, 1,                                                                                   \
{                                                                                         \
Number result;                                                                        \
result.is_float = true;                                                               \
result.floating = sin((F64)(args[0].is_float ? args[0].floating : args[0].integer));  \
return result;                                                                        \
})                                                                                        \
CALC_FUNC(Cos, 1,                                                                                   \
{                                                                                         \
Number result;                                                                        \
result.is_float = true;                                                               \
result.floating = cos((F64)(args[0].is_float ? args[0].floating : args[0].integer));  \
return result;                                                                        \
})                                                                                        \
CALC_FUNC(Tan, 1,                                                                                   \
{                                                                                         \
Number result;                                                                        \
result.is_float = true;                                                               \
result.floating = tan((F64)(args[0].is_float ? args[0].floating : args[0].integer));  \
return result;                                                                        \
})

#define CALC_FUNC(name, arg_count, body) Number name (Number args[CALC_FUNC_MAX_ARG_COUNT]) body
CALC_FUNC_LIST()
#undef CALC_FUNC

void
Plot(Calc_Node* func)
{
    NOT_IMPLEMENTED;
}

Calc_Node*
AddCalcNode(Memory_Arena* arena, U8 kind)
{
    Calc_Node* node = (Calc_Node*)Arena_Allocate(arena, sizeof(Calc_Node), alignof(Calc_Node));
    ZeroStruct(node);
    
    node->kind = kind;
    
    if (node->kind == CalcNode_Call)
    {
        node->call.args = (Calc_Node**)Arena_Allocate(arena, sizeof(Calc_Node*) * CALC_FUNC_MAX_ARG_COUNT,
                                                      alignof(Calc_Node*));
        Zero(node->call.args, sizeof(Calc_Node*) * CALC_FUNC_MAX_ARG_COUNT);
    }
    
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
                    *result = AddCalcNode(arena, CalcNode_Variable);
                    (*result)->var.name      = identifier;
                    (*result)->var.is_global = is_global;
                }
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
                
                Advance(string, 2);
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
            if (ParsePrimaryExpr(arena, string, result)) break;
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
    
    else if (node->kind == CalcNode_Add ||
             node->kind == CalcNode_Sub ||
             node->kind == CalcNode_Mul ||
             node->kind == CalcNode_Div ||
             node->kind == CalcNode_Pow)
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
                number->integer  = lhs.integer  / rhs.integer;
                number->floating = lhs.floating / rhs.floating;
            }
            
            else
            {
                number->is_float = true;
                
                number->floating = pow(lhs.floating, rhs.floating);
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
        
#define ENSURE_ARG_COUNT(n) if (arg_count != (n)) { encountered_errors = true; break; }
        
        if (!encountered_errors) do
        {
            if (StringCompare(node->call.name, CONST_STRING("int"), false))
            {
                ENSURE_ARG_COUNT(1);
                
                number->is_float = false;
                
                if (args[0].is_float)
                {
                    number->integer  =      (I64)args[0].floating;
                    number->floating = (F64)(I64)args[0].floating;
                }
                
                else
                {
                    number->integer  = (I64)args[0].integer;
                    number->floating = (F64)args[0].integer;
                }
            }
            
            else if (StringCompare(node->call.name, CONST_STRING("float"), false))
            {
                ENSURE_ARG_COUNT(1);
                
                number->is_float = true;
                
                if (args[0].is_float)
                {
                    number->integer  = (I64)args[0].floating;
                    number->floating = (F64)args[0].floating;
                }
                
                else
                {
                    number->integer  = (I64)args[0].integer;
                    number->floating = (F64)args[0].integer;
                }
            }
            
#define CALC_FUNC(func_name, req_arg_count, body)                                   \
else if (StringCompare(node->call.name, CONST_STRING(#func_name), false)) \
{                                                                               \
ENSURE_ARG_COUNT(req_arg_count);                                            \
*number = func_name##(args);                                                \
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
    
    for (;;)
    {
        bool encountered_errors = false;
        
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
                
                else
                {
                    if (current->kind == CalcNode_Call && current->left->kind == CalcNode_Variable &&
                        current->left->var.is_global == false &&
                        StringCompare(current->left->var.name, CONST_STRING("plot")))
                    {
                        NOT_IMPLEMENTED;
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
                    if (cursor_pos >= line_start && cursor_pos <= line_end)
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
                                    pack_color(V4f32(1.0f, 0.2f, 0.2f, 1.0f)));
                    }
                }
            }
        }
        
        if (encountered_errors)
        {
            Rect_f32 origin_rect = text_layout_character_on_screen(app, text_layout_id, line_end);
            
            draw_string(app, face_id, string_u8_litexpr("X"),
                        V2f32(origin_rect.x0 + metrics.max_advance, origin_rect.y0),
                        pack_color(V4f32(1.0f, 0.2f, 0.2f, 1.0f)));
            break;
        }
        
        Arena_ClearAll(&arena);
    }
    
    Arena_FreeAll(&arena);
    
    Zero(CalcVariables, sizeof(CalcVariables));
    CalcVariableCount = 0;
}