# IR设计文档
## 指令集
Type| Format |comment
:-: | :-: | :-: 
Label|  \<lbl0\>|
BinaryCalc| \<tr\> = \<ta\|la\|pa\|va\|ga\> \<op\> \<tb\|lb\|pb\|vb\|gb\>|    
UnaryCalc|  \<tr\> = \<op\> \<ta\|la\|pa\|va\|ga\>
Assign|     \<tr\|lr\|pr\|gr\> = \<ta\|la\|pa\|va\|ga\>|
ArrayLoad|       \<tr\|lr\|pr\|gr\> = \<la\|pa\|ga\>[\<tb\|lb\|pb\|gb\|vb\>]
ArrayStore|      \<lr\|pr\|gr\>[\<ta\|la\|pa\|ga\|va\>] = \<tb\|lb\|pb\|vb\|gb\>
Goto|       goto \<lbl0\>
CondGoto|   goto \<ta\|la\|pa\|ga\|va\>?\<lbl1\>:\<lbl0\>
Call|       call \<ga\>,param \<vb\>
CallWithRet|\<tr\> = call \<ga\>,param \<vb\>
Return|    return
ValReturn| return \<ta\|la\|pa\|ga\|va\>
FuncDef|   func \<ga\>,param \<vb\>
FuncEnd|   func end
FParam|   fparam \<pa\>
RParam|    rparam \<ta\|la\|pa\|va\|ga\>
GlobalDecl|global \<ga\>
LocalDecl| local  \<la\>
BlockGoto|      ucond goto
BlockCondGoto|  cond goto \<ta\|la\|pa\|ga\|va\>
PhiFunc|   \<lr\> = \<phia \<l1\> \<l2\> ...\>|在SSA形式下使用

<br/>

## 操作数内存类型

Type|Description| Comment
:-: | :-: | :-:
\<t\>|临时变量|存储临时结果，该类型变量只能被定值一次
\<l\>|局部变量|源程序中的局部变量
\<p\>|形式参数|源程序中的形式参数
\<g\>|全局符号|全局变量和全局函数
\<v\>|立即数|
\<phi\>|φ函数|SSA形式中使用，可以有多个l/p型参数
\<lbl\>|标签|线性IR中描述一个目标，用于跳转

<br/>

## 操作数值类型

Type|Description| Comment
:-: | :-: | :-:
i32|32位整数类型
i32*|32位整数指针类型
f32|32位浮点数类型
f32*|32位浮点数指针类型

<br/>

## 函数定义和调用规则
### 1、函数定义
在线性IR中，FuncDef和FuncEnd成对出现，两者之间描述了一个函数的定义。

函数按照指令类型，从前到后分为三个部分。

（1）定义区

        首先按顺序为所有形参的定义，然后为函数内所有局部变量的定义。

（2）初始化区

        对所有具有初值的局部变量做初始化。

（3）功能区

        函数的其他功能指令

在控制流图中，函数的定义区和其他部分分开存放，定义区第一条指令为FuncDef，但不包含FuncEnd指令。函数其他部分存放在基本块中

### 2、函数调用
函数调用时，首先使用RParam指令按顺序描述所有参数，然后紧跟一条Call或CallWithRet指令进行函数的调用

<br/>

## 符号编号规则
1、所有符号的编号从0开始，每一个函数独立

2、\<lbl\>的编号独立，\<l\>、\<p\>和\<t\>编号混用

3、\<l\>、\<p\>可以被转换为SSA形式，该形式下，每个变量额外附加SSA编号。其中SSA编号从0开始，依次递增。SSA编号为0表示该变量未经修改的原始值（对于\<l\>型符号，使用该值是未定义行为），变量被重新赋值时，将会被分配一个新的SSA编号。

4、SSA形式下，定义语句中的符号也为SSA形式，其SSA编号为0。每一个SSA形式符号需要维护访问原形式符号的方法，以便于将SSA形式转回原形式。
<br/>

## 其他规则

### 1、BinaryCalc指令操作数
Type|Description| Comment
:-: | :-: | :-:
AddI|r=a+b|只有a可以为指针型，此时b为i32型，r为指针型
SubI|r=a-b|
MulI|r=a*b|
DivI|r=a+b|
ModI|r=a%b|
EqualI,|r=a==b|关系运算
NotEqualI|r=a&ne;b|关系运算
GreaterI|r=a\>b|关系运算
LessI|r=a\<b|关系运算
GreaterEqualI|r=a&ge;b|关系运算
LessEqualI|r=a&le;b|关系运算
AndI|r=a&&b|逻辑运算
OrI|r=a\|\|b|逻辑运算
AddF|r=a+b|
SubF|r=a-b|
MulF|r=a*b|
DivF|r=a+b|
EqualF,|r=a==b|关系运算
NotEqualF|r=a&ne;b|关系运算
GreatF|r=a\>b|关系运算
LessF|r=a\<b|关系运算
GreatEqualF|r=a&ge;b|关系运算
LessEqualF|r=a&le;b|关系运算
AndF|r=a&&b|逻辑运算
OrF|r=a\|\|b|逻辑运算


#### 一般规则（如果和特殊规则冲突，特殊规则优先）

(1)操作数a、b和结果类型三者相同

(2)只有AddI运算可以有指针型操作数

(3)关系运算的结果一定为i32型，且取值为0或1

(4)逻辑运算的操作数一定为i32型，结果也一定为i32型，结果取值为0或1

(5)逻辑运算操作数为0时认定为逻辑真，操作数不为0时认定为逻辑假

(6)以I结尾的指令处理i32类型数据，以F结尾的指令处理f32型数据

<br/>

### 2、UnaryCalc指令操作数
Type|Description| Comment
:-: | :-: | :-:
IToF|r=i32->f32 a|a为i32型，r为f32型
FToI|r=f32->i32 a|a为f32型，r为i32型
NegI|r=-a|
NotI|r=!a|逻辑运算
NegF|r=-a|
NotF|r=!a|逻辑运算

#### 一般规则（如果和特殊规则冲突，特殊规则优先）

(1)操作数a和结果r类型相同

(2)操作数不能为指针型

(3)逻辑运算的操作数一定为i32型，结果也一定为i32型，结果取值为0或1

(4)逻辑运算操作数为0时认定为逻辑真，操作数不为0时认定为逻辑假

(5)以I结尾的指令处理i32类型数据，以F结尾的指令处理f32型数据