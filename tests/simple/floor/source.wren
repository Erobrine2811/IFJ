import "ifj25" for Ifj

class Program {
    static main() {
      var f1
      f1 = 3.7
      var f2
      f2 = 3.0
      var f3
      f3 = -2.8

      var r1
      r1 = Ifj.floor(f1)
      Ifj.write(r1)
      Ifj.write("\n")

      var r2
      r2 = Ifj.floor(f2)
      Ifj.write(r2)
      Ifj.write("\n")

      var r3
      r3 = Ifj.floor(f3)
      Ifj.write(r3)
      Ifj.write("\n")
    }
}
