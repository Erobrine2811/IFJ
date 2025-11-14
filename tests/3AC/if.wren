import "ifj25" for Ifj

class Program {

    static main() {
        var a
        var b
        var c
        var d

        a = 1
        b = 2
        c = 3
        d = 4

        if (a < b) {
            a = a + 1

            if (b < c) {
                b = b + 2

                if (c < d) {
                    c = c + 3
                } else {
                    c = c - 3
                }

            } else {
                b = b - 2

                if (a == d) {
                    d = d + 5
                } else {
                    d = d - 5
                }
            }

        } else {
            a = a - 1

            if (b > d) {
                b = b + 10
            } else {
                b = b - 10

                if (c == 5) {
                    c = c + 1
                } else {
                    c = c - 1
                }
            }
        }
    
    }
}