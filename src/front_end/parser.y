%{
    #include <iostream>
    #include <cstdio>
    #include <cstring>
    #include <ast.h>
    #include <lexer.h>
    #include <type_define.h>
    void yyerror(char * msg);
    void yy_warning(char *msg);
    extern int line_no;
%}

%union
{
    class ASTNode* ast_node;
    int int_const_val;
    float float_const_val;
    std::string* string_const_val;
    int value_type;
    int unary_op;
};

%token <int_const_val>INTCONST
%token <float_const_val>FLOATCONST
%token <string_const_val>IDENT
%token IF ELSE WHILE BREAK CONTINUE RETURN
%token INT FLOAT VOID
%token CONST
%token L_PAREN R_PAREN L_BRACK R_BRACK L_BRACE R_BRACE
%token SEMI COMMA
%token ASSIGN
%token ADD SUB MUL DIV MOD AND OR NOT GT LT LE GE EQ NEQ

%start CompUnit
%left ASSIGN
%left ADD SUB
%left MUL DIV MOD
%left OR
%left AND
%left NOT
%type <ast_node> CompUnit Decl ConstDecl ConstDef ConstInitValArray VarDecl VarDef InitValArray FuncDef FuncFParams FuncFParam Block BlockItem
%type <ast_node> Stmt Exp Cond LVal PrimaryExp Number UnaryExp FuncRParams MulExp AddExp RelExp EqExp LAndExp LOrExp ConstExp
%type <unary_op> UnaryOp
%type <value_type> BType
%type <ast_node> ConstArrayIdent        //数组标识符 a[1]
%type <ast_node> ConstInitValArrayList  
%type <ast_node> InitValArrayList
%type <ast_node> FuncFParamArray        //作为函数形参的数组
%type <ast_node> BlockItemList          //多个语句块
%type <ast_node> ArrayIdent             //组标识符 a[i]

%%

CompUnit
    : Decl                                              {$$ = ASTNode::create_comp_unit();
                                                            $$->add_child($1);
                                                            g_ast_root = $$;}
    | FuncDef                                           {$$ = ASTNode::create_comp_unit();
                                                            $$->add_child($1);
                                                            g_ast_root = $$;}
    | CompUnit Decl                                     {$$ = $1;$$->add_child($2);}
    | CompUnit FuncDef                                  {$$ = $1;$$->add_child($2);}
    ;

Decl   
    : ConstDecl SEMI                                    {$$ = $1;}
    | VarDecl SEMI                                      {$$ = $1;}
    ; 

ConstDecl
    : CONST BType ConstDef                              {$$ = ASTNode::create_decl_stmt(ValueType((BasicType)$2));
                                                         $$->add_child(ASTNode::create_const_decl(ValueType((BasicType)$2),$3->get_var_name(),$3->get_value_type().is_array(), $3->get_child()));
                                                         delete $3;
                                                        }
    | ConstDecl COMMA ConstDef                          {$$ = $1;
                                                         $$->add_child(ASTNode::create_const_decl($1->get_value_type(),$3->get_var_name(),$3->get_value_type().is_array(), $3->get_child()));
                                                         delete $3;
                                                        }
    ;

BType
    : INT                                               {$$ = (int)BasicType::Int;}
    | FLOAT                                             {$$ = (int)BasicType::Float;}
    ;

ConstDef
    : IDENT ASSIGN ConstExp                             {$$ = ASTNode::create_const_decl(ValueType(BasicType::Uncertain),*$1, false, {$3});}
    | ConstArrayIdent ASSIGN ConstInitValArray          {$$ = ASTNode::create_const_decl(ValueType(BasicType::Uncertain),$1->get_var_name(), true, {$1,$3});}
    ;

ConstArrayIdent
    : IDENT L_BRACK ConstExp R_BRACK                    {$$ = ASTNode::create_array_size();
                                                         $$->set_var_name(*$1);
                                                         $$->add_child($3);}
    | ConstArrayIdent L_BRACK ConstExp R_BRACK          {$$ = $1;
                                                         $$->add_child($3);}
    ;

ConstInitValArray
    : L_BRACE R_BRACE                                   {$$ = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain));}
    | L_BRACE ConstInitValArrayList R_BRACE             {$$ = $2;}
    ;

ConstInitValArrayList
    : ConstInitValArray                                 {$$ = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain)); 
                                                         $$->add_child($1);}
    | ConstInitValArrayList COMMA ConstInitValArray     {$$ = $1; 
                                                         $$->add_child($3);}
    | ConstExp                                          {$$ = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain)); 
                                                         $$->add_child($1);}
    | ConstInitValArrayList COMMA ConstExp              {$$ = $1; 
                                                         $$->add_child($3);}


VarDecl
    : BType VarDef                                      {$$ = ASTNode::create_decl_stmt(ValueType((BasicType)$1));
                                                         $$->add_child(ASTNode::create_var_decl(ValueType((BasicType)$1),$2->get_var_name(), $2->get_value_type().is_array(), $2->get_child()));
                                                         delete $2;
                                                        }
    | VarDecl COMMA VarDef                              {$$ = $1;
                                                         $$->add_child(ASTNode::create_var_decl($1->get_value_type(),$3->get_var_name(), $3->get_value_type().is_array(), $3->get_child()));
                                                         delete $3;
                                                        }
    ;

VarDef
    : IDENT                                             {$$ = ASTNode::create_var_decl(ValueType(BasicType::Uncertain), *$1, false, {});}
    | ConstArrayIdent                                   {$$ = ASTNode::create_var_decl(ValueType(BasicType::Uncertain),$1->get_var_name(), true, {$1});}
    | IDENT ASSIGN Exp                                  {$$ = ASTNode::create_var_decl(ValueType(BasicType::Uncertain), *$1, false, {$3});}
    | ConstArrayIdent ASSIGN InitValArray               {$$ = ASTNode::create_var_decl(ValueType(BasicType::Uncertain),$1->get_var_name(), true, {$1,$3});}
    ;

InitValArray
    : L_BRACE R_BRACE                                   {$$ = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain));}
    | L_BRACE InitValArrayList R_BRACE                  {$$ = $2;}
    ;

InitValArrayList
    : InitValArray                                      {$$ = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain)); 
                                                         $$->add_child($1);}
    | InitValArrayList COMMA InitValArray               {$$ = $1; 
                                                         $$->add_child($3);}
    | Exp                                               {$$ = ASTNode::create_array_init_val(ValueType(BasicType::Uncertain)); 
                                                         $$->add_child($1);}
    | InitValArrayList COMMA Exp                        {$$ = $1; 
                                                         $$->add_child($3);}
    ;

FuncDef
    : BType IDENT L_PAREN R_PAREN Block                 {$$ = ASTNode::create_func_def(ValueType((BasicType)$1), *$2);
                                                         $$->add_child($5);}
    | VOID IDENT L_PAREN R_PAREN Block                  {$$ = ASTNode::create_func_def(ValueType(BasicType::Void), *$2);
                                                         $$->add_child($5);}
    | BType IDENT L_PAREN FuncFParams R_PAREN Block     {$$ = ASTNode::create_func_def(ValueType((BasicType)$1), *$2);
                                                         $$->add_child({$4,$6});}
    | VOID IDENT L_PAREN FuncFParams R_PAREN Block      {$$ = ASTNode::create_func_def(ValueType(BasicType::Void), *$2);
                                                         $$->add_child({$4,$6});}
    ;

FuncFParams
    : FuncFParam                                        {$$ = ASTNode::create_func_f_params();$$->add_child($1);}
    | FuncFParams COMMA FuncFParam                      {$$ = $1;$$->add_child($3);}
    ;

FuncFParam
    : BType IDENT                                       {$$ = ASTNode::create_func_f_param(ValueType((BasicType)$1), *$2, false, false, {});}
    | FuncFParamArray                                   {$$ = ASTNode::create_func_f_param(ValueType($1->get_value_type().basic()), $1->get_var_name(), true, false, $1->get_child());
                                                         delete $1;}
    ;

FuncFParamArray
    : BType IDENT L_BRACK R_BRACK                       {$$ = ASTNode::create_func_f_param(ValueType((BasicType)$1), *$2, true, false, {});}
    | BType IDENT L_BRACK Exp R_BRACK                   {yy_warning((char*)"FParam Array has value at first dim");$$ = ASTNode::create_func_f_param(ValueType((BasicType)$1), *$2, true, false, {});}
    | FuncFParamArray L_BRACK Exp R_BRACK               {$$ = $1;
                                                         $$->add_child($3);}
    ;

Block
    : L_BRACE R_BRACE                                   {$$ = ASTNode::create_block();}
    | L_BRACE BlockItemList R_BRACE                     {$$ = $2;}
    ;

BlockItemList
    : BlockItem                                         {$$ = ASTNode::create_block();$$->add_child($1);}
    | BlockItemList BlockItem                           {$$ = $1;$$->add_child($2);}
    ;

BlockItem
    : Decl                                              {$$ = $1;}
    | Stmt                                              {$$ = $1;}
    ;

Stmt
    : LVal ASSIGN Exp SEMI                              {$$ = ASTNode::create_binary_op(BinaryOpFunc::Assign,$1->get_value_type(),$1,$3);}
    | SEMI                                              {$$ = ASTNode::create_null_stmt();}       
    | Exp SEMI                                          {$$ = $1;}
    | Block                                             {$$ = $1;}
    | IF L_PAREN Cond R_PAREN Stmt                      {$$ = ASTNode::create_if_stmt($3,$5);}
    | IF L_PAREN Cond R_PAREN Stmt ELSE Stmt            {$$ = ASTNode::create_if_stmt($3,$5,$7);}
    | WHILE L_PAREN Cond R_PAREN Stmt                   {$$ = ASTNode::create_while_stmt($3,$5);}
    | BREAK SEMI                                        {$$ = ASTNode::create_break_stmt();}
    | CONTINUE SEMI                                     {$$ = ASTNode::create_continue_stmt();}
    | RETURN SEMI                                       {$$ = ASTNode::create_return_stmt();}
    | RETURN Exp SEMI                                   {$$ = ASTNode::create_return_stmt($2);}
    ;

Exp
    : AddExp                                            {$$ = $1;}
    ;

Cond 
    : LOrExp                                            {$$ = $1;}
    ;

LVal
    : IDENT                                             {$$ = ASTNode::create_ident(ValueType(BasicType::Uncertain), *$1, true);}
    | ArrayIdent                                        {$$ = $1;}
    ;

ArrayIdent
    : IDENT L_BRACK Exp R_BRACK                         {$$ = ASTNode::create_array_visit(ValueType(BasicType::Uncertain),ASTNode::create_ident(ValueType(BasicType::Uncertain), *$1, true),$3);}
    | ArrayIdent L_BRACK Exp R_BRACK                    {$$ = ASTNode::create_array_visit(ValueType(BasicType::Uncertain),$1,$3);}
    ;

PrimaryExp
    : L_PAREN Exp R_PAREN                               {$$ = $2;}
    | LVal                                              {$$ = $1;}
    | Number                                            {$$ = $1;}
    ;

Number
    : INTCONST                                          {$$ = ASTNode::create_const_value(ValueType(BasicType::Int),BasicValue::create_int($1)); }
    | FLOATCONST                                        {$$ = ASTNode::create_const_value(ValueType(BasicType::Float),BasicValue::create_float($1)); }
    ;

UnaryExp
    : PrimaryExp                                        {$$ = $1;}
    | IDENT L_PAREN R_PAREN                             {$$ = ASTNode::create_func_call(*$1);}
    | IDENT L_PAREN FuncRParams R_PAREN                 {$$ = ASTNode::create_func_call(*$1);$$->add_child($3);}
    | UnaryOp UnaryExp                                  {$$ = ASTNode::create_unary_op((UnaryOpFunc)$1,$2->get_value_type(),$2, false);}
    ;

UnaryOp
    : ADD                                               {$$ = (int)UnaryOpFunc::Positive;}
    | SUB                                               {$$ = (int)UnaryOpFunc::Negative;}
    | NOT                                               {$$ = (int)UnaryOpFunc::Not;}
    ;

FuncRParams
    : Exp                                               {$$ = ASTNode::create_func_r_params();$$->add_child($1);}
    | FuncRParams COMMA Exp                             {$$ = $1;$$->add_child($3);}
    ;

MulExp
    : UnaryExp                                          {$$ = $1;}
    | MulExp MUL UnaryExp                               {$$ = ASTNode::create_binary_op(BinaryOpFunc::Mul,$1->get_value_type(),$1,$3);}
    | MulExp DIV UnaryExp                               {$$ = ASTNode::create_binary_op(BinaryOpFunc::Div,$1->get_value_type(),$1,$3);}
    | MulExp MOD UnaryExp                               {$$ = ASTNode::create_binary_op(BinaryOpFunc::Mod,$1->get_value_type(),$1,$3);}
    ;

AddExp
    : MulExp                                            {$$ = $1;}
    | AddExp ADD MulExp                                 {$$ = ASTNode::create_binary_op(BinaryOpFunc::Add,$1->get_value_type(),$1,$3);}
    | AddExp SUB MulExp                                 {$$ = ASTNode::create_binary_op(BinaryOpFunc::Sub,$1->get_value_type(),$1,$3);}
    ;

RelExp
    : AddExp                                            {$$ = $1;}
    | RelExp LT AddExp                                  {$$ = ASTNode::create_binary_op(BinaryOpFunc::Less,$1->get_value_type(),$1,$3);}
    | RelExp GT AddExp                                  {$$ = ASTNode::create_binary_op(BinaryOpFunc::Great,$1->get_value_type(),$1,$3);}
    | RelExp LE AddExp                                  {$$ = ASTNode::create_binary_op(BinaryOpFunc::LessEqual,$1->get_value_type(),$1,$3);}
    | RelExp GE AddExp                                  {$$ = ASTNode::create_binary_op(BinaryOpFunc::GreatEqual,$1->get_value_type(),$1,$3);}
    ;

EqExp
    : RelExp                                            {$$ = $1;}
    | EqExp EQ RelExp                                   {$$ = ASTNode::create_binary_op(BinaryOpFunc::Equal,$1->get_value_type(),$1,$3);}
    | EqExp NEQ RelExp                                  {$$ = ASTNode::create_binary_op(BinaryOpFunc::NotEqual,$1->get_value_type(),$1,$3);}
    ;

LAndExp
    : EqExp                                             {$$ = $1;}
    | LAndExp AND EqExp                                 {$$ = ASTNode::create_binary_op(BinaryOpFunc::And,$1->get_value_type(),$1,$3);}
    ;

LOrExp
    : LAndExp                                           {$$ = $1;}
    | LOrExp OR LAndExp                                 {$$ = ASTNode::create_binary_op(BinaryOpFunc::Or,$1->get_value_type(),$1,$3);}
    ;

ConstExp
    : AddExp                                            {$$ = $1;}
    ;
%%
void yyerror(char * msg)
{
    std::cerr<<"[Syntax Error] line "<<line_no<<" : "<<msg << " ." << std::endl;
    exit(ErrorCode::SYNTAX_ERROR);
}
void yy_warning(char *msg)
{
    std::cerr<<"[Syntax Warning] line "<<line_no<<" : "<<msg << " ." << std::endl;
}