//=======================================================================
// Copyright (c) 2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#define CPM_PROPAGATE_TUPLE
#define CPM_BENCHMARK "Tests Benchmarks"
#include "cpm/cpm.hpp"

#include <thread>

constexpr const double factor = 1.1;

constexpr std::chrono::nanoseconds operator ""_ns(unsigned long long us){
    return std::chrono::nanoseconds(us);
}

struct test { std::size_t d; };
void randomize(test&){}

#define CPM_WARMUP 10
#define CPM_REPEAT 50

CPM_BENCH() {
    CPM_SIMPLE("simple_a", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 1_ns ); });
    CPM_SIMPLE("simple_b", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 2_ns ); });
}

CPM_DIRECT_BENCH_SIMPLE("simple_c", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 2_ns ); })

CPM_BENCH() {
    CPM_SIMPLE_P(
        NARY_POLICY(VALUES_POLICY(1,2,3,4,5,6), VALUES_POLICY(2,4,8,16,32,64)),
        "simple_a_n", [](auto d){ std::this_thread::sleep_for((factor * std::get<0>(d)) * 1_ns ); });
    CPM_SIMPLE_P(
        NARY_POLICY(VALUES_POLICY(1,2,3,4,5,6)),
        "simple_b_n",
        [](auto d){ std::this_thread::sleep_for((factor * std::get<0>(d)) * 2_ns ); });
    CPM_SIMPLE_P(
        VALUES_POLICY(1,2,3,4,5,6),
        "simple_c_n",
        [](auto d){ std::this_thread::sleep_for((factor * 3 * d) * 2_ns ); });
    CPM_SIMPLE_P( //Simply here for renaming
        VALUES_POLICY(1,2,3,4,5,6),
        "simple_c_n",
        [](auto d){ std::this_thread::sleep_for((factor * 3 * d) * 2_ns ); });
}

CPM_BENCH() {
    test a{3};
    test b{5};
    CPM_GLOBAL("global_a", [&a](std::size_t d){ std::this_thread::sleep_for((factor * d * a.d) * 1_ns ); }, a);
    CPM_GLOBAL("global_b", [&b](std::size_t d){ std::this_thread::sleep_for((factor * d * b.d) * 1_ns ); }, b);
}

CPM_BENCH() {
    CPM_TWO_PASS("2p_a",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * (d + d2.d)) * 1_ns ); }
        );

    CPM_TWO_PASS("2p_b",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 2 * (d + d2.d)) * 1_ns ); }
        );

    CPM_TWO_PASS_NS_P(
        NARY_POLICY(STD_STOP_POLICY, STD_STOP_POLICY),
        "2p_b_n",
        [](auto dd){ return std::make_tuple(test{std::get<0>(dd)}); },
        [](test& d2){ std::this_thread::sleep_for((factor * 2 * (d2.d + d2.d + d2.d)) * 1_ns ); }
        );
}

CPM_DIRECT_BENCH_TWO_PASS("2p_c",
    [](std::size_t d){ return std::make_tuple(test{d}); },
    [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); }
)

CPM_DIRECT_BENCH_TWO_PASS_NS_P(
    NARY_POLICY(VALUES_POLICY(1,2), VALUES_POLICY(3,4)),
    "2p_c_n",
    [](auto dd){ return std::make_tuple(test{std::get<0>(dd)}); },
    [](test& d2){ std::this_thread::sleep_for((factor * 2 * (d2.d + d2.d + d2.d)) * 1_ns ); }
)

CPM_SECTION("mmul")
    CPM_SIMPLE("std", [](std::size_t d){ std::this_thread::sleep_for((factor * d) * 9_ns ); });
    CPM_SIMPLE("fast", [](std::size_t d){ std::this_thread::sleep_for((factor * (d / 3)) * 1_ns ); });
    CPM_SIMPLE("common", [](std::size_t d){ std::this_thread::sleep_for((factor * (d / 2)) * 3_ns ); });
}

CPM_SECTION("conv")
    CPM_TWO_PASS("std",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 5 * (d + d2.d)) * 1_ns ); }
        );

    CPM_TWO_PASS("fast",
        [](std::size_t d){ return std::make_tuple(test{d}); },
        [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); }
        );
}

CPM_DIRECT_SECTION_TWO_PASS("conv2",
    CPM_SECTION_INIT([](std::size_t d){ return std::make_tuple(test{d}); }),
    CPM_SECTION_FUNCTOR("std", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 5 * (d + d2.d)) * 1_ns ); }),
    CPM_SECTION_FUNCTOR("fast", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); })
    )

CPM_DIRECT_SECTION_TWO_PASS_P("conv3",
    VALUES_POLICY(1,2,3,4,5,6,7,8,9,10),
    CPM_SECTION_INIT([](std::size_t d){ return std::make_tuple(test{d}); }),
    CPM_SECTION_FUNCTOR("std", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 5 * (d + d2.d)) * 1_ns ); }),
    CPM_SECTION_FUNCTOR("fast", [](std::size_t d, test& d2){ std::this_thread::sleep_for((factor * 3 * (d + d2.d)) * 1_ns ); })
    )

CPM_SECTION_O("fft",11,51)
    test a{3};
    test b{5};
    CPM_GLOBAL("std", [&a](std::size_t d){ std::this_thread::sleep_for((factor * d * (d % a.d)) * 1_ns ); }, a);
    CPM_GLOBAL("mkl", [&b](std::size_t d){ std::this_thread::sleep_for((factor * d * (d % b.d)) * 1_ns ); }, b);
}

CPM_SECTION_PO("gevv", NARY_POLICY(STD_STOP_POLICY), 9, 49)
    test a{3};
    test b{5};
    CPM_GLOBAL("std", [&a](auto dd){ auto d = std::get<0>(dd); std::this_thread::sleep_for((factor * d * (d % a.d)) * 1_ns ); }, a);
    CPM_GLOBAL("mkl", [&b](auto dd){ auto d = std::get<0>(dd); std::this_thread::sleep_for((factor * d * (d % b.d)) * 1_ns ); }, b);
}

CPM_SECTION_P("gemm", NARY_POLICY(STD_STOP_POLICY, STD_STOP_POLICY))
    test a{3};
    test b{5};
    CPM_GLOBAL("std", [&a](auto dd){ auto d = std::get<0>(dd) + std::get<1>(dd); std::this_thread::sleep_for((factor * d * (d % a.d)) * 1_ns ); }, a);
    CPM_GLOBAL("mkl", [&b](auto dd){ auto d = std::get<0>(dd) + 2 * std::get<1>(dd); std::this_thread::sleep_for((factor * d * (d % b.d)) * 1_ns ); }, b);
}

CPM_SECTION_P("mmul", NARY_POLICY(STD_STOP_POLICY, VALUES_POLICY(1,2,3,4,5,6), VALUES_POLICY(2,4,8,16,32,64)))
    test a{3};
    test b{5};
    CPM_GLOBAL("std", [&a](auto dd){ auto d = std::get<0>(dd) + 2 * std::get<1>(dd) + 4 * std::get<2>(dd); std::this_thread::sleep_for(factor * d * 1_ns ); }, a);
    CPM_GLOBAL("mkl", [&a](auto dd){ auto d = std::get<0>(dd) + std::get<1>(dd) + std::get<2>(dd); std::this_thread::sleep_for(factor * d * 1_ns ); }, b);
    CPM_GLOBAL("bla", [&a](auto dd){ auto d = std::get<0>(dd) + 2 * std::get<1>(dd) + 2 * std::get<2>(dd); std::this_thread::sleep_for(factor * d * 1_ns ); }, a, b);
}
