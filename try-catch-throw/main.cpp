#include <iostream>
#include <csetjmp>
#include <vector>

using namespace std;

// ########################################## try-throw-catch ##########################################

struct Parent {
    jmp_buf env{};
    bool caught = false;
    virtual ~Parent() = default;
};

template <typename T>
struct Child: Parent {
    explicit Child(T value): value(move(value)) {}
    T value;
};

vector<Parent*> contexts;
bool is_equal = false;
bool skip = true;

#define try \
    if (contexts.empty() || !contexts.back()->caught) \
        contexts.push_back(new Parent); \
    if (!contexts.back()->caught) \
        if (!setjmp(contexts.back()->env))

#define throw(error) \
    if (!contexts.back()->caught) {\
        Parent* next_context = contexts.back(); \
        contexts.pop_back(); \
        contexts.push_back(new Child<decltype(error)>(error)); \
        contexts.back()->caught = true; \
        longjmp(next_context->env, 1); \
    } \
    nullptr //for ";" after throw

#define catch(error) \
    skip = contexts.empty(); \
    if (!skip) {\
        is_equal = is_same<All, decltype(error)>::value and contexts.back()->caught; \
            if (!is_equal) {\
                is_equal = dynamic_cast<Child<decltype(error)>*>(contexts.back()) != nullptr; \
                if (is_equal) { \
                    (error) = dynamic_cast<Child<decltype(error)>*>(contexts.back())->value; \
                    contexts.pop_back(); \
                } \
            } else { \
               contexts.pop_back(); \
            } \
    } \
    if (!skip and is_equal)

struct All {};
All all;

// ########################################## try-throw-catch ##########################################

struct CustomStruct {
    CustomStruct() {}
    CustomStruct(int value, string s): value(value), s(move(s)) {}

    int value;
    string s;
};

void bar() {
    CustomStruct custom_struct(2022, "Hello, World!");
    throw(custom_struct);
    cout << "after throw" << endl;
}

void foo() {
    try {
        bar();
    }
}

int main(){
    try {
        cout << "Program started" << endl;
    }
    catch(all) {
        cout << "1st All" << endl; // shouldn't work
    }

    foo();

    try { cout << "2nd try" << endl; } // shouldn't work
    throw(10); // shouldn't work
    try { cout << "3rd try" << endl; } // shouldn't work

    int a = -1;
    catch(a) {
        cout << a << endl; // shouldn't work
    }
    CustomStruct custom_struct;
    catch(custom_struct) {
        cout << custom_struct.value << endl;
        cout << custom_struct.s << endl;
    }
    catch(all) {
        cout << "2nd All" << endl; // shouldn't work
    }

    try {
        int value = -1;
        throw(value);
    }
    catch(all) {
        cout << "3rd All" << endl;
    }
    int value;
    catch(value) {
        cout << value << endl; // shouldn't work
    }

    try {
        int val = -1;
        throw(val);
    }
    int to_catch;
    catch(to_catch) {
        cout << to_catch << endl;
    }
    catch(all) {
        cout << "4st All" << endl; // shouldn't work
    }

    return 0;
}
