
struct compile_file
{
    u8 *Path;
};

struct node_info
{
    compile_file *File;
    u32 Line;
    void *Data;
};

struct node_key
{
    u32 ID;
};

enum ast_node_type
{
    EXPRESSION, 
    STATEMENT,
    
    ADDITION, 
    SUBTRACTION, 
    MULTIPLICATION, 
    DIVISION, 
    NUMBER, 
    IDENTIFIER,
    
    ASSIGN, 
    SEQUENCE,
};

struct ast_node
{
    node_key Key;
    ast_node_type Type;
    ast_node *Child[2];
    void *Data;
};

struct ast_base
{
    ast_node  *BaseNode;
    node_info *InfoTable;
    u32 InfoTableCount;
};

internal ast_node *ParseFile(bucket_allocator *Bucket, read_file_result *Input, string_compound *Result);
internal ast_node *ParseAssignment(bucket_allocator *Bucket, u8* Content, u32 *LookAhead, string_compound *Result);
internal ast_node *ParseExpression(bucket_allocator *Bucket, u8* Content, u32 *LookAhead, string_compound *Result);
internal ast_node *ParseTerm(bucket_allocator *Bucket, u8* Content, u32 *LookAhead, string_compound *Result);
internal ast_node *ParseFactor(bucket_allocator *Bucket, u8* Content, u32 *LookAhead, string_compound *Result);
inline   void ParseMatch(u8* Content, u32 *LookAhead, u8 Check, string_compound *Result);
internal ast_node *ParseNumber(bucket_allocator *Bucket, u8 *Content, u32 *LookAhead, string_compound *Result);
internal ast_node *ParseIdentifier(bucket_allocator *Bucket, u8 *Content, u32 *LookAhead, string_compound *Result);
inline   b32  IsCharDigit(u8 Char);
inline   b32  IsCharIdentifier(u8 Char);
inline   void ConsumeSpaces(u8 *Content, u32 *LookAhead, string_compound *Result);
inline   void ConsumeSpacesAndLinebreaks(u8 *Content, u32 *LookAhead, string_compound *Result);

inline void 
ParseMatch(u8* Content, u32 *LookAhead, u8 Check, string_compound *Result)
{
    if(Content[*LookAhead] == Check)
    {
        (*LookAhead)++;
    }
    else
    {
        CopyStringToCompound(Result, (u8 *)"\nSyntax Error:: Expected \"");
        CopyCharToCompound(Result, Content[*LookAhead]);
        CopyStringToCompound(Result, (u8 *)"\", but found: \"");
        CopyCharToCompound(Result, Check);
        CopyStringToCompound(Result, (u8 *)"\".\n");
    }
}

inline void
ConsumeSpaces(u8 *Content, u32 *LookAhead, string_compound *Result)
{
    while(true)
    {
        if(Content[*LookAhead] == ' ') ParseMatch(Content, LookAhead, ' ', Result);
        else break;
    }
}

inline void
ConsumeSpacesAndLinebreaks(u8 *Content, u32 *LookAhead, string_compound *Result)
{
    while(true)
    {
        if(Content[*LookAhead] == ' ') ParseMatch(Content, LookAhead, ' ', Result);
        else if(Content[*LookAhead] == '\n') ParseMatch(Content, LookAhead, '\n', Result);
        else break;
    }
}

inline b32
IsCharDigit(u8 Char)
{
    // NOTE:: Digits from 0 - 9
    b32 Result = (Char >= 48 && Char < 59);
    
    return Result;
}

inline b32 
IsCharIdentifier(u8 Char)
{
    // NOTE:: Capital letters || underscore || small letters
    b32 Result = (Char >= 65 && Char < 91) || Char == 95 || (Char >= 97 && Char < 123);
    
    return Result;
}

inline ast_node *
ParseNumber(bucket_allocator *Bucket, u8 *Content, u32 *LookAhead, string_compound *Result)
{
    ast_node *Node = PushStructOnTransientBucket(Bucket, ast_node);
    Node->Type = NUMBER;
    
    u32 StartPos = *LookAhead;
    printf("%i", Content[*LookAhead]-48);
    CopyCharToCompound(Result, Content[*LookAhead]);
    ParseMatch(Content, LookAhead, Content[*LookAhead], Result);
    
    while(true)
    {
        if(IsCharDigit(Content[*LookAhead]))
        {
            printf("%i", Content[*LookAhead]-48);
            CopyCharToCompound(Result, Content[*LookAhead]);
            ParseMatch(Content, LookAhead, Content[*LookAhead], Result);
        }
        else break;
    }
    Node->Data = PushArrayOnFixedBucket(Bucket, *LookAhead-StartPos+1, u8);
    u8* Digits = (u8 *)Node->Data;
    for(u32 It = StartPos; It < *LookAhead; ++It)
    {
        *Digits++ = Content[It];
    }
    *Digits = '\0';
    
    CopyCharToCompound(Result, ' ');
    printf(" ");
    ConsumeSpacesAndLinebreaks(Content, LookAhead, Result);
    return Node;
}

inline ast_node *
ParseIdentifier(bucket_allocator *Bucket, u8 *Content, u32 *LookAhead, string_compound *Result)
{
    ast_node *Node = PushStructOnTransientBucket(Bucket, ast_node);
    Node->Type = IDENTIFIER;
    
    u32 StartPos = *LookAhead;
    printf("%c", Content[*LookAhead]);
    CopyCharToCompound(Result, Content[*LookAhead]);
    ParseMatch(Content, LookAhead, Content[*LookAhead], Result);
    
    while(true)
    {
        if(IsCharIdentifier(Content[*LookAhead]))
        {
            printf("%c", Content[*LookAhead]);
            CopyCharToCompound(Result, Content[*LookAhead]);
            ParseMatch(Content, LookAhead, Content[*LookAhead], Result);
        }
        else if(IsCharDigit(Content[*LookAhead]))
        {
            printf("%i ", Content[*LookAhead]-48);
            CopyCharToCompound(Result, Content[*LookAhead]);
            ParseMatch(Content, LookAhead, Content[*LookAhead], Result);
        }
        else break;
    }
    Node->Data = PushArrayOnFixedBucket(Bucket, *LookAhead-StartPos+1, u8);
    u8* Digits = (u8 *)Node->Data;
    for(u32 It = StartPos; It < *LookAhead; ++It)
    {
        *Digits++ = Content[It];
    }
    *Digits = '\0';
    
    CopyCharToCompound(Result, ' ');
    printf(" ");
    ConsumeSpacesAndLinebreaks(Content, LookAhead, Result);
    return Node;
}

internal ast_node *
ParseFile(bucket_allocator *Bucket, read_file_result *Input, string_compound * Result)
{
    u8 *Content = (u8 *)Input->Contents;
    u32 LookAhead = 0;
    
    ast_node *Node = 0;
    
    Node = ParseAssignment(Bucket, Content, &LookAhead, Result);
    
    while(true)
    {
        if(Content[LookAhead] == ';')
        {
            ast_node *NewNode = PushStructOnTransientBucket(Bucket, ast_node);
            NewNode->Type = SEQUENCE;
            NewNode->Child[0] = Node;
            Node = NewNode;
            
            ParseMatch(Content, &LookAhead, ';', Result);
            ConsumeSpacesAndLinebreaks(Content, &LookAhead, Result);
            CopyCharToCompound(Result, '\n');
            Node->Child[1] = ParseAssignment(Bucket, Content, &LookAhead, Result);
        }
        else break;
    }
    
    if(Content[LookAhead] != '\n' && Content[LookAhead] != '\0')
    {
        u8 *ErrorMsg = (u8 *)"\nSyntax Error:: Expected end of line/file!\n";
        CopyStringToCompound(Result, ErrorMsg);
    }
    return Node;
}

internal ast_node *
ParseAssignment(bucket_allocator *Bucket, u8* Content, u32 *LookAhead, string_compound *Result)
{
    ast_node *Node = 0;
    Node = ParseExpression(Bucket, Content, LookAhead, Result);
    
    if(Content[*LookAhead] == '=')
    {
        if(Node->Type == IDENTIFIER)
        {
            ast_node *NewNode = PushStructOnTransientBucket(Bucket, ast_node);
            NewNode->Type = ASSIGN;
            NewNode->Child[0] = Node;
            Node = NewNode;
            
            ParseMatch(Content, LookAhead, '=', Result);
            CopyStringToCompound(Result, (u8 *)"= ");
            printf("= ");
            ConsumeSpacesAndLinebreaks(Content, LookAhead, Result);
            
            Node->Child[1] = ParseExpression(Bucket, Content, LookAhead, Result);
        }
        else
        {
            u8 *ErrorMsg = (u8 *)"\nSyntax Error:: Expected single identifier before assign!\n";
            CopyStringToCompound(Result, ErrorMsg);
        }
    }
    return Node;
}

internal ast_node * 
ParseExpression(bucket_allocator *Bucket, u8* Content, u32 *LookAhead, string_compound *Result)
{
    ast_node *Node = 0;
    Node = ParseTerm(Bucket, Content, LookAhead, Result);
    
    while(true)
    {
        if(Content[*LookAhead] == '+')
        {
            ast_node *NewNode = PushStructOnTransientBucket(Bucket, ast_node);
            NewNode->Type = ADDITION;
            NewNode->Child[0] = Node;
            Node = NewNode;
            
            ParseMatch(Content, LookAhead, '+', Result);
            ConsumeSpacesAndLinebreaks(Content, LookAhead, Result);
            Node->Child[1] = ParseTerm(Bucket, Content, LookAhead, Result);
            CopyStringToCompound(Result, (u8 *)"+ ");
            printf("+ ");
        }
        else if(Content[*LookAhead] == '-')
        {
            ast_node *NewNode = PushStructOnTransientBucket(Bucket, ast_node);
            NewNode->Type = SUBTRACTION;
            NewNode->Child[0] = Node;
            Node = NewNode;
            
            ParseMatch(Content, LookAhead, '-', Result);
            ConsumeSpacesAndLinebreaks(Content, LookAhead, Result);
            Node->Child[1] = ParseTerm(Bucket, Content, LookAhead, Result);
            CopyStringToCompound(Result, (u8 *)"- ");
            printf("- ");
        }
        else break;
    }
    return Node;
}

internal ast_node * 
ParseTerm(bucket_allocator *Bucket, u8* Content, u32 *LookAhead, string_compound *Result)
{
    ast_node *Node = 0;
    Node = ParseFactor(Bucket, Content, LookAhead, Result);
    
    while(true)
    {
        if(Content[*LookAhead] == '*')
        {
            ast_node *NewNode = PushStructOnTransientBucket(Bucket, ast_node);
            NewNode->Type = MULTIPLICATION;
            NewNode->Child[0] = Node;
            Node = NewNode;
            
            ParseMatch(Content, LookAhead, '*', Result);
            ConsumeSpacesAndLinebreaks(Content, LookAhead, Result);
            Node->Child[1] = ParseFactor(Bucket, Content, LookAhead, Result);
            CopyStringToCompound(Result, (u8 *)"* ");
            printf("* ");
        }
        else if(Content[*LookAhead] == '/')
        {
            ast_node *NewNode = PushStructOnTransientBucket(Bucket, ast_node);
            NewNode->Type = DIVISION;
            NewNode->Child[0] = Node;
            Node = NewNode;
            
            ParseMatch(Content, LookAhead, '/', Result);
            ConsumeSpacesAndLinebreaks(Content, LookAhead, Result);
            Node->Child[1] = ParseFactor(Bucket, Content, LookAhead, Result);
            CopyStringToCompound(Result, (u8 *)"/ ");
            printf("/ ");
        }
        else break;
    }
    return Node;
}

internal ast_node *
ParseFactor(bucket_allocator *Bucket, u8* Content, u32 *LookAhead, string_compound *Result)
{
    ast_node *Node = 0;
    
    if(Content[*LookAhead] == '(')
    {
        ParseMatch(Content, LookAhead, '(', Result);
        ConsumeSpacesAndLinebreaks(Content, LookAhead, Result);
        Node = ParseExpression(Bucket, Content, LookAhead, Result);
        ConsumeSpacesAndLinebreaks(Content, LookAhead, Result);
        ParseMatch(Content, LookAhead, ')', Result);
        ConsumeSpacesAndLinebreaks(Content, LookAhead, Result);
    }
    else if(IsCharDigit(Content[*LookAhead]))
    {
        Node = ParseNumber(Bucket, Content, LookAhead, Result);
    }
    else if(IsCharIdentifier(Content[*LookAhead]))
    {
        Node = ParseIdentifier(Bucket, Content, LookAhead, Result);
    }
    else
    {
        u8 *ErrorMsg = (u8 *)"\nSyntax Error:: Expected <NUM> or <(>, \nbut found: ";
        CopyStringToCompound(Result, ErrorMsg);
        CopyCharToCompound(Result, Content[*LookAhead]);
        CopyCharToCompound(Result, '\n');
    }
    return Node;
}

inline u8 *
StringifyASTNodeType(ast_node_type Type)
{
    u8 *Strings[] = 
    {
        (u8 *)"EXPRESSION", 
        (u8 *)"STATEMENT",
        
        (u8 *)"ADDITION", 
        (u8 *)"SUBTRACTION", 
        (u8 *)"MULTIPLICATION", 
        (u8 *)"DIVISION", 
        (u8 *)"NUMBER", 
        (u8 *)"IDENTIFIER",
        
        (u8 *)"ASSIGN", 
        (u8 *)"SEQUENCE",
    };
    return Strings[Type];
}

inline u8 *
StringifyAST(bucket_allocator *Bucket, ast_node *AST)
{
    string_compound S = {};
    NewTMPStringCompound(Bucket, &S, 400);
    
    CopyStringToCompound(&S, StringifyASTNodeType(AST->Type));
    
    if(AST->Type != NUMBER && AST->Type != IDENTIFIER) 
    {
        CopyStringToCompound(&S, (u8 *)"-> ");
        CopyStringToCompound(&S, StringifyAST(Bucket, AST->Child[0]));
        CopyStringToCompound(&S, StringifyAST(Bucket, AST->Child[1]));
    }
    else 
    {
        CopyCharToCompound(&S, '(');
        CopyStringToCompound(&S, (u8 *)AST->Data);
        CopyStringToCompound(&S, (u8 *)") ");
    }
    
    u8 *Result = PushArrayOnTransientBucket(Bucket, S.Pos, u8);
    CopyString(Result, S.S, S.Pos);
    
    DeleteTMPStringCompound(Bucket, &S);
    return Result;
}

inline u8
GiveNodeSymbol(ast_node_type Type)
{
    u8 Result = 0;
    switch(Type)
    {
        case ASSIGN:
        {
            Result = '=';
        } break;
        case SEQUENCE:
        {
            Result = ';';
        } break;
        case ADDITION:
        {
            Result = '+';
        } break;
        case SUBTRACTION:
        {
            Result = '-';
        } break;
        case MULTIPLICATION:
        {
            Result = '*';
        } break;
        case DIVISION:
        {
            Result = '/';
        } break;
    }
    return Result;
}

inline u8 *
StringifyASTPostfix(bucket_allocator *Bucket, ast_node *AST, ast_node *Parent)
{
    
    string_compound S = {};
    NewTMPStringCompound(Bucket, &S, 400);
    
    if(AST->Type != NUMBER && AST->Type != IDENTIFIER)
    {
        CopyStringToCompound(&S, StringifyASTPostfix(Bucket, AST->Child[0], AST));
        if(Parent && AST != Parent->Child[1])
        {
            if(Parent->Type == ASSIGN || Parent->Type == SEQUENCE)
            {
                CopyCharToCompound(&S, GiveNodeSymbol(Parent->Type));
                CopyCharToCompound(&S, ' ');
                CopyStringToCompound(&S, StringifyASTPostfix(Bucket, Parent->Child[1], Parent));
            }
            else
            {
                CopyStringToCompound(&S, StringifyASTPostfix(Bucket, Parent->Child[1], Parent));
                CopyCharToCompound(&S, GiveNodeSymbol(Parent->Type));
            }
        }
    }
    else if(AST != Parent->Child[1])
    {
        if(Parent->Type == ASSIGN || Parent->Type == SEQUENCE)
        {
            CopyStringToCompound(&S, (u8 *)AST->Data);
            CopyCharToCompound(&S, GiveNodeSymbol(Parent->Type));
            CopyCharToCompound(&S, ' ');
            CopyStringToCompound(&S, StringifyASTPostfix(Bucket, Parent->Child[1], Parent));
        }
        else
        {
            CopyStringToCompound(&S, (u8 *)AST->Data);
            CopyStringToCompound(&S, StringifyASTPostfix(Bucket, Parent->Child[1], Parent));
            CopyCharToCompound(&S, GiveNodeSymbol(Parent->Type));
        }
    }
    else 
    {
        CopyStringToCompound(&S, (u8 *)AST->Data);
    }
    
    u8 *Result = PushArrayOnTransientBucket(Bucket, S.Pos, u8);
    CopyString(Result, S.S, S.Pos);
    
    DeleteTMPStringCompound(Bucket, &S);
    return Result;
}