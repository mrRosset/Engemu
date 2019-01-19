#include <random>
#include <iostream>	

#include "../Common.h"
#include "../CPU/Tharm/Utils.h"

#include <catch/catch.hpp>

TEST_CASE("Utils PSR u32", "[Tharm]") {
	std::random_device rd;	
	std::mt19937 gen(rd());
	
	for (int i = 0; i < 100; i++) {
		u32 u = gen();
		PSR p;
		u32_to_PSR(u, p);
		u32 r = PSR_to_u32(p);
		REQUIRE(u == r);
	}
}


