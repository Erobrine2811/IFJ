import "ifj25" for Ifj

class Program {
    static main() {
      var num
      num = Ifj.read_num()
      var str_num
      str_num = Ifj.str(num)
      Ifj.write(str_num)
      Ifj.write("\n")
    }
}
