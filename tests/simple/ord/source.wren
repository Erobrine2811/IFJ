import "ifj25" for Ifj

class Program {
    static main() {
      var s
      s = "hello"
      var r1
      r1 = Ifj.ord(s, 1)
      Ifj.write(r1)
      Ifj.write("\n")

      var r2
      r2 = Ifj.ord(s, 4)
      Ifj.write(r2)
      Ifj.write("\n")

      var r3
      r3 = Ifj.ord(s, 5)
      Ifj.write(r3)
      Ifj.write("\n")
    }
}
