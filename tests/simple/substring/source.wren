import "ifj25" for Ifj

class Program {
    static main() {
      var s
      s = "abcdef"
      
      var sub1
      sub1 = Ifj.substring(s, 1, 4)
      Ifj.write(sub1)
      Ifj.write("\n")

      var sub2
      sub2 = Ifj.substring(s, 0, 6)
      Ifj.write(sub2)
      Ifj.write("\n")

      var sub3
      sub3 = Ifj.substring(s, 3, 3)
      Ifj.write(sub3)
      Ifj.write("\n")

      var sub4
      sub4 = Ifj.substring(s, -1, 3)
      Ifj.write(sub4)
      Ifj.write("\n")

      var sub5
      sub5 = Ifj.substring(s, 0, 7)
      Ifj.write(sub5)
      Ifj.write("\n")
    }
}
