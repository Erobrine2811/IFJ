import "ifj25" for Ifj

class Program {

    static main() {
        var a
        var b
        var c
        var d
        var e
        var f

        a = 0
        b = 5
        c = 3
        d = 10
        e = 2
        f = 1

        while (a < 20) {
            if (a < 7) {
                a = a + 2
            }

            if (a == 8) {
                a = a + 1
            }

            while (c < 10) {
                if (c + 2 == 0) {
                    c = c + 3
                }
                c = c + 1
            }

            a = a + 1
        }

        while (b > 0) {
            if (b > 3) {
                b = b - 1
            }

            while (e < 10) {
                if (e + f < a) {
                    e = e + 2
                }
                e = e + 1
            }

            b = b - 1
        }

        while (d > 0) {
            if (d + 3 == 0) {
                d = d - 2
            }

            while (f < 30) {
                if (f < 10) {
                    f = f + 3
                }

                if (a > b) {
                    f = f + 1
                }

                f = f + 1
            }

            d = d - 1
        }

        while (a < 50) {
            if (a + b < c * d) {
                a = a + b - e
            }

            while (c < 40) {
                if (c + 5 == 0) {
                    c = c + 4
                }
                c = c + 2
            }

            a = a + 1
        }

        while (f < 100) {
            if (f < e * 3) {
                f = f + a - b
            }

            while (d < 20) {
                if (d + c < a) {
                    d = d + 3
                }
                d = d + 1
            }

            f = f + 2
        }
    }
}
