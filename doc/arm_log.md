# 符号规范
## 局部非数组变量
活跃区间：[定值1,使用1],[定值2,使用2],...
是否分配寄存器：是,用于存放数据
内存分配：在函数开始时开辟局部变量空间
内存读写情况：
    当该变量需使用并且spill,则需要load(占用某寄存器rn)
        push {rn}
        ldr rn, [sp, #x]
        op rd, rn, rs
        pop {rn}
    当该变量需定值并且spill,则需要store(占用某寄存器rn)
        push {rn}
        op rn, rt, rs
        str rn, [sp, #x]
        pop {rn}
## 局部数组变量
活跃区间：[0,最后使用]
是否分配寄存器：是，用于存放数组首地址(不变)
内存分配：在函数开始是分配一个首地址空间,和与数组长度匹配的数据存储空间
内存读写情况：
    使用时(ArrayLoad),以[首地址rn,偏移量rf/imm]寻址load
        ldr rt, [rn, rf/imm]
        op rd, rt, rs
    定值时(ArrayStore),以[首地址rn,偏移量rf/imm]寻址store
        op rd, rt, rs
        str rd, [rn, rf/imm]
## 全局非数组变量
活跃区间：无
是否分配寄存器：否
内存分配：数据段
内存读写情况：
    使用时,占用某寄存器rn
        push {rn}
        ldr rn, =A
        ldr rn, [rn]
        op rd, rn, rs
        pop {rn}
    定值时,占用某寄存器rn,rm
        push {rn}
        op rn, rt, rs
        push {rm}
        ldr rm, =A
        str rn, [rm]
        pop {rm}
        pop {rn}
## 全局数组变量
活跃区间：无
是否分配寄存器：否
内存分配：数据段
内存读写情况：
    使用时,以[首地址rn,偏移量rf/imm]寻址(占用寄存器rn,rm)
        push {rn}
        mov32 rn, A
        ldr rn, [rn, rf/imm]
        op rd, rn, rs
        pop {rn}
    定值时,以[首地址rn,偏移量rf/imm]寻址(占用寄存器rn,rm)
        push {rm}
        op rm, rt, rs
        push {rn}
        mov32 rn, A
        str rm, [rn, rf/imm]
        pop {rn}
        pop {rm}
## 立即数
活跃区间：所处指令那一句
是否分配寄存器：是(需要寄存器的立即数)
内存分配：不分配
需要寄存器的立即数：
    1、乘除模运算的立即数
    2、ArrayStore的立即数
    3、不符合imm8m的立即数


# 保存与恢复现场
## 调用者
1. 保护现场，不活跃的寄存器或者函数返回值将要赋值给该寄存器就不用存回去，其余的存回去作为caller_save；此外，如果r0-r3之前保存着函数前4个int实参或者s0-s15之前保存着函数前16个float实参也要存回去（但不作为caller_save）

2. 传参时先将第4个int参数以后的存入栈中，再对r0-r3进行赋值；传参时先将第16个float参数以后的存入栈中，再对s0-s15进行赋值
3. 对r0-r3赋值，分为以下几种情况：例，rparam a （s0-s15同理）
   
    （1）a不是spill或者a是常数且满足operand2，且a的寄存器不是a0-a3

        mov r0, rx

    （2）a不是spill，且a的寄存器是a0-a3之中，将a在内存中的值取出来赋值给r0

        ldr r0, [sp, #offset] 

    （3）a是spill且a不是常数，直接将a存在内存的值取到r0中
    （4）a是spill且a是常数且不满足operand2

        mov32, r0, #imm
    
4. 调用函数
5. 如果函数调用语句（带返回值）等式左边的符号spill，则将返回值r0(s0)存进对应的内存中

6.  如果函数调用语句（带返回值）等式左边的符号没有spill，将返回值取到相应的寄存器中
    
    mov rd, r0  或
    mov sd, s0
7.  恢复现场，将caller_save的值取回寄存器





