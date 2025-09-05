#include "quickjs_wrapper.h"
#include <iostream>
#include <vector>

using namespace QuickJSWrapper;

void basicUsageExample() {
    std::cout << "=== Basic Usage Example ===" << std::endl;
    
    try {
        Context ctx;
        
        // Evaluate simple expressions
        auto result = ctx.eval("2 + 3 * 4");
        std::cout << "2 + 3 * 4 = " << result.toNumber() << std::endl;
        
        // String operations
        auto strResult = ctx.eval("'Hello, ' + 'World!'");
        std::cout << "String result: " << strResult.toString() << std::endl;
        
        // Object creation and manipulation
        ctx.eval("var obj = { name: 'John', age: 30 }");
        auto obj = ctx.getGlobalProperty("obj");
        std::cout << "obj.name: " << obj.getProperty("name").toString() << std::endl;
        std::cout << "obj.age: " << obj.getProperty("age").toNumber() << std::endl;
        
        // Array operations
        ctx.eval("var arr = [1, 2, 3, 4, 5]");
        auto arr = ctx.getGlobalProperty("arr");
        std::cout << "Array length: " << arr.getArrayLength() << std::endl;
        std::cout << "arr[2]: " << arr.getElement(2).toNumber() << std::endl;
        
    } catch (const Exception& e) {
        std::cerr << "Error in basic usage: " << e.what() << std::endl;
    }
}

void nativeFunctionExample() {
    std::cout << "\n=== Native Function Example ===" << std::endl;
    
    try {
        Context ctx;
        
        // Register a C++ function that can be called from JavaScript
        ctx.setGlobalFunction("multiply", [](const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw Exception("multiply() requires exactly 2 arguments");
            }
            
            double a = args[0].toNumber();
            double b = args[1].toNumber();
            
            // Create return value using the context from the arguments
            JSContext* js_ctx = args[0].getContext();
            return Value(js_ctx, JS_NewFloat64(js_ctx, a * b), true);
        });
        
        // Call the native function from JavaScript
        auto result = ctx.eval("multiply(6, 7)");
        std::cout << "multiply(6, 7) = " << result.toNumber() << std::endl;
        
        // Register a function that manipulates objects
        ctx.setGlobalFunction("createPerson", [](const std::vector<Value>& args) -> Value {
            if (args.size() != 2) {
                throw Exception("createPerson() requires exactly 2 arguments");
            }
            
            JSContext* js_ctx = args[0].getContext();
            JSValue obj = JS_NewObject(js_ctx);
            
            // Set name property
            JS_SetPropertyStr(js_ctx, obj, "name", JS_DupValue(js_ctx, args[0].getJSValue()));
            // Set age property  
            JS_SetPropertyStr(js_ctx, obj, "age", JS_DupValue(js_ctx, args[1].getJSValue()));
            
            return Value(js_ctx, obj, true);
        });
        
        // Use the function
        auto person = ctx.eval("createPerson('Alice', 25)");
        std::cout << "Created person: " << person.getProperty("name").toString() 
                  << ", age " << person.getProperty("age").toNumber() << std::endl;
        
    } catch (const Exception& e) {
        std::cerr << "Error in native function example: " << e.what() << std::endl;
    }
}

void valueCreationExample() {
    std::cout << "\n=== Value Creation Example ===" << std::endl;
    
    try {
        Context ctx;
        
        // Create various types of values
        auto undefinedVal = ctx.newUndefined();
        auto nullVal = ctx.newNull();
        auto boolVal = ctx.newBool(true);
        auto numberVal = ctx.newNumber(3.14159);
        auto stringVal = ctx.newString("Hello from C++");
        
        std::cout << "Undefined: " << (undefinedVal.isUndefined() ? "true" : "false") << std::endl;
        std::cout << "Null: " << (nullVal.isNull() ? "true" : "false") << std::endl;
        std::cout << "Bool: " << (boolVal.toBool() ? "true" : "false") << std::endl;
        std::cout << "Number: " << numberVal.toNumber() << std::endl;
        std::cout << "String: " << stringVal.toString() << std::endl;
        
        // Create and populate an object
        auto obj = ctx.newObject();
        obj.setProperty("message", stringVal);
        obj.setProperty("value", numberVal);
        obj.setProperty("active", boolVal);
        
        // Set as global and access from JavaScript
        ctx.setGlobalProperty("myObject", obj);
        
        auto jsResult = ctx.eval("myObject.message + ' - Value: ' + myObject.value");
        std::cout << "JavaScript result: " << jsResult.toString() << std::endl;
        
        // Create an array
        std::vector<Value> elements = {
            ctx.newNumber(1),
            ctx.newNumber(2),
            ctx.newNumber(3),
            ctx.newString("four"),
            ctx.newBool(true)
        };
        auto arr = ctx.newArray(elements);
        
        ctx.setGlobalProperty("myArray", arr);
        auto arrayInfo = ctx.eval("'Array length: ' + myArray.length + ', last element: ' + myArray[myArray.length-1]");
        std::cout << arrayInfo.toString() << std::endl;
        
    } catch (const Exception& e) {
        std::cerr << "Error in value creation example: " << e.what() << std::endl;
    }
}

void errorHandlingExample() {
    std::cout << "\n=== Error Handling Example ===" << std::endl;
    
    try {
        Context ctx;
        
        // This will cause a syntax error
        try {
            ctx.eval("var x = ;"); // Invalid syntax
        } catch (const Exception& e) {
            std::cout << "Caught syntax error: " << e.what() << std::endl;
        }
        
        // This will cause a runtime error
        try {
            ctx.eval("nonExistentFunction()");
        } catch (const Exception& e) {
            std::cout << "Caught runtime error: " << e.what() << std::endl;
        }
        
        // Demonstrate successful execution after error
        auto result = ctx.eval("'Execution continues normally'");
        std::cout << "After errors: " << result.toString() << std::endl;
        
    } catch (const Exception& e) {
        std::cerr << "Unexpected error in error handling example: " << e.what() << std::endl;
    }
}

void memoryUsageExample() {
    std::cout << "\n=== Memory Usage Example ===" << std::endl;
    
    try {
        Context ctx;
        
        std::cout << "Initial memory usage: " << ctx.getMemoryUsage() << " bytes" << std::endl;
        
        // Create some objects
        ctx.eval(R"(
            var objects = [];
            for (var i = 0; i < 1000; i++) {
                objects.push({ id: i, data: 'some data ' + i });
            }
        )");
        
        std::cout << "After creating 1000 objects: " << ctx.getMemoryUsage() << " bytes" << std::endl;
        
        // Clear references
        ctx.eval("objects = null;");
        
        std::cout << "After clearing references: " << ctx.getMemoryUsage() << " bytes" << std::endl;
        
        // Force garbage collection
        ctx.runGC();
        
        std::cout << "After garbage collection: " << ctx.getMemoryUsage() << " bytes" << std::endl;
        
    } catch (const Exception& e) {
        std::cerr << "Error in memory usage example: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "QuickJS C++ Wrapper Examples" << std::endl;
    std::cout << "=============================" << std::endl;
    
    basicUsageExample();
    nativeFunctionExample();
    valueCreationExample();
    errorHandlingExample();
    memoryUsageExample();
    
    std::cout << "\nAll examples completed!" << std::endl;
    return 0;
}