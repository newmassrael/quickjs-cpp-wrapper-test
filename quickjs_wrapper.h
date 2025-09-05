#pragma once

#include "quickjs/quickjs.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <stdexcept>

namespace QuickJSWrapper {

class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& message) : std::runtime_error(message) {}
};

class Value {
private:
    JSContext* ctx_;
    JSValue val_;
    bool owned_;

public:
    Value(JSContext* ctx, JSValue val, bool owned = true);
    Value(const Value& other);
    Value(Value&& other) noexcept;
    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;
    ~Value();

    // Type checking
    bool isUndefined() const;
    bool isNull() const;
    bool isBool() const;
    bool isNumber() const;
    bool isString() const;
    bool isObject() const;
    bool isFunction() const;
    bool isArray() const;

    // Value conversion
    bool toBool() const;
    int32_t toInt32() const;
    double toNumber() const;
    std::string toString() const;
    
    // Object/Array operations
    Value getProperty(const std::string& name) const;
    void setProperty(const std::string& name, const Value& value);
    Value getElement(int index) const;
    void setElement(int index, const Value& value);
    size_t getArrayLength() const;

    // Function call
    Value call(const std::vector<Value>& args = {}) const;
    Value callMethod(const std::string& method, const std::vector<Value>& args = {}) const;

    // Raw JSValue access
    JSValue getJSValue() const { return val_; }
    JSContext* getContext() const { return ctx_; }
};

class Context {
private:
    JSRuntime* runtime_;
    JSContext* context_;

public:
    Context();
    ~Context();

    // Disable copy constructor and assignment
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    // Move constructor and assignment
    Context(Context&& other) noexcept;
    Context& operator=(Context&& other) noexcept;

    // Script execution
    Value eval(const std::string& code, const std::string& filename = "<eval>");
    Value evalFile(const std::string& filename);

    // Value creation
    Value newUndefined();
    Value newNull();
    Value newBool(bool value);
    Value newNumber(double value);
    Value newInt32(int32_t value);
    Value newString(const std::string& str);
    Value newObject();
    Value newArray();
    Value newArray(const std::vector<Value>& elements);

    // Global object access
    Value getGlobal();
    Value getGlobalProperty(const std::string& name);
    void setGlobalProperty(const std::string& name, const Value& value);

    // Function binding
    using NativeFunction = std::function<Value(const std::vector<Value>&)>;
    Value newFunction(const std::string& name, NativeFunction func);
    void setGlobalFunction(const std::string& name, NativeFunction func);

    // Error handling
    bool hasException() const;
    Value getException();
    std::string getExceptionString();
    void throwException(const std::string& message);

    // Memory management
    void runGC();
    size_t getMemoryUsage() const;

    // Raw access
    JSContext* getJSContext() const { return context_; }
    JSRuntime* getJSRuntime() const { return runtime_; }

private:
    Value wrapJSValue(JSValue val, bool owned = true);
    void checkException() const;
};

// Utility functions for easy value creation
namespace Utils {
    Value undefined(Context& ctx);
    Value null(Context& ctx);
    Value boolean(Context& ctx, bool value);
    Value number(Context& ctx, double value);
    Value integer(Context& ctx, int32_t value);
    Value string(Context& ctx, const std::string& str);
    Value object(Context& ctx);
    Value array(Context& ctx);
    Value array(Context& ctx, const std::vector<Value>& elements);
}

} // namespace QuickJSWrapper