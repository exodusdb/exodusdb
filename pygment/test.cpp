#include <iostream> // Preprocessor directive

// Function to demonstrate various C++ keywords
int main() { // 'int' is a fundamental type, 'main' is identifier

	std::cout << something << std::endl;

    // Fundamental types
    char character = 'a';        // Character type
    short short_num = 10;       // Short integer type
    int integer = 20;          // Integer type
    long long_num = 30L;      // Long integer type
    float floating = 3.14f;   // Floating-point type
    double double_num = 3.14159; // Double precision floating-point type
    bool boolean = true;      // Boolean type

    // Storage class specifiers
    auto automatic = 1;       // Automatic storage duration
    static int static_var = 2;// Static storage duration
    extern int external_var;  // External linkage
    register int reg_var = 3; // Register storage suggestion

    // Control flow keywords
    if (boolean) {            // Conditional statement
        goto label;          // Jump statement
    }
 
    while (integer > 0) {     // Loop statement
        integer--;
        if (integer == 10)
            break;           // Loop control
        else if (integer == 15)
            continue;       // Loop control
    }

    do {                     // Loop statement
        integer++;
    } while (integer < 20);

    for (int i = 0; i < 5; i++) { // Loop statement
        switch (i) {         // Selection statement
            case 0:         // Case label
                break;
            default:        // Default case
                break;
        }
    }

    // Type qualifiers
    const int constant = 42;     // Constant qualifier
    volatile int vol_var = 0;    // Volatile qualifier

    // Class-related keywords
    class MyClass {             // Class definition
    public:                    // Access specifier
        virtual void func() = 0; // Virtual function
    private:                  // Access specifier
        int data;
    protected:               // Access specifier
        int more_data;
    };

    struct MyStruct {         // Structure definition
        int x;
    };

    union MyUnion {          // Union definition
        int i;
        float f;
    };

    // Exception handling
    try {                   // Exception handling block
        throw 42;          // Throw expression
    } catch (int e) {      // Exception handler
        // Handle exception
    }

    // Other keywords
    enum Color { RED, GREEN }; // Enumeration
    typedef int Integer;      // Type alias
    using Number = float;     // Type alias (modern)
    namespace MySpace {      // Namespace definition
        int var;
    }

    template <typename T>    // Template declaration
    T add(T a, T b) {
        return a + b;
    }

    // Operator keywords
    int* ptr = new int;     // Dynamic memory allocation
    delete ptr;            // Memory deallocation
    sizeof(int);          // Size operator

    // Boolean literals
    true;                // Boolean literal
    false;              // Boolean literal

    // Null pointer
    nullptr;            // Null pointer literal

label:                  // Label for goto
    return 0;          // Return statement
}

// Note: Some keywords like 'inline', 'friend', 'explicit', 'mutable'
// could be added in more complex examples

// test.cpp
#include <iostream>

int main() {
    dim x;           // EXODUS_TYPES
    var y;           // EXODUS_TYPES
    print("Hello");  // EXODUS_FUNCTIONS
    input(x);        // EXODUS_FUNCTIONS
    read(y);         // EXODUS_FUNCTIONS
    write(x);        // EXODUS_FUNCTIONS
    if (true) {      // EXODUS_FLOWCONTROL, EXODUS_CONSTANTS
        for (int i = 0; i < 5; i++) {  // EXODUS_FLOWCONTROL
            print("Loop");  // EXODUS_FUNCTIONS
            next;           // EXODUS_FLOWCONTROL
        }
        then {          // EXODUS_FLOWCONTROL
            call xyz("sub"); // EXODUS_DECLARATIONS
        } else {        // EXODUS_FLOWCONTROL
            gosub xyz("alt"); // EXODUS_DECLARATIONS
        }
    }
    system("ls");    // EXODUS_PSEUDO
    env("PATH");     // EXODUS_PSEUDO
    int z = len("test");  // EXODUS_BUILTINS
    substr(z, 1, 2);      // EXODUS_BUILTINS
    bool b = x and y;     // EXODUS_OPERATORS
    b = x or false;       // EXODUS_OPERATORS
	b = a eq b ne c lt d gt e le f ge g; // EXODUS_OPERATORS
	osread(xxx on from with to yyy)
    return 0;
}


#include <exodus/library.h>
libraryinit()

function main() {
	printl("f1 says 'Hello World!'");
	return 0;
}

libraryexit()


// ???_common.h
#include <exodus/common.h>
commoninit(abc)

commonexit(abc)


#include <exodus/program.h>

programinit()

  #include <externalfunc.h>

func main() {

    printl("program1 says", "'Hello World!'", datetime());

    let c1 = 1, c2 = 10;
    for (auto i : range(c1, c2)) {
      printt(i);
      if (not mod(i, 3)) printx("Fizz");
      if (not mod(i, 5)) printx("Buzz");
      printl();
    }
    call func1(1'234.56, "USD").outputl();

    gosub subr1(1'234.56, "USD");
    printl(arg3);

    if (123 > 
    return 0;
  }

func func1(in arg1, in arg2) {
	stop();
	abort();
	abortall();
    return arg1 ^ arg2; // Concatenate
  }

subr subr1(in arg1, in arg2, out arg3) {
    arg3 = oconv(arg1, arg2);
  }

programexit()
programinit()
