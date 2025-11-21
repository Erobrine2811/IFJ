import "ifj25" for Ifj

class Program {
    static main() {
      var a = 5 * get_ten() + 2 // should be 5 * 10 + 2 = 52
      print(a) // should print 52

      a = func_in_return() + 3 // should be 4 + 10 + 3 = 17
      print(a) // should print 17

      if (get_ten() == 10) {
        print("get_ten works") // should print this
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
