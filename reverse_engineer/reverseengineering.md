﻿# Reverse Engineering a Stripped Binary
#### by Allan Schwartz, ams613@gmail.com


Because I was inspired by Alon Fleiss’ [recent blog](https://blogcodevalue.wordpress.com/2018/10/08/code-archaeology-how-to-revive-a-more-then-30-years-old-game/), I want to share my story also about binary code, and also from roughly 30 years ago.

Machine architecture and assembly language is very important in the embedded devices world. I was introduced to many assembly languages at the universities:

-   CDC-6100 [COMPASS](https://en.wikipedia.org/wiki/COMPASS)
-   IBM 360 [BAL](https://en.wikipedia.org/wiki/IBM_Basic_assembly_language_and_successors)
-   Intel 8080 [Assembly](https://en.wikipedia.org/wiki/Intel_8080)
-   [MIX](https://en.wikipedia.org/wiki/MIX) Assembly, for Donald Knuth's [The Art of Computer Programming](https://en.wikipedia.org/wiki/The_Art_of_Computer_Programming) class
 
At NASA, I had several projects, in 7600 COMPASS, 360 BAL, PDP 11/70 [MACRO-11](https://en.wikipedia.org/wiki/MACRO-11) Assembly, and [IMLAC PDS-1](https://en.wikipedia.org/wiki/Imlac_PDS-1) Assembly.

While working at [Bridge Communications](https://en.wikipedia.org/wiki/Bridge_Communications), where we built the first TCP/IP router, I wrote the micro-kernel. Although I wrote this in C, I debugged entirely in assembly, using a machine-code debugger within the ROM monitor. I learned to read [Motorola 68000](https://en.wikipedia.org/wiki/Motorola_68000) assembly and visualize the lines of C which generated that code.

## The Job
Skip forward nearly 10 years. I was hired for an interesting consulting job by the 3Com UK office. They had an OEM customer, British Telecom, who had contracted another consulting firm to build a protocol enabled embedded device, based on the Bridge protocol engine platform, to go into phone booths in London. However, some unknown network change made these devices unstable, so they would crash and reboot many times a day. Imagine every pay phone in London crashing several times a day.

**![](https://lh6.googleusercontent.com/sz11uWOGO57luixJPUG-57Vr1eHKnEt-BShSy86IOHko3yGnQgcVyXPYhQG37UZtxjgwVcIDcaDvaQgAiUO6mTjRmD2Nii-Yd1zRUp4Su8JGET73Wbx89vGd6re-44H--GzKs-oL)**

I said

> *Sure! No problem; send me the code, and I will identify the bug.*

However, there was a problem. They didn't have source. Apparently, the initial consulting firm had scrapped the development machine, a PDP 11/60, years before. Backup tapes? Nope, these were also scrapped because no other equipment existed to read DECtapes. Source listings? No, everything was lost.

Well, this is going to be more difficult than I thought.

## Can you debug with a stripped-Binary Object?

All I received from the customer was a single binary file, sent uuencoded over email.

**![](https://lh6.googleusercontent.com/YtEmiQ_HqxlHVrr_p3Jhpwo_-k6yozXEs-ykHguX7agLyIUY7HavYTo4i66FsCCIqjI2FqBEBa5ATEM-yxfztG-rbGlY7c4AYdj5Dz8d4-8L8ZogIL_yEmzRJG9B2sNPDpzHJwBt)**

Even after unpacking into a pure binary object file, no matter how I viewed the file, it was still just binary gibberish. I wanted to read the source code, to find the bug(s). Not even assembly code, preferably C code.

I am going to need some tools. Here was my process.

-   I ordered a Sun 3/80. At the time, my primary personal computer was a 1 MB MacPlus with a 20MB SCSI hard drive. The Sun workstation was an enormous upgrade. Since the Object code file is a standard format, the **size**(1) and **nm**(1) and **adb**(1) other Unix utilities could operate on the object file. Running certain **adb** commands on the binary will produce an assembly listing.

-   With help from 3Com, I obtained a copy of the Bridge Development Environment (toolpath) including kernel and standard protocol object files and a compiler, assembler, link editor.

-   I found a symbol table listing for the Bridge Kernel, from 5 years before. Luckily, no one had changed the kernel in the years since. This was helpful.
    
Once these tools arrived, I created the first assembly listing of the entire binary. However this listing still looked like a giant blob without symbols, or branch-point labels. Nonetheless, I was thrilled to see source code, albeit assembly. Here is portion of that first assembly listing: [reference/cfn.asm](reference/cfn.asm)

Where to start? So I started at the beginning, the first instruction was branch to the entry-point, `_start`, the C-startup code. Then `_start` calls the `main()` of the Bridge micro-kernel. Good, I had written this code about 9 years before. Lets see if I recognize any of it. I remembered key details about the Bridge micro-kernel. It had an important data-structure called `SYSINIT`, comprised of `SYSTBL`  structures. By carefully tracing the code, I found `SYSINIT`, and on the way deciphered many hex addresses into symbols.

**![](https://lh3.googleusercontent.com/TqJnj8WFfGtObbDJB9WFsnZxtOyF_nbwfSpEbrJuBjNKvgao_SjNYwVoVNjF_FW_XJSSF4GvtdOhkAbogt_pR7PYE21aOa1WduNU4qSQ3N3RhXND4XKPxhFwNBzbo2f_aqIgFW1B)**

The SYSINIT table defined the processes that would be run by the Bridge Kernel. Each SYSTBL entry was this:

```C
/*
 * SYSTBL (initial system process) STRUCTURE
 *
 * The SYSINIT table is used to form an array of initial system
 * process descriptors which the process "parent" uses to spawn
 * the boot-up processes.
 */
typedef struct systbl {
    int   (*p_initentry)();
    ADDRESS p_arg;
    char   *p_nam;
    ushort  p_pri;
    short   p_mod;
    int   (*p_mainentry)();
    int   (*p_inittest)();
    MREQ   *p_mreq;
} SYSTBL;
```

From a [hex dump](SYSTBL) of `SYSINIT`, a wrote this C-code, having peeked at the appropriate .h (header) files:

 **![](https://lh4.googleusercontent.com/CF33XtqgnaczVTzPKVahXvJDAv-yA8MoPFM4gchnpYKUV-6ATuyQkNit3UebvSdCrN2-L-zUhUQ7WuzN7oAjytnM-nIGMvg8ClOJ5B9rWdff_uv9cpnJylGgfaP1s2j7QMVWEh2J)**

Boom! My first piece of reversed-engineered C code. I have already learned a lot by studying this `SYSINIT` table. First, which protocol family they were using, and which protocol layers were included in this product.

Most essential, I learned the OEM’s custom layer was called “**CFN**” – this name was a code literal – and it was appended to the end of this table, and likewise to the end of the link (**ln**) statement in the *Makefile*. From that, I identified exactly the code I was interested in.

So let’s look at code at `cfn_main()`. (Complete listing at [reference/cfn.asm](reference/cfn.asm)).

**![](https://lh4.googleusercontent.com/xADCiIdDydvdjvlReDU7IRmRdaMKitejsusKV5_tEAqG0yVVFs7o2ey7lxk-PYveKRVDhTLXqJr-npkXlurellEIfVGAI5GOECDdOGNr90pLuIoSkf_ahJ8hwjq1QLT_zw5LIe9P)**

Notice that the Unix *adb* command can be used to disassemble. This is because the development machine shared the same CPU as the target embedded machine.

The code remains dense and hard to interpret. While listing all the code, I determined that there are 45 functions in the *cfn* layer, and some 2100 lines of assembly. However, the project is now sized and solvable.

## Time to Create Some New Tools

I need to break this task down more. After hand translating some code, I realized I needed better tools. For example, adding symbols for the function calls would make the assembly code understandable. I carefully created a [complete disassembly listing](reference/kernel.asm) of the kernel code.

The biggest challenge was to figure out the address range of the Bridge Kernel. Once disassembled, I found the `linkw a6,#n` instructions which began a function, the `rts` instruction with ended the function, and the various `bsr` instructions where C functions are called. From this exercise, I created a table of the address of each kernel function:

**![](https://lh6.googleusercontent.com/9hiUj-pvUBM9SI_wVxM9Der3cuFnknQa7R6xv0zx9Gva70OUOBwzGihHcPCAOqoU0dqwn6qIKuwD3kN0c7_MVcwkmoAGVQZVzckvjbb3_WrPf0Dodh_OGSyKXY09pIidOOwg98vC)**

Similarly, I created a [disassembly listing](reference/libc.asm) of libc, and then created a [table of addresses](reference/libc.symbols) of all the libc routines called from the kernel.

I also created symbol names for all of the fixed addresses referenced in the code. Initially, these were names without meaning. For example, variables, `cfn_g1`, `cfn_g2`, …, `cfn_d1`, `cfn_d2,` ...  (See the complete [list of globals](reference/globals)).

The next step was to insert these names into the object file and regenerate the assembly for all of the `cfn` functions. Remember that the binary I received was stripped, that is, all the symbols names were removed from the binary. I created a tool called ***unstrip*** which would do the opposite, it would take a list of symbol names and place them back into the binary.

The implementation of the *unstrip* tool is fairly interesting. (See [tools/unstrip.c](tools/unstrip.c)).

Then, again we use *adb* and disassemble the `cfn` layer. Wow, a lot better. I can begin to almost visualize the lines of C: (See [reference/cfn.asm.3](reference/cfn.asm.3)).

Having symbol names makes this assembly much more readable than before. However, in order to see the C flow control structures, I would need more help.

### Shell, AWK, and SED tools

To solve this problem, I wrote a tool called ***relabel.awk*** to mark branch-points labels and remove unneeded labels from the assembly code.

```bash
#! /bin/sh
#
#  relabel
#
nawk '
BEGIN   { ln = 0; }
/beq|bge|bgt|bhi|ble|bls|blt|bne|bra/  { label[$3] = 1; }
        { line[ln] = $0; ln++; }
END     { for ( i = 0; i < ln ; i++ ) {
            lbl = substr(line[i], 0, index(line[i],":")-1)
            if (label[lbl]) 
                printf ("\n_")
            print line[i]
            if ( line[i] ~ /beq|bge|bhi|ble|bls|blt|bne|bra/ )
                printf ("\n")
          }
          print(":3,$s/^cfn[^ ]*://")
          print(":%s/cfn_[0-9]*\\+/L_/g")
} ' -
```

After applying this *awk* code to all the assembly code, we start to see code that looks like hand-crafted assembly.

I created a tool called ***autos*** which lists the stack-referenced variables used in a function – both formal parameters to the function and local autos. The tool autos works by matching one key regular expression:

```bash
#! /bin/sh
#
# autos, print auto-stack variables
#
nawk '
/a6@/ { for (i = 1; i <=NF; i++ )
           if ( match($i,"a6@\\([-x0-9a-f]*\\)" ) )
              print substr($i,RSTART,RLENGTH)
}' $1 | sort -u
```
  
Using autos, I was able to list the auto or stack variables each function used. I could comprehensively write the C function signature and declare local variables. Next, I examined the code for register autos. Global variables were, of course, more difficult to figure out. For every variable, I would have to look at them in context to determine the size, type, and purpose; and then make up a variable name.

I wrote a tool to find the string literals in preinitialized data-space and added comments where they were referenced.

I wrote a tool called ***extract*** to extract a single `cfn` function from the larger assembly listing.

I wrote a tool called ***xref.awk***, to make a function cross-reference table.

## Beginning the Reverse-Engineering

#### The Code Generation Basics

First some background about the M680*x*0 generated code.

-  The compiler has a rather simplistic register model compared to the processor itself. Registers `a0`, `a1`, `d0`, `d1` are used for all intermediate and temporary calculations. Registers `a6` and `a7` are used for the frame and stack pointers. Registers `d2`, `d3`, `d4`, `d5`, `d6`, `d7` and `a2`, `a3`, `a4`, `a5` are not used at all, unless a register declaration is made, or a higher level of compiler optimization is used.
    
-   C functions always begin with  
```asm
        linkw a6,#-n
```

    Where *n* is the size of the local autos for this function.  
    Functions always end with 
```asm
        unlk a6  
        rts
```

-   The stack pointer `a7` *aka* `sp`. `sp` isn’t referenced in a function unless that function makes further calls, or unless register variables are declared.
    
-   C code may declare locals to use registers like this  
```C
    register int k;  
    register short len;  
    register BD *bd;  
    register char *ptr;  
```

-   In the example above, the code would use register `d7` as k, register `d6` as len, register `a5` as bd, and register `a4` as ptr. If this occurs, the registers will be saved at the beginning of the function with the
```asm
        moveml d7/d6/a5/a4,sp@
```

instruction, (where the `sp@` is auto-decrementing) and restored at the end of the function with the
```asm
        moveml sp@,d7/d6/a5/a4
```

instruction, where the `sp@` is auto-incrementing.

Students of computer science recognize that this contributes to making the function reentrant and thread-safe.
    
-   Normal C scoping would apply, however, before C89, declarations were always at the top of the function.

With that background, you can look at snippets of assembly, and perhaps see the C code which generated that code.

Note that current compilers have many more optimization algorithms and current CPUs often have more opcodes and registers, so nowadays reverse engineering modern optimized code is more complex, but there are also better open-source tools. This project was done 25 years ago, before such tools were available.

### Easy-Peasy Examples

In case you are still in disbelief that reverse engineering into C could even be done, I want to show some of the less complicated examples.

Here are a five examples of short assembly functions, that is, less than 20 lines of assembly each.

- - - -
#### EXAMPLE 1 - Function cfn_8() *aka* cfn_cksum() - illustrates the *for* loop

```asm
                                        ;unsigned char cfn_cksum(char *txt)
                                        ;{
cfn_8:  linkw a6,#-4
        clrb  a6@(-1)                   ;unsigned char sum = 0;
        bras  L2                        ;for( ; *txt; txt++) {

L1:     movl  a6@(8),a0

        movb  a0@,d0
        eorb  d0,a6@(-1)                ;sum ^= *txt;
        addql #1,a6@(8)                 ;//txt++;

L2:     movl  a6@(8),a0
        tstb  a0@
        bnes  L1                        ;}

        movb  a6@(-1),d0                ;return sum;
        unlk  a6
        rts                             ;}
```

The above 13 lines of assembly translate to these couple lines of C.
```C
unsigned char cfn_cksum(char *txt)      /* a6@(8) */ 
{
    unsigned char sum = 0;              /* a6@(-1) */

    for (; *txt; txt++) {
        sum ^= *txt;
    }
    return sum;
}
```

From this example we learn the following code generation rules:

1.  C function parameters, which have been pushed onto the stack by the caller, are referenced as `a6@(n)` where *n* is a positive number.
    
2.  Local auto variables in the function are referenced as `a6@(-n)`.
    
3. `for` loops of the format:

```C
        for (exp1; exp2; exp3) {
            statement-list;
        }
```
Generate code like this:

```asm
        exp1
        bras  L2
L1:     statement-list
        exp3
L2:     exp2
        bnes L1
```

It is essential to recognize the **for** loops, **while** loops and other C flow control. As one might expect, spaghetti code is quite difficult to reverse engineer.

- - - -
#### EXAMPLE 2 - Function cfn_10() *aka* cfn_gethash() - illustrates a function call and *if* construct

```asm
                                        ;int cfn_gethash(char *str)
                                        ;{
cfn_10: linkw   a6,#-4
        moveml  d7,sp@                  ;register int val;       // d7

        movl    a6@(8),sp@-
        bsr     cfn_ascii_to_hex        ;val = cfn_ascii_to_hex(str);
        addql   #4,sp

        movl    d0,d7                   ;if ( val == -1 ) {
        moveq   #-1,d1
        cmpl    d1,d0
        bnes    L1

        moveq   #-1,d0                  ;return -1;
        bras    L2                      ;}

L1:     andl    #0xff,d7
        movl    d7,d0                   ;return (val & 0xff);

L2:     moveml  sp@,d7
        unlk    a6
        rts                             ;}
```


The above 16 lines of assembly translate to these few lines of C.
```C
int cfn_gethash(char *str)
{
    register int val;                   /* d7 */

    if ((val = cfn_ascii_to_hex(str)) == -1) {
        return -1;
    }

    return (val & 0xff);
}
```

This function converts an ascii string, of presumably hexadecimal bytes, into a 32-bit value, and then returns the low-order 8 bits of this value. I imagine this is used to create some sort of hash value.
From this example, we learn a few more code generation rules.

1.  Function calls consist of pushing the arguments onto the stack, illustrated by the instruction  
```
        movl a6@(8),sp@-
```


Where `a6@(8)` is the argument to *cfn_ascii_to_hex*(), and also the first function argument. Then, `sp@-`, means “onto the stack, with the stack pointer auto decrementing”. Then the function is called  
```asm
        bsr cfn_ascii_to_hex
```

Upon return, the SP is restored with the equivalent of a “pop” instruction  
```asm
        addql #4,sp  
```

2.  If the function returns a value, that value is in `d0`, as evidenced in the next line of assembly  
```asm
        movl d0,d7  
```

3.  if statements of the form  
```C
    if (expression == k) {
        statement-list;
    }
```


Generate code like this:
```asm
        calculate expression in d0
        moveq   #k,d1
        cmpl    d1,d0
        bnes    L1
        statement-list
L1:     ...
```

- - - -
#### EXAMPLE 3 - Function cfn_37() - illustrates structure members
```asm
                                        ;cfn_37(register CXDATA* cxdata) 
                                        ;{
cfn_37: linkw   a6,#-4
        moveml  a5,sp@
        movl    a6@(8),a5

        pea     0x201:w
        pea     a5@(0x22)
        bsr     cx_fill_ff              ;cx_fill_ff(cxdata->cx_databuf, 513);
        addql   #8,sp

        lea     a5@(0x22),a0
        movl    a0,a5@(0x224)           ;cxdata->cx_dataptr = cxdata->cx_databuf;

        clrw    a5@(0xa)                ;cxdata->cx_datastate = 0;

        moveml  sp@,a5
        unlk    a6
        rts                             ;}
```


The above 13 lines of assembly translate to these few lines of C.
```C
cfn_37(register CXDATA *cxdata)
{
    cx_fill_ff(cxdata->cx_databuf, 513);
    cxdata->cx_dataptr = cxdata->cx_databuf;
    cxdata->cx_datastate = 0;
}
```

What we learn from this little snippet of code, is that `cxdata` points to a packet. The packet consists of a data portion `cx_databuf`, which is a continuous memory buffer of 513 bytes, and a header which includes a pointer to the data portion `cx_dataptr`.

We also see how the compiler generates references to members within the structure, for example:
```asm
        lea     a5@(0x22),a0            ;; &cx_data->cx_databuf
```

- - - -
#### EXAMPLE 4 - Function cfn_40() - Found our first probably bug
```asm
                                        ;BOOL  cfn_40(register CXDATA *cxdata)
                                        ;{
cfn_40: linkw   a6,#-8
        moveml  a4/a5,sp@               ;register char *p;        // a4
  
        movl    a6@(8),a5
        movl    a5@(0x224),a4           ;p = cxdata->cx_dataptr;
  
        addql   #1,a4                   ;p++;
        cmpb    #0xff,a4@               ;if ( *p != 0xff )
        beqs    L_0x22                  ;{
  
        movw    #0xb,a5@(0xa)           ;cxdata->cx_datastate = 0xb;
  
        moveq   #1,d0                   ;return 1;
        bras    L_0x24                  ;}
  
L_0x22: moveq   #1,d0                   ;return 1;
L_0x24: moveml  sp@,a4/a5
        unlk    a6
        rts                             ;}
```

The above 14 lines of assembly translate to these few lines of C.
```C
BOOL  cfn_40(register CXDATA *cxdata)
{
    register char *pp;                  /* a4 */

    pp = cxdata->cx_dataptr;
    pp++;
    if (*pp != -1) {
        cxdata->cx_datastate = 0x0b;
        return TRUE;
    }
    return TRUE;  
}
```
 
This example doesn’t teach us anything new about code generation. However, we learn something about how the data buffer, pointed to by `cx_dataptr` is being used, and this function seems to return *TRUE* if the packet is empty (or the second element == `0xff`), and *TRUE* otherwise. This looks like a bug to me. Also, we learn that member `cx_datastate` is some kind of state variable, and `0x0b` is one value.

- - - - 
#### EXAMPLE 5 - Function cfn_44() *aka* cx_countbytes() - illustrates a *while* loop, and auto-incrementing pointers

```asm
                                        ;short cx_countbytes(REG uchar *p)
                                        ;{
cfn_44: linkw   a6,#-8
        moveml  d7/a5,sp@               ;REG short k;                  // d7
  
        movl    a6@(8),a5               ;// p                          // a5
        clrw    d7                      ;k = 0;
        bras    L_0x12                  ;while (*p++ != 0xff) {
L_0x10: addqw   #1,d7                   ;k++;
L_0x12: cmpb    #0xff,a5@+
        bnes    L_0x10                  ;}
  
        movw    d7,d0                   ;return k;
        moveml  sp@,d7/a5
        unlk    a6
        rts                             ;}
```


The above 12 lines of assembly translate to these few lines of C.
```C
short cx_countbytes(register unsigned char *p)
{
    register short k;                   /* d7 */

    k = 0;
    while (*p++ != 0xff) {
        k++;
    } 
    return k;
}
```

We notice that this could be a for loop, or it could be a while loop. The code pattern is very close. Also, notice that the expression `*p++` compiles very efficiently into a reference through `a5`, and an auto-increment of `a5` in the same instruction. That's pretty cool!

This function seems to count the number of non-`0xff` bytes in the CX data packet.

- - - - 

From these examples, we can see that it is not all misery and difficulty. In fact, it was was quite fun deciphering the code to this point. Once these small functions are understood, along with the *clib* and Bridge Kernel calls, the meaning of longer functions which use these building blocks, becomes clear.

### Next Comes the Tedious Work

Over the next month, I reverse-engineered the other 40 functions into C code. As I did it and learned more about the protocol layer being implemented, I assigned better names to these 45 functions, which were initially called `cfn_main()`, `cfn_1()`, `cfn_2()`, `cfn_3()` …

The list of symbol names continued to be added to the file *symbols.sorted*. This was again used as input to the program *unstrip*. I would insert these names back into the object binary with *unstrip*, and then disassemble then whole layer again. Or, I would use [this sed script](tools/edit_cmds) to change to names in the source codes.

In the end, I had a rather complete symbol table, [reference/symbols.sorted](reference/symbols.sorted), derived from not only all the function names I discovered in my disassembly, but also other globals referenced in the code.

As a final example, here is an assembly code function of more average length and complexity. We notice that the `L1_ADDR` structure `addr` is call by value, so the entire structure is on the stack.

 - [asm/cfn_5](asm/cfn_5)

After quite a bit of study, clean-up, and documentation, the corresponding C code for cfn_5 is this:

- [c/cfn_5.c](c/cfn_5.c)

```C
/********************************************************************* *
 *  $Header:$
 *
 *  NAME
 *      cfn_sendidpdata(bd, src_socket, addr)
 *
 *  DESCRIPTION
 *      cfn_sendidpdata() sends an IDP packet to the specified destination
 *      address.
 *
 *  FUNCTIONS CALLED
 *      cfn_reboot, cfn_error, freemsg, getmaxmsg, sendmsg, sprintf
 *
 *  CALLED BY
 *      cfn_34, cfn_36, cfn_send2sock, cfn_error
 *
 *  ARGUMENTS
 *      ~~~
 *
 *  HISTORY
 *      reversed engineering from binary, June/July 1991,
 *      by Allan M. Schwartz
 *
 *  BUGS
 *      If there are no messages, this routine will be called recursively.
 *
 ********************************************************************/

#include "cfn.h"

cfn_sendidpdata(bd, src_socket, addr)
    BD         *bd;             /* a6@(8) */
    short       src_socket;     /* a6@(0xe) */
    L1_ADDR     addr;           /* a6@(0x10) */
{
    REG short   rc;             /* d7 */
    REG MSG    *msg;            /* a5 */

    if ((msg = getmaxmsg()) == NULL) {
        sprintf(cfn_linebuf, "%s: out of memory in getmsg", 
            cfn_hostaddr);
        cfn_error(cfn_linebuf);
        cfn_reboot();
    }

    MOVEL1ADDR(((IDL2_SDATAMSG *) msg)->idsd_sadd, cfn_attnet);

    ((IDL2_SDATAMSG *) msg)->idsd_sadd.l1_sock = src_socket;

    MOVEL1ADDR(((IDL2_SDATAMSG *) msg)->idsd_dadd, addr);

    ((IDL2_SDATAMSG *) msg)->idsd_ptype = 0x63; /* protocol type */
    msg->m_bufdes = bd;
    msg->m_prio = NORMAL;
    msg->m_type = MIDSDATA;     /* IDP send data */

    rc = sendmsg(msg, idu2nmbox);

    if (rc != NoError) {
        freemsg(msg);
    }
}
```


## Resulting ASM and C Files

Eventually, I produced 45 assembly files with some C code written as comments or margin notes. These margin notes became complete, equivalent to the original C source code.

I extracted the C code, (with another tool), and made sure each one compiled.

Here are all 45 Assembly, reverse-engineered C files, and calling signatures:

#### Table of Asm files, C files, and calling signatures

|  |  |  |
|--|--|--|
|[cfn_0](asm/cfn_0)|[cfn_0.c](c/cfn_0.c)|`void cfn_main()`|
|[cfn_1](asm/cfn_1)|[cfn_1.c](c/cfn_1.c)|`void cfn_initworld()`|
|[cfn_2](asm/cfn_2)|[cfn_2.c](c/cfn_2.c)|`void cfn_receive()`|
|[cfn_3](asm/cfn_3)|[cfn_3.c](c/cfn_3.c)|`void cfn_set_alarm()`|
|[cfn_4](asm/cfn_4)|[cfn_4.c](c/cfn_4.c)|`void cfn_send2sock(short sock)`|
|[cfn_5](asm/cfn_5)|[cfn_5.c](c/cfn_5.c)|`void cfn_sendidpdata(BD *bd, short sock, L1_ADDR addr)`|
|[cfn_6](asm/cfn_6)|[cfn_6.c](c/cfn_6.c)|`void cfn_error(char *txt)`|
|[cfn_7](asm/cfn_7)|[cfn_7.c](c/cfn_7.c)|`char *cfn_fmt_txt_pkt(char *dst, char *txt1, char *txt2)`|
|[cfn_8](asm/cfn_8)|[cfn_8.c](c/cfn_8.c)|`uchar cfn_cksum(uchar *p)`|
|[cfn_9](asm/cfn_9)|[cfn_9.c](c/cfn_9.c)|`int cfn_ascii_to_hex(uchar *p)`|
|[cfn_10](asm/cfn_10)|[cfn_10.c](c/cfn_10.c)|`int cfn_gethash(uchar *str)`|
|[cfn_11](asm/cfn_11)|[cfn_11.c](c/cfn_11.c)|`void cfn_tcreate(int i, char *name, L1_ADDR addr)`|
|[cfn_12](asm/cfn_12)|[cfn_12.c](c/cfn_12.c)|`void cfn_update(int index, L1_ADDR addr)`|
|[cfn_13](asm/cfn_13)|[cfn_13.c](c/cfn_13.c)|`BD *cfn_str_to_bd(char *str, short flag)`|
|[cfn_14](asm/cfn_14)|[cfn_14.c](c/cfn_14.c)|`void cfn_panic(char *str)`|
|[cfn_15](asm/cfn_15)|[cfn_15.c](c/cfn_15.c)|`void cfn_panic2(char *str,arg2)`|


#### Table (continued) of Asm files, C files, and calling signatures

|  |  |  |
|--|--|--|
|[cfn_16](asm/cfn_16)|[cfn_16.c](c/cfn_16.c)|`void cfn_dumptable()`|
|[cfn_17](asm/cfn_17)|[cfn_17.c](c/cfn_17.c)|`void cfn_print1_tentry(int index)`|
|[cfn_18](asm/cfn_18)|[cfn_18.c](c/cfn_18.c)|`void cfn_tableinit()`|
|[cfn_19](asm/cfn_19)|[cfn_19.c](c/cfn_19.c)|`void cfn_reboot()`|
|[cfn_20](asm/cfn_20)|[cfn_20.c](c/cfn_20.c)|`void cx_psp_packet(BD *bd, short flag)`|
|[cfn_21](asm/cfn_21)|[cfn_21.c](c/cfn_21.c)|`int cx_atoh_n(char *str, short count)`|
|[cfn_22](asm/cfn_22)|[cfn_22.c](c/cfn_22.c)|`uchar cx_psp_cksum(BD *bd)`|
|[cfn_23](asm/cfn_23)|[cfn_23.c](c/cfn_23.c)|`void cx_init(short index)`|
|[cfn_24](asm/cfn_24)|[cfn_24.c](c/cfn_24.c)|`void cx_main(MSG *msg, MBID mbid)`|
|[cfn_25](asm/cfn_25)|[cfn_25.c](c/cfn_25.c)|`void cx_listenmsg(MSG *msg)`|
|[cfn_26](asm/cfn_26)|[cfn_26.c](c/cfn_26.c)|`void cx_connectmsg(MSG *msg)`|
|[cfn_27](asm/cfn_27)|[cfn_27.c](c/cfn_27.c)|`void cs_disconnectmsg(MSG *msg)`|
|[cfn_28](asm/cfn_28)|[cfn_28.c](c/cfn_28.c)|`void cx_IDPdatamsg(MSG *msg)`|
|[cfn_29](asm/cfn_29)|[cfn_29.c](c/cfn_29.c)|`void cs_printpkt(BD *bd)`|
|[cfn_30](asm/cfn_30)|[cfn_30.c](c/cfn_30.c)|`PSPPKT cx_xr_packet(BD *bd, L1_ADDR addr, ushort sock)`|


#### Table (continued) of Asm files, C files, and calling signatures

|  |  |  |
|--|--|--|
|[cfn_31](asm/cfn_31)|[cfn_31.c](c/cfn_31.c)|`BOOL cfn_31(BD *bd, L1_ADDR *addrp, short port)`|
|[cfn_32](asm/cfn_32)|[cfn_32.c](c/cfn_32.c)|`BOOL cx_xr_psp_valid(PSPPKT *psp, BD *bd, L1_ADDR addr)`|
|[cfn_33](asm/cfn_33)|[cfn_33.c](c/cfn_33.c)|`void cfn_33(PSPPKT *psp, BD *bd, short port)`|
|[cfn_34](asm/cfn_34)|[cfn_34.c](c/cfn_34.c)|`cx_psp_reply(PSPPKT *psp, BD *bd, L1_ADDR addr, ushort sock)`|
|[cfn_35](asm/cfn_35)|[cfn_35.c](c/cfn_35.c)|`void cx_SAdatamsg(MSG *msg)`|
|[cfn_36](asm/cfn_36)|[cfn_36.c](c/cfn_36.c)|`BOOL cfn_36(CXDATA *csdata)`|
|[cfn_37](asm/cfn_37)|[cfn_37.c](c/cfn_37.c)|`void cfn_37(CXDATA *cxdata)`|
|[cfn_38](asm/cfn_38)|[cfn_38.c](c/cfn_38.c)|`BOOL cfn_38(CXDATA *cxdata)`|
|[cfn_39](asm/cfn_39)|[cfn_39.c](c/cfn_39.c)|`BOOL cfn_39(CXDATA *cxdata)`|
|[cfn_40](asm/cfn_40)|[cfn_40.c](c/cfn_40.c)|`BOOL cfn_40(CXDATA *cxdata)`|
|[cfn_41](asm/cfn_41)|[cfn_41.c](c/cfn_41.c)|`BOOL cfn_41(CXDATA *cxdata)`|
|[cfn_42](asm/cfn_42)|[cfn_42.c](c/cfn_42.c)|`char *cfn_42(char *pp, CSDATA *cxdata)`|
|[cfn_43](asm/cfn_43)|[cfn_43.c](c/cfn_43.c)|`BOOL cfn_43(BD *bd, CXDATA *cxdata )`|
|[cfn_44](asm/cfn_44)|[cfn_44.c](c/cfn_44.c)|`short cx_countbytes(uchar *p)`|
|[cfn_45](asm/cfn_45)|[cfn_45.c](c/cfn_45.c)|`void cx_fill_ff(uchar *p, short cnt)`|


Analyzing all the global variables, some of which are structures, was quite difficult. However, the process is no different then when you come across a poorly documented and poorly named variable in normal source code. One *greps* across all the files, and figures out how it is being used and what it could be.

Next we need the header files, which were also incrementally developed. They enabled me to compile the C files, and check that the resultant assembly code matches the original.

-   See [c/cfn.h](c/cfn.h)

-   See [c/cfn_externs.h](c/cfn_externs.h)

    By the way, notice that C calling signatures was not a developed concept back then. In fact, our compiler didn’t even accept ANSI C89 standards, as you can see in each function declaration.
    
-   See [c/cfn_structs.h](c/cfn_structs.h)
    
-   See [c/cfn_vars.h](c/cfn_vars.h)

I also analyzed the task message flow and built state tables. (See [c/msg_types](c/msg_types)).


## Conclusion

Did I find bugs? Absolutely. When you examine every line of code, from the machine level code on up, you are studying all cross references, and carefully reconstructing the control flow, the bugs leap off the page and scream at you. In my C listing, I identify a number of bugs, memory leaks, and other issues.

In short, there are a couple of error conditions or corner cases which were not well tested. These errors often led to a memory leak of the msg space, and eventually *getmsg()* would fail. In reporting *getmsg()* failing, more memory would be leaked … and very soon the machine would crash and reboot.


### Epiphanies

1.  In retrospect, this was the best hack of my career.
2.  The Sun 3/80 was a way-cool Unix machine. I’m sorry I got rid of that machine.
3.  No consulting job is too hard.
4.  ***We should never complain about debugging.***

    -   *Now* we have sophisticated tools.
    -   *Now*, we have the customer’s exact runtime environment.
    -   *Now*, we have (or should have) unit-tests.
    -   *Now* we have a supportive team.
    -   And, most significantly, *now* we have ***source code*** to work with.

<!--
<script type="text/javascript" src="//cdnjs.cloudflare.com/ajax/libs/prism/1.15.0/prism.js"></script>
<script type="text/javascript" src="//cdnjs.cloudflare.com/ajax/libs/prism/1.15.0/components/prism-bash.js"></script>
<script type="text/javascript" src="//cdnjs.cloudflare.com/ajax/libs/prism/1.15.0/components/prism-c.js"></script>
<script type="text/javascript" src="//cdnjs.cloudflare.com/ajax/libs/prism/1.15.0/components/prism-python.js"></script>
-->
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTExOTE3Njk1MF19
-->
