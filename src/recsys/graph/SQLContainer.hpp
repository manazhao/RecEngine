/*
 * SQLContainer.hpp
 *
 *  Created on: May 12, 2014
 *      Author: manazhao
 */

#ifndef SQLCONTAINER_HPP_
#define SQLCONTAINER_HPP_
#include "Basic.hpp"
#include "recsys/data/SQL.h"

namespace recsys {
namespace graph {

class SQLEntityContainer {
public:
	struct SharedData {
		prepared_statement_ptr m_insertStmtPtr;
		prepared_statement_ptr m_queryByIdStmtPtr;
		prepared_statement_ptr m_updateStmtPtr;
	};

public:
	template<typename EntityValueType>
	shared_ent_ptr index(Entity::ent_idx_type const& idx,
			Entity::ent_type const& type, EntityValueType const& value =
					EntityValueType());
	shared_ent_ptr exist(Entity::ent_idx_type const& idx) const;
	void
			get_type_idx_set(Entity::ent_type const& type, ent_idx_set& idxSet) const;
	type_idx_set_map const& get_idx_set_map() const;
public:
	static void init_shared();
public:
	static SharedData SHARED_DATA;

};

}
}

#endif /* SQLCONTAINER_HPP_ */
