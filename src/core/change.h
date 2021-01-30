//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_CHANGE_H
#define OT_VARIATION_CHANGE_H

enum change_type { insert, update, del };

class change {
public:
    change_type type;
    int value;

    change(change_type type, int value) : type(type), value(value) {}
};

/*class insert : public change {
public:
    int value;

    insert(int value) : change(insert), value(value) {}
};

class update : public change {
public:
    int value;

    update(int value) : value(value) {}
};

class del : public change {};

//static change del = change();*/

#endif //OT_VARIATION_CHANGE_H
