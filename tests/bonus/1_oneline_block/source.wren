import "ifj25" for Ifj

class Program {
    static main() {
      var x = Ifj.read_num()
      print(x) // should print whatever was input
      if (x > 5) {
        var x = x * 2
        print(x) // should print 20
        x = 42
        print(x) // should print 42
      } else {
        print(x) // should print whatever was input
      }
      print(x) // should print whatever was input
    }

    static print(a) {
      Ifj.write(a)
      Ifj.write("\n")
    }
}
