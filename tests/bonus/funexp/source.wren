import "ifj25" for Ifj

class Program {
    static main() {
      var a = 5 * get_ten() + 2
      print(a)

      a = func_in_return() + 3
      print(a) // should print 17

      if (get_ten() == 10) {
        print("get_ten works")
      } else {
        print("get_ten does not work")
      }
    }

    static get_ten() {
      return 10
    }

    static func_in_return() {
      return 4 + get_ten()
    }

    static print(a) {
      Ifj.write(a)
      Ifj.write("\n")
    }
}
