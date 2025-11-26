import "ifj25" for Ifj

class Program {
    static main() {
      var s1
      s1 = "hello"
      var l1
      l1 = Ifj.length(s1)
      Ifj.write(l1)
      Ifj.write("\n")

      var s2
      s2 = ""
      var l2
      l2 = Ifj.length(s2)
      Ifj.write(l2)
      Ifj.write("\n")
    }
}
