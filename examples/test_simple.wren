import "ifj25" for Ifj

class Program {
    // staticky getter -> vraci hodnotu
    static unicorn {
        // __a je globalni promenna. Pokud neni zatim definovana, implicitne je hodnota null.
        // null se v podmince chova jako false.
        if (__a) {
            return __a + 10
        } else {
            return null
        }
    }

    // staticky setter -> chova se jako funkce, muze mit vedlejsi efekty, ale pristupuje se k ni jinak (viz nize)
    static unicorn=(val) {  
        Ifj.write("Jsem jednorozci setter, ziskal jsem ")
        Ifj.write(val)
        Ifj.write("\n")
        __a = val
    }

    // jina funkce se muze jmenovat stejne jako getter, nema s nim nic spolecneho!
    static unicorn() {
        Ifj.write("Jsem ve funkci unicorn, ne v getteru\n")
    }

    static main() {
        Ifj.write(unicorn) /* Staticky getter muze byt parametr ve volani funkce.
                            * Tohle tedy prvne zavola a vyhodnoti telo getteru unicorn,
                            * vysledek to pak pouzije jako parametr pro zavolani Ifj.write.
                            * Toto volani skonci vypisem "null". */
        Ifj.write("\n")

        unicorn = 5 /* Zavola telo setteru -> 
                     * ten vypise zpravu "Jsem jednorozci setter, ziskal jsem 5\n" 
                     * a nastavi glob. promennou __a na 5.
                     * Mimochodem /* viceradkove komentare mohou byt /* vnorene */, hura. */ */

        var myValue         // definice promenne - v zakladnim zadani neni s prirazenim (var myValue = ...), 
                            // vychozi hodnota je null
        myValue = unicorn   // zavola telo getteru, ten vrati 15 (== __a + 10)
        Ifj.write(myValue)  // vypise 15
        Ifj.write("\n")

        myValue = unicorn()  // zavola funkci, ta vypise "Jsem ve funkci unicorn, ne v getteru\n"
        Ifj.write(myValue)   // vypise null (funkce neurcila navratovou hodnotu, implicitne null)
        Ifj.write("\n")
    }
}
