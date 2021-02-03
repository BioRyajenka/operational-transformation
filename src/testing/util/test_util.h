//
// Created by Igor on 03.02.2021.
//

#ifndef OT_VARIATION_TEST_UTIL_H
#define OT_VARIATION_TEST_UTIL_H

#include "../../util.h"
#include "../../core/symbol.h"
#include "../../core/chain.h"
#include "../../client/document.h"
#include <initializer_list>

symbol create_symbol(const node_id_t &id, const int &val);

chain create_chain(std::initializer_list<node_id_t> ids);

bool check_doc_contents(const document &doc, std::initializer_list<node_id_t> expected);

#endif //OT_VARIATION_TEST_UTIL_H
