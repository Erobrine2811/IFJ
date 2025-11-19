import "ifj25" for Ifj

class Program {
    static main() {
      var a
      a = 123
      {
        a = "outer" // dynamicka zmena typu promenne
        var a
        a = "inner"
        __dummy = Ifj.write(a) // vypise "inner"
        // Nyni ale nelze znovu (redefinice):
        // var a
      }
      __dummy = Ifj.write(a) // vypise outer
    }
}
