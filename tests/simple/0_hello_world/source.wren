import "ifj25" for Ifj

class Program {
    static main() {
      text_fun_with_return()
    }

    static text_fun_without_return() {
      var str = "This is a text function without return.\n"
      Ifj.write(str)
    }
}
