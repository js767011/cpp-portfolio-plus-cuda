//This small section of code will serve to increase my knowledge of C++11 and learn some more complex topics. Feel free to utilize my code as you see fit.

// ---------------------------------------------------------
// WELCOME TO C++ 11! This will cover the following features:
//  1. smart pointers:                  std::unique_ptr
//  2. move semantics:                  std::move
//  3. lambda expressions:              dispatcher.subscribe(EventType::PlayerJoined, [](const Event* e) {...})
//  4. auto keyword:                    for (const auto& eventPtr : eventQueue)
//  5. range-based for loops:           for (const auto& eventPtr : eventQueue)
//  6. type aliases:                    using EventCallback = ...
//  7. explicitly defaulted functions:  = default
//  8. strongly-typed enums:            enum class
// ---------------------------------------------------------
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>

// ---------------------------------------------------------
// 1. THE EVENTS (Dummy Event Objects)
// ---------------------------------------------------------
//utilize an enum class for the different event types -> think of a drop-down list of options
enum class EventType {
    PlayerJoined,   //stored as 0
    PlayerLeft      //stored as 1
};

//Base Event struct -> acts as the parent struct
//breakdown of the destructor:
//  virtual -> a keyword that ensures that the child struct below that calls the parent will be destroyed, not just the parent
//  ~Event() -> the actual destructor syntax
//  = default -> c++11 feature keyword that tells the compiler: dont need to write custom cleanup code, just generate standard c++ destruction logic
struct Event {
    EventType type;
    Event(EventType t) : type(t) {} //the constructor, in member initializer list format (faster than alternative: Event(EventTypet) {type = t;} where garbage member is assigned then overwritten with type)
    virtual ~Event() = default;     //the virtual destructor that ensures proper cleanup
};

//specific dummy events carrying data
//the syntax of PlayerJoinedEvent : public Event -> the ':' is the inheritance symbol and identifies PlayerJoinedEvent as a child of Event
//  public is an access specifier and means everything the outside world was allowed to see in parent Event is viewable in new child
//using this strategy of polymoprhism means the compiler will recognize the child structs the same as the parent Event, meaning easier consolidation in 
//  a container like a vector as shown below
struct PlayerJoinedEvent : public Event {
    std::string playerName;
    //below is child struct constructor:
    //  PlayerJoinedEvent(const std::string& name) -> defines how the event is created, via a const ref of a string (dont waste time making a copy of string, just look at the original string in memory)
    //  : -> again, the member initializer list
    //  Event(EventType::PlayerJoined) -> call the parent constructor with a hard-coded type of PlayerJoined
    //  , playerName(name) -> once parent is built, take sname argument into function and assign it to the playerName variable
    //  {} -> no logic left to execute, body is left empty
    PlayerJoinedEvent(const std::string& name) : Event(EventType::PlayerJoined), playerName(name) {}
};

//similar logic as above
struct PlayerLeftEvent : public Event {
    int playerId;
    PlayerLeftEvent(int id) : Event(EventType::PlayerLeft), playerId(id) {}
};

// ---------------------------------------------------------
// 2. THE EVENT SYSTEM (The Dispatcher)
// ---------------------------------------------------------
//FEATURE use a class and not a struct, object is an active manager not just a container
class EventSystem { 
private:
    //2 c++11 features here: 'using' and 'std::function'
    //using is a c++11 keyword is a type alias (replaces C typedef) and allows the definition of almost anything,
        //just cant use a protected keyword as the name of the alias. Only for types, never for values or overriding the language's
        //core syntax. Also cannot do macros. Very handy for hiding complex pointer logic or deeply nested std library types.
        //Can also make alias temples like so:
        //template <typename T>
        //using StringMap = std::map<std::string, T>;
        //StringMap<int> playerScores;    -> becomes std::map<std::string, int>
        //StringMap<float> playerHealth;  -> becomes std::map<std::string, float>
        //
    //std::function solves a large problem of the c++ compiler:
        //keep in mind that function signatures are actually Data Types.
        //so std::function<void(const Event*)> is essentially a container (std::function) being handed a very specific type of void(const Event*), which
        //is just a read-only pointer to an event (const Event*) that does not return anything. But if we know the lambda returns void and takes a const Event*,
        //why need the std::function at all? Why not make a vector of lambdas? this is because every single lambda is assigned a unique, unnamable data type by the
        //compiler under the hood. so even identical lambdas such as:
        //  auto lambda1 = [](const Event* e) {std::cout << "Hi";};
        //  auto lambda2 = [](const Event* e) {std::cout << "Hi";};
        //will be assigned different types bt the compiler like: lambda1 is type CompilerGeneratedClass_8473 and lambda2 is type CompilerGeneratedClass_9921
        //so since std::vector can only hold one specific data type, this would throw errors if multiple lambdas were pushed into a single vector.
        //std::function utilizes type erasure and acts as a universal adapter, which hides the weird compiler-generated type and presents a uniform
        //type of void(const Event*) to the vector
    using EventCallback = std::function<void(const Event*)>;
    
    //routing table that maps an EventType to a list of callback functions:
    //  uses unordered_map hash map, key is EventType (e.g., PlayerJoined), value is a std::vector of EventCallback functions
    //  basically, if I look up the PlayerJoined key, hand me the list of every single function that wants to know when a player joins
    std::unordered_map<EventType, std::vector<EventCallback>> listeners;

    //std::unique_ptr manages the memory of our events automatically. vector acts as the waiting room, collecting events as they happen
    std::vector<std::unique_ptr<Event>> eventQueue;

public:
    //subscribe a lambda function to a specific event type. 
        //listeners[type].push_back(callback): map looks for the EventType key passed in, then grabs the std::vector associated with that key (if none exist, map makes new vector for it)
        //then the .push_back(callback) takes the std::function container passed in and adds it to the end of the specific vector
    void subscribe(EventType type, EventCallback callback) {
        listeners[type].push_back(callback);
    }

    //add a new event to the queue (Takes ownership of the unique_ptr)
    void postEvent(std::unique_ptr<Event> newEvent) {
        //std::move transfers ownership of the memory into the vector
            //by definition, std::unique_ptr represents exclusive ownership of a piece of memory. only one unique_ptr can point to a specific memory address at a time.
            //so a normal push into a vector such as eventQueue.push_back(newEvent) will result in an error, because a standard push_back tries to make a copy of the
            //variable. if the compiler allowed that, there would be 2 unique_ptr's to the same memory address. std::move() solves this -> when applied to an object, that object
            //is stripped of its ownership and its contents is offered up. as applied below, here is how it works:
                //1. void postEvent(std::unique_ptr<Event> newEvent) is called, a new unique_ptr named newEvent is created; holds the raw memory address of the event (e.g.,a PlayerJoinedEvent)
                //2. by wrapping newEvent as: std::move(newEvent), the compiler is told "we dont need newEvent anymore, get ready to gut it"
                //3. eventQueue.push_back then occurs, and the vector sees that there is a "guttable" variable being passed in. instead of trying to make a copy, the vecor triggers a move constructor
                //      where the vector reaches inside newEvent, steals memory address, places the memory address inside a new unique_ptr within its own array, leaves original newEvent empty (pointing to nullptr)
        eventQueue.push_back(std::move(newEvent));
    }

    //process all events and trigger callbacks
    void dispatch() {
        //range-based for loop and 'auto' keyword
        for (const auto& eventPtr : eventQueue) {
            EventType currentType = eventPtr->type;
            
            // Check if anyone is listening to this event type
            if (listeners.find(currentType) != listeners.end()) {
                // Iterate through all callbacks registered to this type
                for (const auto& callback : listeners[currentType]) {
                    callback(eventPtr.get()); // Pass the raw pointer to the lambda
                }
            }
        }

        // When we clear the queue, std::unique_ptr automatically calls the 
        // destructors for all the events. No 'delete' keyword needed!
        eventQueue.clear(); 
    }
};

// ---------------------------------------------------------
// 3. TESTING THE IMPLEMENTATION
// ---------------------------------------------------------
int main() {
    EventSystem dispatcher;

    // FEATURE 4: Register callbacks using Inline Lambda Expressions
    // We capture [] nothing, pass (const Event* e) as arguments, and write the logic inline.

    
    dispatcher.subscribe(EventType::PlayerJoined, [](const Event* e) {
        // Safely cast the base Event to our specific PlayerJoinedEvent
        const auto* joinedEvent = static_cast<const PlayerJoinedEvent*>(e);
        std::cout << "[SYSTEM] Welcome to the server, " << joinedEvent->playerName << "!\n";
    });

    dispatcher.subscribe(EventType::PlayerLeft, [](const Event* e) {
        const auto* leftEvent = static_cast<const PlayerLeftEvent*>(e);
        std::cout << "[SYSTEM] Player ID " << leftEvent->playerId << " has disconnected.\n";
    });

    // Let's add a second listener to the PlayerJoined event to show multiple callbacks work!
    dispatcher.subscribe(EventType::PlayerJoined, [](const Event* e) {
        std::cout << "[METRICS] A new player joined. Updating daily active user count...\n";
    });

    // --- Simulating Gameplay ---
    std::cout << "--- Starting Simulation ---\n";

    // Pushing events into the system using std::unique_ptr
    dispatcher.postEvent(std::unique_ptr<Event>(new PlayerJoinedEvent("Gordon_Freeman")));
    dispatcher.postEvent(std::unique_ptr<Event>(new PlayerLeftEvent(404)));
    dispatcher.postEvent(std::unique_ptr<Event>(new PlayerJoinedEvent("Alyx_Vance")));

    std::cout << "Events queued. Dispatching now...\n\n";

    // Trigger all callbacks and automatically clean up memory
    dispatcher.dispatch();

    std::cout << "\n--- Simulation Ended (Zero Memory Leaks) ---\n";

    return 0;
}