import "ifj25" for Ifj

class Program {
    static main() {
      var r1
      r1 = Ifj.strcmp("a", "b")
      Ifj.write(r1)
      Ifj.write("\n")

      var r2
      r2 = Ifj.strcmp("b", "a")
      Ifj.write(r2)
      Ifj.write("\n")

      var r3
      r3 = Ifj.strcmp("a", "a")
      Ifj.write(r3)
      Ifj.write("\n")
    }
}
