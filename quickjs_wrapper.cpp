#include "quickjs_wrapper.h"
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>

namespace QuickJSWrapper {

// Global storage for native function callbacks
static std::map<int, std::unique_ptr<Context::NativeFunction>> g_native_functions;
static int g_next_function_id = 1;

// Value class implementation
Value::Value(JSContext* ctx, JSValue val, bool owned) 
    : ctx_(ctx), val_(val), owned_(owned) {
    if (owned_ && !JS_IsUninitialized(val_)) {
        val_ = JS_DupValue(ctx_, val_);
    }
}

Value::Value(const Value& other) 
    : ctx_(other.ctx_), val_(JS_DupValue(other.ctx_, other.val_)), owned_(true) {
}

Value::Value(Value&& other) noexcept 
    : ctx_(other.ctx_), val_(other.val_), owned_(other.owned_) {
    other.owned_ = false;
    other.val_ = JS_UNINITIALIZED;
}

Value& Value::operator=(const Value& other) {
    if (this != &other) {
        if (owned_ && !JS_IsUninitialized(val_)) {
            JS_FreeValue(ctx_, val_);
        }
        ctx_ = other.ctx_;
        val_ = JS_DupValue(other.ctx_, other.val_);
        owned_ = true;
    }
    return *this;
}

Value& Value::operator=(Value&& other) noexcept {
    if (this != &other) {
        if (owned_ && !JS_IsUninitialized(val_)) {
            JS_FreeValue(ctx_, val_);
        }
        ctx_ = other.ctx_;
        val_ = other.val_;
        owned_ = other.owned_;
        other.owned_ = false;
        other.val_ = JS_UNINITIALIZED;
    }
    return *this;
}

Value::~Value() {
    if (owned_ && !JS_IsUninitialized(val_)) {
        JS_FreeValue(ctx_, val_);
    }
}

bool Value::isUndefined() const {
    return JS_IsUndefined(val_);
}

bool Value::isNull() const {
    return JS_IsNull(val_);
}

bool Value::isBool() const {
    return JS_IsBool(val_);
}

bool Value::isNumber() const {
    return JS_IsNumber(val_);
}

bool Value::isString() const {
    return JS_IsString(val_);
}

bool Value::isObject() const {
    return JS_IsObject(val_);
}

bool Value::isFunction() const {
    return JS_IsFunction(ctx_, val_);
}

bool Value::isArray() const {
    return JS_IsArray(val_);
}

bool Value::toBool() const {
    return JS_ToBool(ctx_, val_);
}

int32_t Value::toInt32() const {
    int32_t result;
    if (JS_ToInt32(ctx_, &result, val_) < 0) {
        throw Exception("Failed to convert value to int32");
    }
    return result;
}

double Value::toNumber() const {
    double result;
    if (JS_ToFloat64(ctx_, &result, val_) < 0) {
        throw Exception("Failed to convert value to number");
    }
    return result;
}

std::string Value::toString() const {
    const char* str = JS_ToCString(ctx_, val_);
    if (!str) {
        throw Exception("Failed to convert value to string");
    }
    std::string result(str);
    JS_FreeCString(ctx_, str);
    return result;
}

Value Value::getProperty(const std::string& name) const {
    JSValue prop = JS_GetPropertyStr(ctx_, val_, name.c_str());
    if (JS_IsException(prop)) {
        throw Exception("Failed to get property: " + name);
    }
    return Value(ctx_, prop, true);
}

void Value::setProperty(const std::string& name, const Value& value) {
    if (JS_SetPropertyStr(ctx_, val_, name.c_str(), JS_DupValue(ctx_, value.val_)) < 0) {
        throw Exception("Failed to set property: " + name);
    }
}

Value Value::getElement(int index) const {
    JSValue elem = JS_GetPropertyUint32(ctx_, val_, index);
    if (JS_IsException(elem)) {
        throw Exception("Failed to get array element at index: " + std::to_string(index));
    }
    return Value(ctx_, elem, true);
}

void Value::setElement(int index, const Value& value) {
    if (JS_SetPropertyUint32(ctx_, val_, index, JS_DupValue(ctx_, value.val_)) < 0) {
        throw Exception("Failed to set array element at index: " + std::to_string(index));
    }
}

size_t Value::getArrayLength() const {
    Value length = getProperty("length");
    return static_cast<size_t>(length.toInt32());
}

Value Value::call(const std::vector<Value>& args) const {
    std::vector<JSValue> jsArgs;
    jsArgs.reserve(args.size());
    for (const auto& arg : args) {
        jsArgs.push_back(arg.val_);
    }
    
    JSValue result = JS_Call(ctx_, val_, JS_UNDEFINED, jsArgs.size(), jsArgs.data());
    if (JS_IsException(result)) {
        throw Exception("Function call failed");
    }
    return Value(ctx_, result, true);
}

Value Value::callMethod(const std::string& method, const std::vector<Value>& args) const {
    Value methodFunc = getProperty(method);
    if (!methodFunc.isFunction()) {
        throw Exception("Property is not a function: " + method);
    }
    
    std::vector<JSValue> jsArgs;
    jsArgs.reserve(args.size());
    for (const auto& arg : args) {
        jsArgs.push_back(arg.val_);
    }
    
    JSValue result = JS_Call(ctx_, methodFunc.val_, val_, jsArgs.size(), jsArgs.data());
    if (JS_IsException(result)) {
        throw Exception("Method call failed: " + method);
    }
    return Value(ctx_, result, true);
}

// Context class implementation
Context::Context() : runtime_(nullptr), context_(nullptr) {
    runtime_ = JS_NewRuntime();
    if (!runtime_) {
        throw Exception("Failed to create JS runtime");
    }
    
    context_ = JS_NewContext(runtime_);
    if (!context_) {
        JS_FreeRuntime(runtime_);
        throw Exception("Failed to create JS context");
    }
}

Context::~Context() {
    if (context_) {
        JS_FreeContext(context_);
    }
    if (runtime_) {
        JS_FreeRuntime(runtime_);
    }
}

Context::Context(Context&& other) noexcept 
    : runtime_(other.runtime_), context_(other.context_) {
    other.runtime_ = nullptr;
    other.context_ = nullptr;
}

Context& Context::operator=(Context&& other) noexcept {
    if (this != &other) {
        if (context_) {
            JS_FreeContext(context_);
        }
        if (runtime_) {
            JS_FreeRuntime(runtime_);
        }
        
        runtime_ = other.runtime_;
        context_ = other.context_;
        other.runtime_ = nullptr;
        other.context_ = nullptr;
    }
    return *this;
}

Value Context::eval(const std::string& code, const std::string& filename) {
    JSValue result = JS_Eval(context_, code.c_str(), code.length(), 
                            filename.c_str(), JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
        std::string errorMsg = getExceptionString();
        throw Exception("Script evaluation failed: " + errorMsg);
    }
    return wrapJSValue(result, true);
}

Value Context::evalFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw Exception("Failed to open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return eval(buffer.str(), filename);
}

Value Context::newUndefined() {
    return wrapJSValue(JS_UNDEFINED, false);
}

Value Context::newNull() {
    return wrapJSValue(JS_NULL, false);
}

Value Context::newBool(bool value) {
    return wrapJSValue(JS_NewBool(context_, value), true);
}

Value Context::newNumber(double value) {
    return wrapJSValue(JS_NewFloat64(context_, value), true);
}

Value Context::newInt32(int32_t value) {
    return wrapJSValue(JS_NewInt32(context_, value), true);
}

Value Context::newString(const std::string& str) {
    return wrapJSValue(JS_NewString(context_, str.c_str()), true);
}

Value Context::newObject() {
    return wrapJSValue(JS_NewObject(context_), true);
}

Value Context::newArray() {
    return wrapJSValue(JS_NewArray(context_), true);
}

Value Context::newArray(const std::vector<Value>& elements) {
    JSValue arr = JS_NewArray(context_);
    for (size_t i = 0; i < elements.size(); ++i) {
        JS_SetPropertyUint32(context_, arr, i, JS_DupValue(context_, elements[i].getJSValue()));
    }
    return wrapJSValue(arr, true);
}

Value Context::getGlobal() {
    return wrapJSValue(JS_GetGlobalObject(context_), true);
}

Value Context::getGlobalProperty(const std::string& name) {
    JSValue global = JS_GetGlobalObject(context_);
    JSValue prop = JS_GetPropertyStr(context_, global, name.c_str());
    JS_FreeValue(context_, global);
    
    if (JS_IsException(prop)) {
        throw Exception("Failed to get global property: " + name);
    }
    return wrapJSValue(prop, true);
}

void Context::setGlobalProperty(const std::string& name, const Value& value) {
    JSValue global = JS_GetGlobalObject(context_);
    int result = JS_SetPropertyStr(context_, global, name.c_str(), 
                                  JS_DupValue(context_, value.getJSValue()));
    JS_FreeValue(context_, global);
    
    if (result < 0) {
        throw Exception("Failed to set global property: " + name);
    }
}

JSValue nativeFunctionCallback(JSContext* ctx, JSValueConst, 
                              int argc, JSValueConst* argv, 
                              int magic) {
    try {
        // Find the native function
        auto it = g_native_functions.find(magic);
        if (it == g_native_functions.end()) {
            return JS_ThrowInternalError(ctx, "Native function not found");
        }
        
        // Convert arguments
        std::vector<Value> args;
        args.reserve(argc);
        for (int i = 0; i < argc; ++i) {
            args.emplace_back(ctx, argv[i], false);
        }
        
        // Call the native function
        Value result = (*it->second)(args);
        return JS_DupValue(ctx, result.getJSValue());
        
    } catch (const Exception& e) {
        return JS_ThrowInternalError(ctx, "%s", e.what());
    } catch (const std::exception& e) {
        return JS_ThrowInternalError(ctx, "%s", e.what());
    } catch (...) {
        return JS_ThrowInternalError(ctx, "Unknown error in native function");
    }
}

Value Context::newFunction(const std::string& name, NativeFunction func) {
    // Store the function with a unique ID
    int functionId = g_next_function_id++;
    g_native_functions[functionId] = std::make_unique<NativeFunction>(std::move(func));
    
    // Create the JS function with the ID as magic number
    JSValue jsFunc = JS_NewCFunctionMagic(context_, nativeFunctionCallback, 
                                          name.c_str(), 0, JS_CFUNC_generic_magic, 
                                          functionId);
    
    return wrapJSValue(jsFunc, true);
}

void Context::setGlobalFunction(const std::string& name, NativeFunction func) {
    Value jsFunc = newFunction(name, std::move(func));
    setGlobalProperty(name, jsFunc);
}

bool Context::hasException() const {
    return JS_IsException(JS_GetException(context_));
}

Value Context::getException() {
    return wrapJSValue(JS_GetException(context_), true);
}

std::string Context::getExceptionString() {
    JSValue exception = JS_GetException(context_);
    if (JS_IsNull(exception)) {
        return "No exception";
    }
    
    const char* str = JS_ToCString(context_, exception);
    std::string result = str ? str : "Unknown exception";
    if (str) {
        JS_FreeCString(context_, str);
    }
    JS_FreeValue(context_, exception);
    
    return result;
}

void Context::throwException(const std::string& message) {
    JS_ThrowInternalError(context_, "%s", message.c_str());
}

void Context::runGC() {
    JS_RunGC(runtime_);
}

size_t Context::getMemoryUsage() const {
    JSMemoryUsage usage;
    JS_ComputeMemoryUsage(runtime_, &usage);
    return usage.memory_used_size;
}

Value Context::wrapJSValue(JSValue val, bool owned) {
    return Value(context_, val, owned);
}

void Context::checkException() const {
    if (hasException()) {
        throw Exception("JavaScript exception occurred");
    }
}

// Utility functions
namespace Utils {
    Value undefined(Context& ctx) {
        return ctx.newUndefined();
    }
    
    Value null(Context& ctx) {
        return ctx.newNull();
    }
    
    Value boolean(Context& ctx, bool value) {
        return ctx.newBool(value);
    }
    
    Value number(Context& ctx, double value) {
        return ctx.newNumber(value);
    }
    
    Value integer(Context& ctx, int32_t value) {
        return ctx.newInt32(value);
    }
    
    Value string(Context& ctx, const std::string& str) {
        return ctx.newString(str);
    }
    
    Value object(Context& ctx) {
        return ctx.newObject();
    }
    
    Value array(Context& ctx) {
        return ctx.newArray();
    }
    
    Value array(Context& ctx, const std::vector<Value>& elements) {
        return ctx.newArray(elements);
    }
}

} // namespace QuickJSWrapper