import "ifj25" for Ifj

class Program {
    static main() {
      var a = Ifj.read_num()

      var b = a > 0 ? 10 : 20
      print(b) // should print 10 if a > 0 else 20
    }

    static print(a) {
      Ifj.write(a)
      Ifj.write("\n")
    }
}
