# monadic - stack-based esolang where everything is curried

## Syntax

The program is a space-separated list of literals. A number or string calls the topmost function on the stack and replaces it with the result. A lone `^` will call the second-top-most function with the topmost function as the argument, while a lone `-` will pop a value from the stack. (used to remove null values returned by statements like `print`). Any other identifier must be a valid function name, and pushes the function, onto the stack.

### Examples

1. `#3` is the unsigned integer 3.
2. `~5.5` is the single-precision floating point number 5.5.
3. `'Hello,%20world!` is the string `"Hello, world!"`. A percent and two hex digits encodes a UTF-8 byte with the corresponding value, while two percent signs (`%%`) encodes a percent sign.
4. `addi #3 #4` adds the integers 3 and 4.
5. `mcall addi #3 ^ #4` first calls `addi #3`, returning a new function. `^` calls `mcall` on the new function, effectively binding the first argument. (remember, all functions are curried, so `mcall func` is similar to e.g. `mcall.bind(null,func)` in JavaScript.). Then, the result is called with `#4`, actually performing the addition.
6. `println #3` puts an empty value on the stack. This value is erroneous -- you cannot do anything while it is on the stack. The `-` operator is used to pop the empty value: `println #3 -`.

### Built-in functions

- `print x` prints the value x.
- `println x` prints the value x with a trailing newline.
- `addi x y` adds the integers x and y.
- `addf x y` adds the floats x and y.
- `mcall f x` invokes `x` on `f`; note that `^` must be used: `f x` is equivalent to `mcall f ^ x`.
- `dcall f x y` invokes `x` on `f` and `y` on the result.

## Building

After cloning, create empty directories `bin`, `obj` and `src` in the project root and run `make bin/test` (or `make bin/test.exe` on Windows msys2). Then, run the binary with CWD set as the project root (**not** the bin folder!)
