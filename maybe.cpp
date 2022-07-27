#include <iostream>
#include <functional>
#include <math.h>
#include <map>


template <typename A> class Maybe {
    private:
        A value;
        bool is_empty;

        Maybe() : value(), is_empty(true) {}
        Maybe(A value) : value(value), is_empty(false) {}
    public:
        // wrap/return/unit
        static Maybe<A> Just(A value) { return Maybe(value); }
        static Maybe<A> Nothing()     { return Maybe(); }

        template <typename B>
        static Maybe<B> bind(Maybe<A> const& a, std::function<Maybe<B>(A)> f) {
            return a.is_empty ? Maybe<B>::Nothing() : f(a.value);
        }

        // bind/flatmap
        // As an operator
        template <typename B>
        Maybe<B> operator>>=(std::function<Maybe<B>(A)> f) const {
            return is_empty ? Maybe<B>::Nothing() : f(value);
        }

        // Overloading << operator for ease of printing
        friend std::ostream& operator<<(std::ostream &out, Maybe<A> const& maybe) {
            out << (maybe.is_empty ? "Nothing" : "Just ");

            if (!maybe.is_empty) {
                out << maybe.value;
            }

            return out;
        }
};

int main(const int argc, char const *argv[]) {
    std::function<Maybe<float>(float)> safe_log = [] (float n) {
        return n == 0 ? Maybe<float>::Nothing() : Maybe<float>::Just(log(n));
    };
    std::function<Maybe<float>(float)> reciporical = [] (float n) {
        return n == 0 ? Maybe<float>::Nothing() : Maybe<float>::Just(1 / n);
    };

    const auto something = Maybe<float>::Just(2);
    // Using the bind() function
    auto result = Maybe<float>::bind(Maybe<float>::bind(something, safe_log), reciporical);
    // Using the >>= operator
    result = (something >>= safe_log) >>= reciporical;
    std::cout << "'something' is " << something << '\n';
    std::cout << "1 / ln(something) = " << result << '\n';

    auto nothing = Maybe<float>::Nothing();
    const auto nothing_result = (nothing >>= safe_log) >>= reciporical;
    std::cout << "'nothing' is " << nothing << '\n';
    std::cout << "1 / ln(nothing) = " << nothing_result << '\n';

    // ln(1) = 0
    const auto erroneous = Maybe<float>::Just(1);
    const auto erroneous_result = (erroneous >>= safe_log) >>= reciporical;
    std::cout << "'erroneous_result' 1 / ln(1) is " << erroneous_result << '\n';


    // Mock requests to web servers
    std::function<Maybe<int>(int)> get_user_wallet_id = [] (int user_id) {
        std::map<int, int> database { {0xC001D00D, 0x63617368} };

        const auto query_result = database.find(user_id);

        return query_result != database.end() ? Maybe<int>::Just(query_result->second) : Maybe<int>::Nothing();
    };

    std::function<Maybe<float>(int)> get_wallet_balance = [] (int wallet_id) {
        std::map<int, float> database { {0x63617368, 123.45} };

        const auto query_result = database.find(wallet_id);

        return query_result != database.end() ? Maybe<float>::Just(query_result->second) : Maybe<float>::Nothing();
    };

    // Users should not be able to purchase if they don't have enough in their wallet
    std::function<Maybe<float>(float)> withdraw_from_wallet = [] (float balance) {
        const auto new_balance = balance - 5;
        return new_balance >= 0 ? Maybe<float>::Just(new_balance) : Maybe<float>::Nothing();
    };

    const auto existing_users_balance = (((Maybe<int>::Just(0xC001D00D) >>= get_user_wallet_id)
                                                                        >>= get_wallet_balance)
                                                                        >>= withdraw_from_wallet);
    std::cout << "Existing user's balance after purchase: " << existing_users_balance << '\n';

    const auto nonexistent = (((Maybe<int>::Just(0x1BADD00D) >>= get_user_wallet_id)
                                                             >>= get_wallet_balance)
                                                             >>= withdraw_from_wallet);
    std::cout << "Non-existent user's balance after purchase: " << nonexistent << '\n';

    return 0;
}
