#include <iostream>
#include <list>
#include <functional>
#include <vector>
#include <tuple>
#include <set>

template <typename A, typename B>
std::list<B> operator>>=(std::list<A> const& in_list, std::function<std::list<B>(A)> f) {
    std::list<B> new_list;

    for (const auto item : in_list) {
        const auto list_of_transformed_items = f(item);

        // Flatten the list
        for (const auto transformed_item : list_of_transformed_items) {
            new_list.push_back(transformed_item);
        }
    }

    return new_list;
}

int main(const int argc, char const *argv[]) {
    std::function<std::list<int>(int)> add_one_and_two = [] (int n) {
        std::list<int> new_list;

        new_list.push_back(n + 1);
        new_list.push_back(n + 2);

        return new_list;
    };

    const std::list<int> fibbos = { 1, 1, 2, 3, 5, 8 };

    const auto transformed_fibbos = fibbos >>= add_one_and_two;

    std::cout << "Fibonacci numbers, pre flatmap:\n";
    for (const auto fibbi : fibbos) {
        std::cout << fibbi << '\n';
    }
    std::cout << "Fibonacci numbers, post flatmap:\n";
    for (const auto fibbi : transformed_fibbos) {
        std::cout << fibbi << '\n';
    }

    // -------------------------- Gridworld example ---------------------------

    enum tiles {
        empty = 0,
        blocked = 1,
        good_end = 2,
        bad_end = 3
    };

    const std::vector<std::vector<tiles>> gridworld = {
        { empty,   empty, empty, good_end },
        { empty, blocked, empty,  bad_end },
        { empty,   empty, empty,    empty }
    };

    const std::list<std::tuple<int, int>> start_coords = {
        std::tuple<int, int> {2, 0}
    };

    // Repeatedly applying this is essentially flood-fill/breadth first search
    std::function<std::list<std::tuple<int, int>>(std::tuple<int, int>)>
    get_neighbour_coords = [gridworld] (std::tuple<int, int> coord) {
        std::list<std::tuple<int, int>> surrounding_coords;

        const int upRow = std::get<0>(coord) - 1;
        if ((upRow >= 0) && (gridworld[upRow][std::get<1>(coord)] != blocked)) {
            surrounding_coords.push_back(std::tuple<int, int> {upRow, std::get<1>(coord)});
        }

        const int rightColumn = std::get<1>(coord) + 1;
        if ((rightColumn < gridworld[std::get<0>(coord)].size()) && (gridworld[std::get<0>(coord)][rightColumn] != blocked)) {
            surrounding_coords.push_back(std::tuple<int, int> {std::get<0>(coord), rightColumn});
        }

        const int downRow = std::get<0>(coord) + 1;
        if ((downRow < gridworld.size()) && (gridworld[downRow][std::get<1>(coord)] != blocked)) {
            surrounding_coords.push_back(std::tuple<int, int> {downRow, std::get<1>(coord)});
        }

        const int leftColumn = std::get<1>(coord) - 1;
        if ((leftColumn >= 0) && (gridworld[std::get<0>(coord)][leftColumn] != blocked)) {
            surrounding_coords.push_back(std::tuple<int, int> {std::get<0>(coord), leftColumn});
        }

        return surrounding_coords;
    };


    const auto next_moves = start_coords >>= get_neighbour_coords;

    std::cout << "next_moves:" << '\n';
    for (const auto &coord : next_moves) {
        std::cout << "(row, column): (" << std::get<0>(coord) << ", " << std::get<1>(coord) << ")\n";
    }


    const auto after_two_turns = (start_coords >>= get_neighbour_coords) >>= get_neighbour_coords;
    // Note the duplicates that will appear from different combinations of moves leading to the same square
    std::cout << "after_two_turns:" << '\n';
    for (const auto &coord : after_two_turns) {
        std::cout << "(row, column): (" << std::get<0>(coord) << ", " << std::get<1>(coord) << ")\n";
    }

    return 0;
}
