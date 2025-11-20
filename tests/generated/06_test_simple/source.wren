import "ifj25" for Ifj

class Program {
    static simpleGetter {
      if (__a == 2) {
        return 2
      }
      else {
        return 1
      }
    }

    static simpleSetter=(val) {
        Ifj.write("Jsem jednorozci setter, ziskal jsem ")
        Ifj.write(val)
        Ifj.write("\n")
        __a = val
    }

    static main() {
        __a = 2
        simpleSetter = 5
    }

}
