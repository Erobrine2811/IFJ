import "ifj25" for Ifj

class Program {

    static add(a, b) {
        return a + b
    }

    static mul(a, b) {
        return a * b
    }

    static mod(a, b) {
        return a - (a / b) * b
    }

    static calc(a, b, c) {
        var x
        x = a * b + c
        return x - b
    }

    static main() {
        var a
        var b
        var c
        var d
        var e
        var f

        a = 5
        b = 3
        c = 10
        d = 2
        e = 1
        f = 0

        while (a < 50) {
            if (a < b) {
                a = a + 1
            } else {
                a = a + b
            }

            if (c > a) {
                c = c - d
            } else {
                c = c + e
            }

            while (b < 20) {
                if (b == e) {
                    b = b + a
                } else {
                    b = b + f + 1
                }

                if (b > 15) {
                    b = b + 2
                } else {
                    b = b - 1
                }

                d = mul(b, 2)
            }

            e = add(a, d)
            f = calc(e, b, c)

            if (f < b) {
                f = f + 10
            } else {
                f = f - 5
            }

            a = a + 3
        }

        while (f < 200) {
            if (f > 100) {
                f = f - b
            } else {
                f = f + a
            }

            if (a < d) {
                a = a + e
            } else {
                a = a - e
            }

            c = mul(a, b)
            d = calc(c, f, e)

            while (e < 30) {
                if (e == 0) {
                    e = e + 3
                } else {
                    e = e + 1
                }

                if (d > c) {
                    d = d - 1
                } else {
                    d = d + 2
                }
            }

            f = f + d
        }
    }
}
