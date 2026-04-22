#include <iostream>
#include <algorithm> //needed for std::copy

class MyVector {
  private:
    int* data;            //raw pointer to the heap memory
    std::size_t size;     //number of elements in the vecotr
    std::size_t capacity; //total allocated memory

  public:
    
    //default constructor
    MyVector() : data(nullptr), size(0), capacity(0) {} //member initialization list, more efficient than calling MyVector() {data, size, capacity}

    //parameterized constructor
    MyVector(std::size_t cap) : size(0), capacity(cap) {
      data = new int[capacity];
    }

    //desctructor (Rule 1)
    ~MyVector() {
      std::cout << "--> Desctructor Called! Freeing memory at pointer: " << data << "\n";
      delete[] data;
    }

    //copy semantics: deep copies (Rules 2 & 3)
    //copy constructor
    MyVector(const MyVector& other) : size(other.size), capacity(other.capacity) {
      std::cout << "--> Copy Constructor Called (Deep Copy)\n";
      data = new int[capacity];
      std::copy(other.data, other.data + size, data);
    }

    //copy assignment operator
    MyVector& operator=(const MyVector& other) {
        std::cout << "-> Copy Assignment Called (Deep Copy)\n";
        
        // Guard against self-assignment (e.g., vec1 = vec1)
        if (this == &other) {
            return *this;
        }

        // Free the existing memory of the target object
        delete[] data;

        // Perform the deep copy
        size = other.size;
        capacity = other.capacity;
        data = new int[capacity];
        std::copy(other.data, other.data + size, data);

        return *this;
    }

    // --- 4. Move Semantics: Pointer Stealing (Rules 4 & 5) ---
    // Move Constructor
    MyVector(MyVector&& other) noexcept 
        : data(other.data), size(other.size), capacity(other.capacity) {
        
        std::cout << "-> Move Constructor Called (Pointer Stolen)\n";
        
        // Nullify the source object so its destructor safely does nothing
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }

    // Move Assignment Operator
    MyVector& operator=(MyVector&& other) noexcept {
        std::cout << "-> Move Assignment Called (Pointer Stolen)\n";
        
        // Guard against self-assignment
        if (this == &other) {
            return *this;
        }

        // Free our current memory
        delete[] data;

        // Steal the memory from the other object
        data = other.data;
        size = other.size;
        capacity = other.capacity;

        // Nullify the source object
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;

        return *this;
    }

    // --- 5. Utility ---
    void printInfo() const {
        std::cout << "Size: " << size << ", Capacity: " << capacity << ", Pointer: " << data << "\n";
    }
  };

  // --- Execution & Testing ---
int main() {
    std::cout << "--- 1. Initialization ---\n";
    MyVector vec1(10); 
    std::cout << "vec1 created. "; vec1.printInfo();

    std::cout << "\n--- 2. Copy Constructor ---\n";
    MyVector vec2 = vec1; //MyVector(vec1)
    std::cout << "vec2 created from vec1. "; vec2.printInfo();

    std::cout << "\n--- 3. Copy Assignment ---\n";
    MyVector vec3;
    vec3 = vec1; 
    std::cout << "vec3 assigned from vec1. "; vec3.printInfo();

    std::cout << "\n--- 4. Move Constructor ---\n";
    // std::move casts vec1 to an rvalue, allowing vec4 to steal its memory
    MyVector vec4 = std::move(vec1); 
    std::cout << "vec4 stole memory from vec1. "; vec4.printInfo();

    std::cout << "\n--- 5. Move Assignment ---\n";
    MyVector vec5;
    vec5 = std::move(vec2);
    std::cout << "vec5 stole memory from vec2. "; vec5.printInfo();

    std::cout << "\n--- Final Verification ---\n";
    std::cout << "vec1 (after being robbed): "; vec1.printInfo(); // Should show size 0, pointer 0x0
    std::cout << "vec2 (after being robbed): "; vec2.printInfo(); // Should show size 0, pointer 0x0

    std::cout << "\n(Destructors will now run automatically to clean up the heap)\n";
    return 0; 
}