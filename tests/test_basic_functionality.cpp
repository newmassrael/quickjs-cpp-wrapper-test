#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "quickjs_wrapper.h"
#include <chrono>
#include <thread>

using namespace QuickJSWrapper;
using namespace testing;

// Tests validating QuickJS core functionality safety and reliability
class BasicFunctionalityTest : public Test {
protected:
    void SetUp() override {
        ctx = std::make_unique<Context>();
    }

    void TearDown() override {
        ctx.reset();
    }

    std::unique_ptr<Context> ctx;
};

// Validates QuickJS safely handles value creation and type conversion
TEST_F(BasicFunctionalityTest, ValueCreationAndConversion) {
    // Number values
    auto numVal = ctx->newNumber(42.5);
    EXPECT_TRUE(numVal.isNumber());
    EXPECT_FALSE(numVal.isString());
    EXPECT_DOUBLE_EQ(numVal.toNumber(), 42.5);
    
    // Integer values
    auto intVal = ctx->newInt32(123);
    EXPECT_TRUE(intVal.isNumber());
    EXPECT_EQ(intVal.toInt32(), 123);
    
    // String values
    auto strVal = ctx->newString("Hello, World!");
    EXPECT_TRUE(strVal.isString());
    EXPECT_FALSE(strVal.isNumber());
    EXPECT_EQ(strVal.toString(), "Hello, World!");
    
    // Boolean values
    auto boolVal = ctx->newBool(true);
    EXPECT_TRUE(boolVal.isBool());
    EXPECT_TRUE(boolVal.toBool());
    
    auto falseBool = ctx->newBool(false);
    EXPECT_TRUE(falseBool.isBool());
    EXPECT_FALSE(falseBool.toBool());
    
    // Undefined and null
    auto undefinedVal = ctx->newUndefined();
    EXPECT_TRUE(undefinedVal.isUndefined());
    EXPECT_FALSE(undefinedVal.isNull());
    
    auto nullVal = ctx->newNull();
    EXPECT_TRUE(nullVal.isNull());
    EXPECT_FALSE(nullVal.isUndefined());
}

// Validates QuickJS safely executes JavaScript code with proper error handling
TEST_F(BasicFunctionalityTest, JavaScriptExecution) {
    // Simple arithmetic
    auto result = ctx->eval("2 + 3 * 4");
    EXPECT_EQ(result.toInt32(), 14);
    
    // String operations
    auto strResult = ctx->eval("'Hello, ' + 'World!'");
    EXPECT_EQ(strResult.toString(), "Hello, World!");
    
    // Boolean operations
    auto boolResult = ctx->eval("true && false");
    EXPECT_FALSE(boolResult.toBool());
    
    // Complex expression
    auto complexResult = ctx->eval("Math.pow(2, 10)");
    EXPECT_EQ(complexResult.toInt32(), 1024);
}

// Validates QuickJS safely handles object property operations
TEST_F(BasicFunctionalityTest, ObjectOperations) {
    // Create object
    auto obj = ctx->newObject();
    EXPECT_TRUE(obj.isObject());
    
    // Set properties
    obj.setProperty("name", ctx->newString("John"));
    obj.setProperty("age", ctx->newInt32(30));
    obj.setProperty("active", ctx->newBool(true));
    
    // Get properties
    auto name = obj.getProperty("name");
    EXPECT_TRUE(name.isString());
    EXPECT_EQ(name.toString(), "John");
    
    auto age = obj.getProperty("age");
    EXPECT_TRUE(age.isNumber());
    EXPECT_EQ(age.toInt32(), 30);
    
    auto active = obj.getProperty("active");
    EXPECT_TRUE(active.isBool());
    EXPECT_TRUE(active.toBool());
}

// Validates QuickJS safely processes array operations and methods
TEST_F(BasicFunctionalityTest, ArrayOperations) {
    // Create empty array
    auto arr = ctx->newArray();
    EXPECT_TRUE(arr.isArray());
    EXPECT_EQ(arr.getArrayLength(), 0);
    
    // Create array with elements
    std::vector<QuickJSWrapper::Value> elements = {
        ctx->newInt32(1),
        ctx->newInt32(2), 
        ctx->newInt32(3)
    };
    auto populatedArr = ctx->newArray(elements);
    EXPECT_TRUE(populatedArr.isArray());
    EXPECT_EQ(populatedArr.getArrayLength(), 3);
    
    // Access elements
    auto firstElement = populatedArr.getElement(0);
    EXPECT_EQ(firstElement.toInt32(), 1);
    
    auto lastElement = populatedArr.getElement(2);
    EXPECT_EQ(lastElement.toInt32(), 3);
    
    // Modify elements
    populatedArr.setElement(1, ctx->newString("modified"));
    auto modifiedElement = populatedArr.getElement(1);
    EXPECT_TRUE(modifiedElement.isString());
    EXPECT_EQ(modifiedElement.toString(), "modified");
}

// Validates QuickJS safely manages global scope property access
TEST_F(BasicFunctionalityTest, GlobalPropertyAccess) {
    // Set global property
    ctx->setGlobalProperty("testGlobal", ctx->newString("global_value"));
    
    // Access via JavaScript
    auto result = ctx->eval("testGlobal");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.toString(), "global_value");
    
    // Access via wrapper
    auto globalProp = ctx->getGlobalProperty("testGlobal");
    EXPECT_TRUE(globalProp.isString());
    EXPECT_EQ(globalProp.toString(), "global_value");
}

// Validates QuickJS safely binds and calls native C++ functions
TEST_F(BasicFunctionalityTest, NativeFunctionBinding) {
    // Simple native function
    ctx->setGlobalFunction("add", [](const std::vector<QuickJSWrapper::Value>& args) -> QuickJSWrapper::Value {
        if (args.size() != 2) {
            throw Exception("add() requires exactly 2 arguments");
        }
        
        double a = args[0].toNumber();
        double b = args[1].toNumber();
        JSContext* js_ctx = args[0].getContext();
        
        return QuickJSWrapper::Value(js_ctx, JS_NewFloat64(js_ctx, a + b), true);
    });
    
    // Call from JavaScript
    auto result = ctx->eval("add(10, 32)");
    EXPECT_EQ(result.toInt32(), 42);
    
    // Test error handling
    EXPECT_THROW({
        ctx->eval("add(1)"); // Wrong number of arguments
    }, Exception);
}

// Validates QuickJS safely handles JavaScript errors and exceptions
TEST_F(BasicFunctionalityTest, ErrorHandling) {
    // Syntax error
    EXPECT_THROW({
        ctx->eval("var x = ;"); // Invalid syntax
    }, Exception);
    
    // Runtime error
    EXPECT_THROW({
        ctx->eval("nonExistentFunction()"); // Undefined function
    }, Exception);
    
    // Reference error
    EXPECT_THROW({
        ctx->eval("undeclaredVariable"); // Undefined variable
    }, Exception);
    
    // Context should still work after errors
    auto result = ctx->eval("2 + 2");
    EXPECT_EQ(result.toInt32(), 4);
}

// Validates QuickJS safely manages memory with proper RAII patterns
TEST_F(BasicFunctionalityTest, MemoryManagement) {
    size_t initialMemory = ctx->getMemoryUsage();
    EXPECT_GT(initialMemory, 0);
    
    // Create many objects
    ctx->eval(R"(
        var objects = [];
        for (var i = 0; i < 1000; i++) {
            objects.push({
                id: i,
                data: 'data_' + i,
                nested: { value: i * 2 }
            });
        }
    )");
    
    size_t afterCreation = ctx->getMemoryUsage();
    EXPECT_GT(afterCreation, initialMemory);
    
    // Clear references
    ctx->eval("objects = null;");
    
    // Force garbage collection
    ctx->runGC();
    
    size_t afterGC = ctx->getMemoryUsage();
    // Memory should be reduced after GC, but might not return to initial level
    EXPECT_LE(afterGC, afterCreation);
}

// Validates QuickJS safely handles complex nested data structures
TEST_F(BasicFunctionalityTest, ComplexDataStructures) {
    // Nested objects and arrays
    ctx->eval(R"(
        var complex = {
            numbers: [1, 2, 3, 4, 5],
            strings: ['a', 'b', 'c'],
            nested: {
                deep: {
                    value: 'deep_value',
                    array: [
                        { id: 1, name: 'first' },
                        { id: 2, name: 'second' }
                    ]
                }
            }
        };
    )");
    
    auto complex = ctx->getGlobalProperty("complex");
    EXPECT_TRUE(complex.isObject());
    
    auto numbers = complex.getProperty("numbers");
    EXPECT_TRUE(numbers.isArray());
    EXPECT_EQ(numbers.getArrayLength(), 5);
    
    auto firstNumber = numbers.getElement(0);
    EXPECT_EQ(firstNumber.toInt32(), 1);
    
    auto nested = complex.getProperty("nested");
    auto deep = nested.getProperty("deep");
    auto deepValue = deep.getProperty("value");
    EXPECT_EQ(deepValue.toString(), "deep_value");
    
    auto deepArray = deep.getProperty("array");
    EXPECT_TRUE(deepArray.isArray());
    EXPECT_EQ(deepArray.getArrayLength(), 2);
    
    auto firstItem = deepArray.getElement(0);
    auto itemName = firstItem.getProperty("name");
    EXPECT_EQ(itemName.toString(), "first");
}

// Validates QuickJS safely executes function calls and method invocations
TEST_F(BasicFunctionalityTest, FunctionCallsAndMethods) {
    // Define object with methods
    ctx->eval(R"(
        var calculator = {
            value: 0,
            add: function(n) {
                this.value += n;
                return this;
            },
            multiply: function(n) {
                this.value *= n;
                return this;
            },
            getValue: function() {
                return this.value;
            }
        };
    )");
    
    auto calc = ctx->getGlobalProperty("calculator");
    
    // Method chaining
    auto result = calc.callMethod("add", {ctx->newInt32(5)});
    result.callMethod("multiply", {ctx->newInt32(3)});
    
    auto finalValue = calc.callMethod("getValue", {});
    EXPECT_EQ(finalValue.toInt32(), 15);
}