import "ifj25" for Ifj

class Program {
    static main() {
      var n
      n = 123
      var f
      f = 45.67
      var s
      s = "hello"
      var nil
      nil = null

      var ns
      ns = Ifj.str(n)
      Ifj.write(ns)
      Ifj.write("\n")

      var fs
      fs = Ifj.str(f)
      Ifj.write(fs)
      Ifj.write("\n")

      var ss
      ss = Ifj.str(s)
      Ifj.write(ss)
      Ifj.write("\n")

      var nils
      nils = Ifj.str(nil)
      Ifj.write(nils)
      Ifj.write("\n")
    }
}
