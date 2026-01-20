#include <iostream>
#include <vector>
#include <concepts>
#include <iterator>
#include <string>

using namespace std;

template<typename C>
concept Iterable = requires (C c) {
    std::begin(c);
    std::end(c);
};

template<typename T>
concept Addable = requires (T a, T b) {
    {a + b} -> std::same_as<T>;
};

template<typename T>
concept Divisible = requires (T a , std::size_t n) {
    {a / n} -> std::convertible_to<T>;
};

template<typename T>
concept Comparable =
    is_arithmetic_v<T> &&
    !std::is_same_v<T, char> &&
    !std::is_same_v<T, signed char> &&
    !std::is_same_v<T, unsigned char>;

namespace core_numeric {

    template<Iterable T>
    requires Addable<typename T::value_type>
    auto sum(const T& container) {
        using Q = typename T::value_type;

        Q result{};

        for (const auto &elem : container)
            result += elem;

        return result;
    }

    template<Iterable T>
    requires Divisible<typename T::value_type>
    auto mean(const T& container) {
        using Q = typename T::value_type;

        if constexpr (std::is_integral_v<Q>) {
            return sum(container) / static_cast<Q>(container.size());
        } else {
            double s = 0.0;
            for (const auto& x : container) s += static_cast<double>(x);
            return s / static_cast<double>(container.size());
        }
    }

    template<Iterable T>
    requires Addable<typename T::value_type>
    auto variance(const T& container) {
        using Q = typename T::value_type;
        if constexpr (std::is_integral_v<Q>) {
            double mu = static_cast<double>(sum(container)) / container.size();

            double acc = 0.0;
            for (const auto& x : container) {
                double d = static_cast<double>(x) - mu;
                acc += d * d;
            }
            return acc / static_cast<double>(container.size());
        } else {
            double mu = 0.0;
            for (const auto& x : container) mu += static_cast<double>(x);
            mu /= static_cast<double>(container.size());

            double acc = 0.0;
            for (const auto& x : container) {
                double d = static_cast<double>(x) - mu;
                acc += d * d;
            }
            return acc / static_cast<double>(container.size());
        }
    }

    template<Iterable T>
    requires Comparable<typename T::value_type>
    auto max(const T& container) {
        using Q = typename T::value_type;
        Q result = container[0];

        for (const auto &elem : container) {
            if (elem > result)
                result = elem;
        }

        return result;
    }

    template <Iterable T, typename F>
    auto transform_reduce(const T& container, F func) {
        using R = decltype(func(*container.begin()));

        R result{};

        for (const auto& x : container)
            result += func(x);

        return result;
    }

    template<Comparable... Ts>
    auto sum_variadic(Ts... xs) {
        return (xs + ...);
    }

    template<Comparable... Ts>
    double mean_variadic(Ts... xs) {
        constexpr std::size_t n = sizeof...(xs);
        return (static_cast<double>(xs) + ...) / n;
    }

    template<Comparable... Ts>
    double variance_variadic(Ts... xs) {
        constexpr std::size_t n = sizeof...(xs);
        double mean = (static_cast<double>(xs) + ...) / n;

        return (( (static_cast<double>(xs) - mean) *
                  (static_cast<double>(xs) - mean) ) + ...) / n;
    }

    template<Comparable T, Comparable... Ts>
    auto max_variadic(T first, Ts... rest) {
        auto max_val = first;
        ((max_val = max_val > rest ? max_val : rest), ...);
        return max_val;
    }
}

void testSum() {

    cout << "Testing sum:" << endl;

    vector<int> v1{1,2,3,4};
    cout << core_numeric::sum(v1) << endl;
    // Resultado esperado: 10

    vector<double> v2{1.5, 2.0, 0.5};
    cout << core_numeric::sum(v2) << endl;
    // Resultado esperado: 4.0

    // NO EJECUTA
    // Falla concept Addable<value_type> (no existe a + b del mismo tipo)
    // struct X{};
    // vector<X> vx(3);
    // cout << core_numeric::sum(vx) << endl;
}

void testMean() {

    cout << "--------------------------------------------" << endl;
    cout << "Testing mean:" << endl;

    vector<int> v1{1,2,3,4};
    cout << core_numeric::mean(v1) << endl;
    // Resultado esperado: 2

    vector<double> v2{1,2,3,4};
    cout << core_numeric::mean(v2) << endl;
    // Resultado esperado: 2.5

    // NO EJECUTA
    // Falla concept Divisible<std::string> (no existe string / size_t)
    // vector<string> vs{"a","bb"};
    // cout << core_numeric::mean(vs) << endl;

}

void testVariance() {

    cout << "--------------------------------------------" << endl;
    cout << "Testing variance:" << endl;

    vector<int> v1{1,2,3,4};
    cout << core_numeric::variance(v1) << endl;
    // Resultado esperado: 1.25

    vector<double> v2{1,2,3,4};
    cout << core_numeric::variance(v2) << endl;
    // Resultado esperado: 1.25

    // NO EJECUTA
    // Termina fallando en mean(vs): falla Divisible<std::string> (no existe / size_t)
    // vector<string> vs{"a","bb"};
    // cout << core_numeric::variance(vs) << endl;
}

void testMax() {

    cout << "--------------------------------------------" << endl;
    cout << "Testing max:" << endl;

    vector<int> v1{3, 9, 2, 7};
    cout << core_numeric::max(v1) << endl;
    // Resultado esperado: 9

    vector<double> v2{1.2, 4.8, 3.1};
    cout << core_numeric::max(v2) << endl;
    // Resultado esperado: 4.8

    // NO EJECUTA
    // Falla concept Comparable<string> (si Comparable es "numerico" y excluye string)
    // vector<string> vs{"a","zz"};
    // cout << core_numeric::max(vs) << endl;

}

void testTransformReduce() {

    cout << "--------------------------------------------" << endl;
    cout << "Testing transform_reduce:" << endl;

    vector<double> v{1,2,3};
    auto r = core_numeric::transform_reduce(v, [](double x){ return x*x; });
    cout << r << endl;
    // Resultado esperado: 14  (1^2 + 2^2 + 3^2)

    vector<int> w{1,2,3};
    auto r2 = core_numeric::transform_reduce(w, [](int x){ return x + 10; });
    cout << r2 << endl;
    // Resultado esperado: 36  (11 + 12 + 13)

    // NO EJECUTA
    // Falla porque el tipo de retorno no soporta "result += ..."
    // struct X { int v; };
    // auto bad = core_numeric::transform_reduce(w, [](int x){ return X{x}; });
}

void testSumVariadic() {

    cout << "--------------------------------------------" << endl;
    cout << "Testing sum_variadic:" << endl;

    cout << core_numeric::sum_variadic(1,2,33,4) << endl;
    // Resultado esperado: 40

    cout << core_numeric::sum_variadic(0.5, 1, 2.5) << endl;
    // Resultado esperado: 4

    // NO EJECUTA
    // Falla concept Comparable<string>
    // cout << core_numeric::sum_variadic(string("a"), string("b")) << endl;

    // NO EJECUTA
    // Falla concept Comparable<char>
    // cout << core_numeric::sum_variadic('a','b') << endl;
}

void testMeanVariadic() {

    cout << "--------------------------------------------" << endl;
    cout << "Testing mean_variadic:" << endl;

    cout << core_numeric::mean_variadic(0.1,2,3,4) << endl;
    // Resultado esperado: 2.275

    cout << core_numeric::mean_variadic(1,2,3,4) << endl;
    // Resultado esperado: 2.5

    // NO EJECUTA
    // Falla concept Comparable<string>
    // cout << core_numeric::mean_variadic(string("a"), string("b")) << endl;

}

void testVarianceVariadic() {

    cout << "--------------------------------------------" << endl;
    cout << "Testing variance_variadic:" << endl;

    std::cout << core_numeric::variance_variadic(1,2,3,4) << endl;
    // Resultado esperado: 1.25

    cout << core_numeric::variance_variadic(0.1,2,3,4) << endl;
    // Resultado esperado: 2.07

    // NO EJECUTA
    // Falla Comparable<char>
    // cout << core_numeric::variance_variadic('a','b','c') << endl;
}

void testMaxVariadic() {

    cout << "--------------------------------------------" << endl;
    cout << "Testing max_variadic:" << endl;

    cout << core_numeric::max_variadic(1,2.7,3,4) << endl;
    // Resultado esperado: 4

    cout << core_numeric::max_variadic(1,2,33,4) << endl;
    // Resultado esperado: 33

    // NO EJECUTA
    // Falla concept Comparable<string>
    // cout << core_numeric::max_variadic(string("a"), string("b")) << endl;
}

int main() {
    testSum();
    testMean();
    testVariance();
    testMax();
    testTransformReduce();
    testSumVariadic();
    testMeanVariadic();
    testVarianceVariadic();
    testMaxVariadic();

    return 0;
}