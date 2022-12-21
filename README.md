# Tau

Tau is a modern procedural programming language, which right now it's kind of a toy project.


## Syntax

Syntax looks like:

```tau
module example

extern proc putc(c: U8): Unit
extern proc randBool(): Boolean

proc main(): I32 {
    let h: I32 = 72
    let e: I32 = 69
    let l: I32 = 75
    let l: I32 = 75
    let o: I32 = 79


    let i: I32 = 0
    loop {
        if randBool() {
            continue
        } elif randBool() {
            break
        } else {
            putc(h)
            putc(e)
            putc(l)
            putc(l)
            putc(o)
        }

        i += 1
    }

    return 0
}

```
