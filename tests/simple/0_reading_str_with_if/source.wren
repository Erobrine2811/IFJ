import "ifj25" for Ifj

class Program {
    static main() {
      Ifj.write("Type 'y' to pass the test: \n")
      var str = Ifj.read_str()
      if (str == "y") {
        Ifj.write("OK\n")
      } else {
        Ifj.write("FAIL\n")
      }
    }
}
