import "ifj25" for Ifj

class Program {
    static main() {
      var x = 5
      var res
      Ifj.write("Regular function call result: ")
      res = func1(x)
      Ifj.write(res)
      Ifj.write("\n")

      var res2
      Ifj.write("Oneline function call result: ")
      res2 = func2(x)
      Ifj.write(res2)
      Ifj.write("\n")

      if (res == res2) {
        Ifj.write("Both functions returned the same result.\n")
      } else {
        Ifj.write("Functions returned different results.\n")
      }
    }

    static func1(x) {
      return x + 1
    }

    static func2(x) { x + 1 }
}
