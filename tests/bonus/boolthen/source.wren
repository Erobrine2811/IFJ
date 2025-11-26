
import "ifj25" for Ifj

class Program {
    static main() {
      var read_bool = Ifj.read_bool()
      print(read_bool) // should print true or false
      var bool_as_num = boolean_checker(read_bool)
      print(bool_as_num) // should print 1 if true else 0

      if (read_bool && true) {
        print("It's true!")
      } else {
        print("It's false!")
      }
    }

    static boolean_checker(b) {
      if (b) {
        return 1
      } else {
        return 0
      }
    }

    static print(a) {
      Ifj.write(a)
      Ifj.write("\n")
    }
}
