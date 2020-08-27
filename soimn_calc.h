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
    CalcNode_Assignment,
    CalcNode_Variable,
    CalcNode_Number,
    CalcNode_Subscript,
    
    CALC_NODE_KIND_COUNT
};

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
        
        struct
        {
            Calc_Node* left;
            Calc_Node* right;
            U8 op;
        } assignment;
        
        Number number;
    };
};

Calc_Node*
AddCalcNode(Memory_Arena* arena, U8 kind)
{
    Calc_Node* node = (Calc_Node*)Arena_Allocate(arena, sizeof(Calc_Node), alignof(Calc_Node));
    node->kind = kind;
    
    return node;
}

bool ParsePrimaryExpr(Memory_Arena* arena, String* string, Calc_Node** result);

bool
ParsePostExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    while (!encountered_errors)
    {
        EatAllWhitespace(string);
        
        if (string->size > 1) break;
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
            
            if (!IsAlpha(string->data[0]) || string->data[0] != '_') encountered_errors = true;
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
    
    if (!ParseUnaryExpr(arena, string, result)) encountered_errors = true;
    else
    {
        EatAllWhitespace(string);
        
        if (string->size >= 1                            &&
            !(string->size >= 2 && string->data[1] == '=') &&
            !(string->size >= 3 && string->data[0] == string->data[1] && string->data[2] == '='))
        {
            U8 kind = CalcNode_Invalid;
            
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
            
            if (kind != CalcNode_Invalid)
            {
                if (kind == CalcNode_BitLShift || kind == CalcNode_BitRShift) Advance(string, 2);
                else                                                          Advance(string, 1);
                
                Calc_Node* lhs = *result;
                
                *result = AddCalcNode(arena, kind);
                (*result)->left = lhs;
                
                if (!ParseMulLevelExpr(arena, string, &(*result)->right))
                {
                    encountered_errors = true;
                }
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseAddLevelExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    if (!ParseMulLevelExpr(arena, string, result)) encountered_errors = true;
    else
    {
        EatAllWhitespace(string);
        
        if (string->size >= 1 &&
            !(string->size >= 2 && string->data[1] == '='))
        {
            U8 kind = CalcNode_Invalid;
            
            if      (string->data[0] == '+')                           kind = CalcNode_Add;
            else if (string->data[0] == '-')                           kind = CalcNode_Sub;
            else if (string->data[0] == '|' && string->data[1] != '|') kind = CalcNode_BitOr;
            
            if (kind != CalcNode_Invalid)
            {
                Advance(string, 1);
                
                Calc_Node* lhs = *result;
                
                *result = AddCalcNode(arena, kind);
                (*result)->left = lhs;
                
                if (!ParseAddLevelExpr(arena, string, &(*result)->right))
                {
                    encountered_errors = true;
                }
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseComparisonExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    if (!ParseAddLevelExpr(arena, string, result)) encountered_errors = true;
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
            
            if (kind != CalcNode_Invalid)
            {
                if (kind == CalcNode_IsLess || kind == CalcNode_IsGreater) Advance(string, 1);
                else                                                       Advance(string, 2);
                
                Calc_Node* lhs = *result;
                
                *result = AddCalcNode(arena, kind);
                (*result)->left = lhs;
                
                if (!ParseAddLevelExpr(arena, string, &(*result)->right))
                {
                    encountered_errors = true;
                }
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseLogicalAndExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    if (!ParseComparisonExpr(arena, string, result)) encountered_errors = true;
    else
    {
        EatAllWhitespace(string);
        
        if (string->size > 1       &&
            string->data[0] == '&' &&
            string->data[1] == '&' &&
            (string->size < 3 || string->data[2] != '='))
        {
            Advance(string, 2);
            
            Calc_Node* lhs = *result;
            
            *result = AddCalcNode(arena, CalcNode_And);
            (*result)->left = lhs;
            
            if (!ParseLogicalAndExpr(arena, string, &(*result)->right))
            {
                encountered_errors = true;
            }
        }
    }
    
    return !encountered_errors;
}

bool
ParseLogicalOrExpr(Memory_Arena* arena, String* string, Calc_Node** result)
{
    bool encountered_errors = false;
    
    if (!ParseLogicalAndExpr(arena, string, result)) encountered_errors = true;
    else
    {
        EatAllWhitespace(string);
        
        if (string->size > 1       &&
            string->data[0] == '|' &&
            string->data[1] == '|' &&
            (string->size < 3 || string->data[2] != '='))
        {
            Advance(string, 2);
            
            Calc_Node* lhs = *result;
            
            *result = AddCalcNode(arena, CalcNode_Or);
            (*result)->left = lhs;
            
            if (!ParseLogicalOrExpr(arena, string, &(*result)->right))
            {
                encountered_errors = true;
            }
        }
    }
    
    return !encountered_errors;
}

void
RenderCalcComment(I64 base_pos, String string)
{
    Memory_Arena arena = {};
    
    for (;;)
    {
        bool encountered_errors = false;
        
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
                    
                    Calc_Node* lhs = current;
                    
                    current = AddCalcNode(&arena, CalcNode_Assignment);
                    current->assignment.op   = kind;
                    current->assignment.left = lhs;
                    
                    if (!ParseLogicalOrExpr(&arena, &string, &current->assignment.right))
                    {
                        encountered_errors = true;
                    }
                }
            }
            
            EatAllWhitespace(&string);
            
            if (string.size != 0 && string.data[0] != '\r' && string.data[0] != '\n')
            {
                encountered_errors = true;
            }
        }
        
        if (encountered_errors)
        {
            break;
        }
        
        else
        {
            NOT_IMPLEMENTED;
            
            Arena_ClearAll(&arena);
        }
    }
    
    Arena_FreeAll(&arena);
}