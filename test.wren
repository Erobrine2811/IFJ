import "ifj25" for Ifj
class Program {
    static main() {
        var arg = 10
        if (arg is Num) {
            Ifj.write("Celkem velke cislo!\n")
        } else {
            Ifj.write("Neplatny argument\n")
        }
    }
}