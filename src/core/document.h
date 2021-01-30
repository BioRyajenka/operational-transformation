//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_DOCUMENT_H
#define OT_VARIATION_DOCUMENT_H


#include "operation.h"

class document {


public:
    document(const document &rhs);

    void apply(const operation &op);
    void apply(const int &pos, const change &change);

    int hash() const;
    int size() const;
};


#endif //OT_VARIATION_DOCUMENT_H
