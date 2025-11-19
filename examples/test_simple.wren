import "ifj25" for Ifj

class Program {
    static main() {
        var a = 1
        var b = 2
        var c = printAndAdd(a, b)
        Ifj.write(c)
        Ifj.write("\n")
    }

    static printAndAdd(a, b){
      Ifj.write(a)
      Ifj.write(" + ")
      Ifj.write(b)
      Ifj.write("\n")
      return a + b
    }
}
