import "ifj25" for Ifj

class Program {
    static main() {
        var dummy
        dummy = printMessage()
    }

    static printMessage() {
        Ifj.write("Hello from function!\n")
        return null
    }
}

