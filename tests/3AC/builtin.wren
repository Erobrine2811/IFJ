import "ifj25" for Ifj

class Program {

    static main() {
        var s
        var t
        var n
        var a
        var b
        var i
        var j

        s = Ifj.read_str()
        t = Ifj.read_num()

        Ifj.write(s)
        Ifj.write(t)

        a = Ifj.floor(t)
        b = Ifj.str(a)
        Ifj.write(b)

        i = Ifj.length(s)
        Ifj.write(i)

        j = 0
        while (j < i) {
            var ch
            ch = Ifj.ord(s, j)
            Ifj.write(ch)
            j = j + 1
        }

        var sub
        sub = Ifj.substring(s, 0, i)
        Ifj.write(sub)

        var cmp
        cmp = Ifj.strcmp(s, sub)
        Ifj.write(cmp)

        var c
        c = Ifj.chr(65)
        Ifj.write(c)

        if (cmp == 0) {
            var x
            x = Ifj.read_num()
            var y
            y = Ifj.floor(x)
            Ifj.write(y)
        } else {
            var z
            z = Ifj.str(s)
            Ifj.write(z)
        }

        var k
        k = 0
        while (k < 3) {
            var p
            p = Ifj.read_str()
            var q
            q = Ifj.strcmp(p, s)
            Ifj.write(q)
            k = k + 1
        }
    }
}
