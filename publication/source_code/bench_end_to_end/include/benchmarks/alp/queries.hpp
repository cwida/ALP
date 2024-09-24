#pragma once
#include "benchmarks/alp/config.hpp"
#include "benchmarks/tpch/Queries.hpp"

class alp_q_builder : public vectorwise::QueryBuilder {
public:
	struct query {
		double                                aggregator = 0;
		std::unique_ptr<vectorwise::Operator> root_op;
	}; //
public:
	alp_q_builder(runtime::Database& db, vectorwise::SharedStateManager& shared, size_t size)
	    : QueryBuilder(db, shared, size) {} //
public:
	std::unique_ptr<query> get_q();
};

runtime::Relation alp_q(runtime::Database& db, size_t t_c, size_t vec_sz);