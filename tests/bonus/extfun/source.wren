import "ifj25" for Ifj

class Program {
    static main() {
      var str = "Hello world!\n"
      Ifj.write(str)
      test_without_return()
    }

    static test_without_return() {
      Ifj.write("This is a text function without return.\n")
    }
}
