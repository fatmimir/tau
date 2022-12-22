# Tau

Tau is a modern procedural programming language, which right now it's kind of a toy project.

## Features
Planned features for next release:
* Procedural-oriented programming.
* Strong typed and statically typed.
* Clean, predictable and simple syntax.
* Compiling to machine-code using [MIR Backend](https://github.com/vnmakarov/mir).

Features for the future:
* Tree oriented module system.
* Abstract Data Types.
* Arrays and Dicts.
* Template types and type procedures (kind of dependent types).
* First-class procedures.
* Effects and effect handling.
* Duck-typing.
* Data Embedding.
* ARC (Automatic Reference Counting) for heap allocated types.
* A nice set of runtime modules.
  * Math
  * Time
  * String
  * Errors/Exceptions
  * Byte manipulation
  * Syscall
  * Container types
    * List
    * Ring
    * Heap
    * Stack

## Syntax

Tau syntax it's pretty straightforward, here is the [EBNF Syntax](misc/syntax_ebnf.txt) file for reference. But a summary is given bellow. 

Every file on Tau must contain at least a module declaration: 
```tau
module examples::myexample;
```

To declare functions:
```tau
proc example1(arg: Type): Type {
    return value;
}

proc example2(arg: U32): U32 = arg * 2;
proc example3(arg: U32): U32 prototype;
```

To declare bindings:
```tau
let name: Type = value;
```

To declare external references (only at file level):
```tau
module example;

extern proc putc(c: U8): Unit prototype;
extern let value: U32 prototype;
```

There are only `if` and `loop` branching facilities:
```tau
proc conditionalBranchingExample(): Unit {
    if random() {
        doSomething();
    } elif random() {
        doOtherThing();
    } else {
        doOtherThing();
    }
}

proc loopBranchingExample(): Unit {
    let i: I32 = 0;
    loop {
        i += 1;
        if i > 1000 {
            break;
        }
        
        if random() {
            continue;
        }
        
        i *= 2;
    }
}
```

Aside from `continue` and `break`, there is only a `return` statement 
which can be omitted on single-line procs:
```tau
proc add(a: I32, b: I32): I32 = a + b;
proc sub(a: I32, b: I32): I32 {
    return a - b;
}
```

Expressions work like most languages:
```tau
let a: I32 = 10;
let b: I32 = 20;
let c: I32 = 30;

// arithmetic:
let add: I32 = a + b;
let sub: I32 = a - b;
let mul: I32 = a * b;
let div: I32 = a / b;
let rem: I32 = a % b;
let neg: I32 = -a;
let pos: I32 = +a;

// relation: 
let eq: Boolean = a == b;
let ne: Boolean = a != b;
let lt: Boolean = a < n; 
let le: Boolean = a <= b;
let gt: Boolean = a > n; 
let ge: Boolean = a >= b;

// logic: 
let lAnd: Boolean = true && false;
let lOr: Boolean = true || false;
let lNot: Boolean = !true;

// bitwise:
let bAnd: U32 = 1 & 1;
let bXor: U32 = 1 ^ 1;
let bOr: U32 = 1 | 1;
let bNot: U32 = ~1; 
let bLshf: U32 = 1<<2;
let bRshf: U32 = 1>>2;

// calling:
let call: I32 = add(1, 2);

// referencing:
let d: &I32 = &a;
```

It's all the language right now, I'll include new features such more convenient
looping (for-loop, while) and matching, also I have plans to use ADTs and effect handlers.

## Type-Checking

Tau is planned to be statically type checked, meaning that all validations must be done at build time,
however I'm planing to include a small runtime to do introspection and runtime checking.

Right now the type-checker will only verify:
* Left-hand assignation check. Left hand result must match with right hand type.
* Passing args. Number and type of arguments must match declaration.

All the built-in types are the following:
```tau
// unsigned values
let a: U8 = 1;
let b: U16 = 2;
let c: U32 = 3;
let d: U64 = 4;

// signed values
let e: I8 = -1;
let f: I16 = -2;
let g: I32 = -3;
let h: I64 = -4;

// floating point (ieee-754)
let i: F32 = 1.0;
let j: F64 = 2.0;

// references and pointers
let k: &U8 = &a;

// boolean values
let l: Boolean = true 

// support types
let m: Int = 0;
let n: Uint = 0;
let o: Float = 0.0;
let p: Size = 0;
let q: Uintptr = 0;
```

There is no automatic type coalescing or promoting, all casts must be done manually, 
right now this is done by using `convert()` and `cast()` function:
```
let x: I32 = convert(12.2, I32) // reinterpretative, result is 12
let y: U32 = cast(12.2, U32) // hard cast, result is 0xC1CAC6A7
```

## Memory safety
There is no memory safety mechanisms right now.

To perform raw memory operations, you must call `_o()` and `_d()`:
```tau
let str: &U8 = "hello world"; // this is going to change in a nearby future 
let world: &U8 = _off(str, 6); // may reference invalid memory addresses
let wLetter: U8 = _deref(world); // may segfault
```

Most of this will change to make the language safer, it will be implemented:
* Bound checking for strings and arrays.
* No raw-operations over pointers at user-level.
* ARC for heap allocated structures.

