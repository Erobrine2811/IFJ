import "ifj25" for Ifj

class Program {

    static main() {
        __d = 3 + 2
        var a = 5 + 3
        var b = a * 2

        b = 5
        __d = 6

        __d = Ifj.write(b * 2 + 3)
        Ifj.write(__d)
    }


    static test(c) {
        var a = 5 + 3
        var b = a * 2
    }
}
