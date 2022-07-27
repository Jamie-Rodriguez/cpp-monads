#include <iostream>
#include <functional>
#include <string>


// This is a hack to create a "void type" that we can specify as a template
// value.
// Otherwise calling the default contructor with IO<void>() will attempt to set
// the type of IO.value as type "void"
// See functions greet_user() and show_better_number()
class MyVoid {
        // Overloading << operator for ease of printing
        friend std::ostream& operator<<(std::ostream &out, MyVoid const& v) {
            out << "";

            return out;
        }
};

template <typename A> class IO {
    private:
        A value;

    public:
        // wrap/return/unit
        IO() {}
        IO(A value) : value(value) {}

        // bind/flatmap
        // As an operator
        template <typename B>
        IO<B> operator>>=(std::function<IO<B>(A)> f) const {
            return f(value);
        }

        // Overloading << operator for ease of printing
        friend std::ostream& operator<<(std::ostream &out, IO<A> const& io) {
            out << "IO (" << io.value << ")";

            return out;
        }
};

int main(const int argc, char const *argv[]) {
    std::function<IO<std::string>(void)> get_user_name = [] (void) {
        std::string user_input;

        std::cout << "Hello, what is your name?\n";
        std::cin >> user_input;

        return user_input;
    };

    std::function<IO<MyVoid>(std::string)> greet_user = [] (std::string user) {
        std::cout << "Pleased to meet you, " << user << "!\n";

        return IO<MyVoid>();
    };

    get_user_name() >>= greet_user;

    //  ----------------------- More intricate example ------------------------

    // Obviously, this is a very unsafe function with no sanitisation!
    std::function<IO<int>(void)> get_favourite_number = [] (void) {
        int user_input;

        std::cout << "What is your favourite number?\n";
        std::cin >> user_input;

        return IO<int>(user_input);
    };

    // *gasp* a pure function??
    std::function<int(int)> better_number = [] (int n) {
        return n + 1;
    };

    std::function<IO<MyVoid>(int, int)> show_better_number = [] (int n, int better) {
        std::cout << "Personally, I think " << better << " is a better number than " << n << '\n';

        return IO<MyVoid>();
    };

    /*
      This is clunky because C++'s type-system was not designed with this in
      mind :P

      If we could remove the superfluous type information and lambda captures,
      it would look like:

        get_favourite_number() >>=
            ((int n) {
                return IO<int>(better_number(n)) >>=
                    ((int better) {
                        return show_better_number(n, better);
                    });
            });

      Functional languages like Haskell provide syntactic sugar called
      "do" notation that is then transformed into the above bindings. If it
      were implemented in C++ it, might look something similar to:

        do {
            IO<int> n = get_favourite_number();
            IO<int> better = IO<int>(better_number(n));
            show_better_number(n, better);
        };
    */
    // Have to provide the lambdas with explicit compile-time typing using
    // std::function
    const auto io = get_favourite_number() >>= std::function<IO<MyVoid>(int)> ([&]
                        (int n) {
                            // Because better_number() is a pure function
                            // i.e. not wrapped in a monad, it has no
                            // associated bind() function.
                            // It needs to be wrapped before sending it's
                            // result further down the pipeline:
                            return IO<int>(better_number(n)) >>= std::function<IO<MyVoid>(int)> ([&]
                                (int better) {
                                    return show_better_number(n, better);
                                });
                        });

    std::cout << "End value of computation is " << io << '\n';

    return 0;
}
