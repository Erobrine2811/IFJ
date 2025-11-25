import "ifj25" for Ifj

class Program {
    static myValue {
        if (__my_hidden_value == null) {
            return "null"
        } else {
            return __my_hidden_value
        }
    }

    static myValue=(val) {
        __my_hidden_value = val * 2
    }

    static main() {
        var val
        val = myValue
        Ifj.write(val) 
        Ifj.write("\n")

        myValue = 10 
        
        val = myValue 
        var valStr
        valStr = Ifj.str(val)
        Ifj.write("Value is: ")
        Ifj.write(valStr)
        Ifj.write("\n")
    }
}
