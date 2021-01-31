//
// Created by Igor on 31.01.2021.
//

#ifndef OT_VARIATION_DOCUMENT_H
#define OT_VARIATION_DOCUMENT_H

#include "../core/node.h"
#include "../core/symbol.h"
#include "../testing/magic_list.h"
#include "../core/chain.h"
#include "../core/operation.h"

class document {
public:
    document() = default;

    virtual void apply(const operation &op) = 0;
};

#endif //OT_VARIATION_DOCUMENT_H
