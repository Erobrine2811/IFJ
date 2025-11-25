import "ifj25" for Ifj

class Program {
    static main() {
      for (index in 10..20) {

        print(index)

        if (is_even(index) == 1) {
          var i = 0
          print("Printing even 3 times:")
          while (i < 3) {
            print("  even")
            i = i + 1
          }
        } else {
          print("odd")
        }
      }
    }

    static is_even(n) {
      var res = n / 2
      var floored = Ifj.floor(res)
      if (res == floored) {
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
