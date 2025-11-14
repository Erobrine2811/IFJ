import "ifj25" for Ifj

class Program {

    static add(a, b) {
        return a + b
    }

    static mul(a, b) {
        return a * b
    }

    static compute(a, b, c) {
        var x
        x = a + b
        return x * (c - 1)
    }

    static square(x) {
        return x * x
    }

    static main() {
        var a
        var b
        var c
        var r1
        var r2
        var r3
        var r4
        var r5

        r1 = compute(a,b,c)
        r2 = compute(r1,a,b)
        r3 = square(r3)
    }
}
