// Program: nedeklarovaná funkcia
import "ifj25" for Ifj
class Program {

    static main() {
        var x
        x = mystery();  // táto funkcia nikdy nebude definovaná
        Ifj.write(x)
    }

    static foo() {
        return 42
    }
}
