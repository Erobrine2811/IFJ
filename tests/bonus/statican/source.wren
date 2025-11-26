import "ifj25" for Ifj

class Program {
    static main() {
      var x = Ifj.read_num()

      var y

      if (x >= 0) {
        y = 3

      } else {
        y = "a"
      }

      var z = y * x
    }
}
