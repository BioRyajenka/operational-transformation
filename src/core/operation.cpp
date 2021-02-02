//
// Created by Igor on 30.01.2021.
//

#include "operation.h"

operation::operation(const node_id_t &node_id, const chain &ch) {
    // TODO: по сути тут копирование идет. как бы совсем не копировать?
    insertions[node_id] = chain(ch.get_root());
}
