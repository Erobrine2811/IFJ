import "ifj25" for Ifj

class Program {
    static main() {
        var result
        result = add(5, 3)
        var floored
        floored = Ifj.floor(result)
        var resultStr
        resultStr = Ifj.str(floored)
        Ifj.write(resultStr)
        Ifj.write("\n")
    }

    static add(a, b) {
        return
    }
}
