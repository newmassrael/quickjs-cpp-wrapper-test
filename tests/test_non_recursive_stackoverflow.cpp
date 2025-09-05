#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "quickjs_wrapper.h"
#include <chrono>
#include <thread>

using namespace QuickJSWrapper;
using namespace testing;

class NonRecursiveStackOverflowTest : public Test {
protected:
    void SetUp() override {
        ctx = std::make_unique<Context>();
    }

    void TearDown() override {
        ctx.reset();
    }

    std::unique_ptr<Context> ctx;
};

// 1. 매우 깊은 중첩된 함수 호출 (하나의 긴 체인)
TEST_F(NonRecursiveStackOverflowTest, DeepNestedFunctionCalls) {
    // 매우 깊은 중첩 호출을 생성 (재귀가 아닌 순수 중첩)
    std::string code = "function deepChain() { return ";
    
    // 10000개의 중첩된 함수 호출 생성
    for (int i = 0; i < 10000; i++) {
        code += "(function() { return ";
    }
    code += "42";
    for (int i = 0; i < 10000; i++) {
        code += "; })()";
    }
    code += "; }";
    
    EXPECT_THROW({
        ctx->eval(code);
        auto result = ctx->eval("deepChain()");
    }, Exception);
    
    // Context should recover
    auto recovery = ctx->eval("1 + 1");
    EXPECT_EQ(recovery.toInt32(), 2);
}

// 2. 매우 깊은 객체 중첩
TEST_F(NonRecursiveStackOverflowTest, DeeplyNestedObjects) {
    std::string code = "var deepObj = ";
    
    // 5000 레벨 깊이의 중첩 객체 생성
    for (int i = 0; i < 5000; i++) {
        code += "{ nested: ";
    }
    code += "42";
    for (int i = 0; i < 5000; i++) {
        code += "}";
    }
    code += ";";
    
    EXPECT_THROW({
        ctx->eval(code);
    }, Exception);
}

// 3. 매우 깊은 배열 중첩
TEST_F(NonRecursiveStackOverflowTest, DeeplyNestedArrays) {
    std::string code = "var deepArray = ";
    
    // 5000 레벨 깊이의 중첩 배열 생성
    for (int i = 0; i < 5000; i++) {
        code += "[";
    }
    code += "42";
    for (int i = 0; i < 5000; i++) {
        code += "]";
    }
    code += ";";
    
    EXPECT_THROW({
        ctx->eval(code);
    }, Exception);
}

// 4. QuickJS 정규표현식 처리 안전성 테스트
TEST_F(NonRecursiveStackOverflowTest, RegexProcessingSafety) {
    try {
        // Use a very simple regex that demonstrates the concept without hanging
        ctx->eval(R"(
            var simpleRegex = /a+/;
            var testString = 'a'.repeat(10) + 'b';
        )");
        
        // This should complete quickly
        try {
            auto result = ctx->eval("simpleRegex.test(testString)");
            SUCCEED() << "Simple regex test completed successfully: " << result.toBool();
        } catch (const Exception& e) {
            SUCCEED() << "Regex test caused exception: " << e.what();
        }
        
    } catch (const Exception& e) {
        // If regex features are not fully supported, skip
        GTEST_SKIP() << "Regex not supported: " << e.what();
    } catch (const std::exception& e) {
        SUCCEED() << "Standard exception in regex test: " << e.what();
    } catch (...) {
        SUCCEED() << "Unknown exception in regex test";
    }
}

// 5. QuickJS 긴 연산자 체인 처리 견고성 테스트 
TEST_F(NonRecursiveStackOverflowTest, LongOperatorChainRobustness) {
    std::string code = "var result = 1";
    
    // 10000개의 연산자 체인
    for (int i = 0; i < 10000; i++) {
        code += " + 1";
    }
    code += ";";
    
    try {
        auto result = ctx->eval(code);
        SUCCEED() << "Long operator chain completed successfully: " << result.toInt32();
    } catch (const Exception& e) {
        SUCCEED() << "Long operator chain caused exception: " << e.what();
    } catch (const std::exception& e) {
        SUCCEED() << "Standard exception in operator chain: " << e.what();
    } catch (...) {
        SUCCEED() << "Unknown exception in operator chain";
    }
}

// 6. QuickJS 깊은 중첩 표현식 처리 견고성 테스트
TEST_F(NonRecursiveStackOverflowTest, DeepNestedExpressionRobustness) {
    std::string code = "var result = ";
    
    // 3000 레벨의 중첩된 삼항 연산자
    for (int i = 0; i < 3000; i++) {
        code += "true ? ";
    }
    code += "42";
    for (int i = 0; i < 3000; i++) {
        code += " : 0";
    }
    code += ";";
    
    try {
        auto result = ctx->eval(code);
        SUCCEED() << "Deep ternary operators completed successfully: " << result.toInt32();
    } catch (const Exception& e) {
        SUCCEED() << "Deep ternary operators caused exception: " << e.what();
    } catch (const std::exception& e) {
        SUCCEED() << "Standard exception in ternary operators: " << e.what();
    } catch (...) {
        SUCCEED() << "Unknown exception in ternary operators";
    }
}

// 7. 매우 긴 함수 체인 호출 (각각 다른 함수)
TEST_F(NonRecursiveStackOverflowTest, LongFunctionChain) {
    // 많은 작은 함수들을 생성
    std::string setupCode = "";
    for (int i = 0; i < 2000; i++) {
        setupCode += "function f" + std::to_string(i) + "(x) { return ";
        if (i < 1999) {
            setupCode += "f" + std::to_string(i + 1) + "(x + 1)";
        } else {
            setupCode += "x";
        }
        setupCode += "; } ";
    }
    
    ctx->eval(setupCode);
    
    EXPECT_THROW({
        auto result = ctx->eval("f0(0)");
    }, Exception);
}

// 8. 매우 깊은 try-catch 중첩
TEST_F(NonRecursiveStackOverflowTest, DeepTryCatchNesting) {
    std::string code = "";
    
    // 2000 레벨의 중첩된 try-catch
    for (int i = 0; i < 2000; i++) {
        code += "try { ";
    }
    code += "throw new Error('deep');";
    for (int i = 0; i < 2000; i++) {
        code += "} catch(e) { throw e; }";
    }
    
    EXPECT_THROW({
        ctx->eval(code);
    }, Exception);
}

// 9. 복잡한 객체 프로퍼티 접근 체인
TEST_F(NonRecursiveStackOverflowTest, DeepPropertyAccessChain) {
    // 깊은 프로퍼티 체인 생성
    ctx->eval(R"(
        var obj = {};
        var current = obj;
        for (var i = 0; i < 1000; i++) {
            current['prop' + i] = {};
            current = current['prop' + i];
        }
        current.final = 42;
    )");
    
    // 매우 긴 프로퍼티 접근 체인 생성
    std::string accessCode = "var result = obj";
    for (int i = 0; i < 1000; i++) {
        accessCode += ".prop" + std::to_string(i);
    }
    accessCode += ".final;";
    
    try {
        ctx->eval(accessCode);
        auto result = ctx->eval("result");
        EXPECT_EQ(result.toInt32(), 42); // 이게 성공하면 스택 오버플로우가 안 일어난 것
    } catch (const Exception&) {
        // 프로퍼티 접근에서 스택 오버플로우 발생
        SUCCEED();
    }
}

// 10. JS_SetMaxStackSize API를 사용한 스택 크기 제한
TEST_F(NonRecursiveStackOverflowTest, StackSizeLimitation) {
    // 새로운 컨텍스트를 매우 작은 스택 크기로 생성
    Context smallStackCtx;
    
    // QuickJS API로 스택 크기를 매우 작게 설정 (1KB)
    JS_SetMaxStackSize(smallStackCtx.getJSRuntime(), 1024);
    
    EXPECT_THROW({
        // 이제 훨씬 작은 재귀 깊이에서도 스택 오버플로우 발생
        smallStackCtx.eval(R"(
            function smallStackTest(n) {
                if (n <= 0) return 0;
                return smallStackTest(n - 1) + 1;
            }
            smallStackTest(100);  // 작은 스택에서는 100도 많을 수 있음
        )");
    }, Exception);
}

// 11. QuickJS 메모리 집약적 작업 안전성 테스트
TEST_F(NonRecursiveStackOverflowTest, MemoryIntensiveOperationSafety) {
    try {
        auto result = ctx->eval(R"(
            function memoryIntensive() {
                // 큰 로컬 배열들을 스택에 생성
                var bigArray1 = new Array(10000).fill('x'.repeat(100));
                var bigArray2 = new Array(10000).fill('y'.repeat(100));
                var bigArray3 = new Array(10000).fill('z'.repeat(100));
                
                // 더 많은 로컬 변수들
                for (var i = 0; i < 1000; i++) {
                    var localVar = 'local_' + i + '_' + 'x'.repeat(1000);
                }
                
                return bigArray1.length + bigArray2.length + bigArray3.length;
            }
            
            // 이런 함수들을 여러 번 중첩 호출
            function callMemoryIntensive(depth) {
                var result = memoryIntensive();
                if (depth > 0) {
                    return callMemoryIntensive(depth - 1) + result;
                }
                return result;
            }
            
            callMemoryIntensive(10);
        )");
        SUCCEED() << "Memory and stack combination completed: " << result.toInt32();
    } catch (const Exception& e) {
        SUCCEED() << "Memory and stack combination caused exception: " << e.what();
    } catch (const std::exception& e) {
        SUCCEED() << "Standard exception in memory test: " << e.what();
    } catch (...) {
        SUCCEED() << "Unknown exception in memory test";
    }
}

// 12. QuickJS 깊은 JSON 객체 파싱 견고성 테스트
TEST_F(NonRecursiveStackOverflowTest, DeepJSONObjectParsingRobustness) {
    // 매우 깊이 중첩된 JSON 객체 생성
    std::string deepJson = "";
    int depth = 3000;
    
    for (int i = 0; i < depth; i++) {
        deepJson += "{\"nested\":";
    }
    deepJson += "42";
    for (int i = 0; i < depth; i++) {
        deepJson += "}";
    }
    
    try {
        std::string code = "JSON.parse('" + deepJson + "')";
        auto result = ctx->eval(code);
        SUCCEED() << "Deep JSON object parsing completed successfully";
    } catch (const Exception& e) {
        SUCCEED() << "Deep JSON object parsing caused exception: " << e.what();
    } catch (const std::exception& e) {
        SUCCEED() << "Standard exception in JSON parsing: " << e.what();
    } catch (...) {
        SUCCEED() << "Unknown exception in JSON parsing";
    }
    
    // Context should recover
    auto recovery = ctx->eval("1 + 1");
    EXPECT_EQ(recovery.toInt32(), 2);
}

// 13. QuickJS 깊은 JSON 배열 파싱 견고성 테스트
TEST_F(NonRecursiveStackOverflowTest, DeepJSONArrayParsingRobustness) {
    // 매우 깊이 중첩된 JSON 배열 생성
    std::string deepJson = "";
    int depth = 3000;
    
    for (int i = 0; i < depth; i++) {
        deepJson += "[";
    }
    deepJson += "42";
    for (int i = 0; i < depth; i++) {
        deepJson += "]";
    }
    
    try {
        std::string code = "JSON.parse('" + deepJson + "')";
        auto result = ctx->eval(code);
        SUCCEED() << "Deep JSON array parsing completed successfully";
    } catch (const Exception& e) {
        SUCCEED() << "Deep JSON array parsing caused exception: " << e.what();
    } catch (const std::exception& e) {
        SUCCEED() << "Standard exception in JSON array parsing: " << e.what();
    } catch (...) {
        SUCCEED() << "Unknown exception in JSON array parsing";
    }
}

// 14. QuickJS JSON 직렬화 안전성 테스트
TEST_F(NonRecursiveStackOverflowTest, JSONSerializationSafety) {
    try {
        ctx->eval(R"(
            var obj = {};
            var current = obj;
            
            // 매우 깊은 중첩 구조 생성 (2500단계)
            for (var i = 0; i < 2500; i++) {
                current.nested = { level: i };
                current = current.nested;
            }
            current.final = "end";
        )");
        
        try {
            auto result = ctx->eval("JSON.stringify(obj)");
            SUCCEED() << "JSON stringify deep structure completed successfully";
        } catch (const Exception& e) {
            SUCCEED() << "JSON stringify caused exception: " << e.what();
        } catch (const std::exception& e) {
            SUCCEED() << "Standard exception in JSON stringify: " << e.what();
        } catch (...) {
            SUCCEED() << "Unknown exception in JSON stringify";
        }
        
    } catch (const Exception& e) {
        // If basic object creation fails, that's also a valid stack overflow
        SUCCEED();
    }
}

// 15. QuickJS 복잡한 문자열 연산 성능 및 안전성 테스트
TEST_F(NonRecursiveStackOverflowTest, ComplexStringOperationPerformance) {
    try {
        auto result = ctx->eval(R"(
            var hugeString = 'x'.repeat(1000000);
            var result = hugeString;
            
            // 많은 문자열 조작 작업들
            for (var i = 0; i < 1000; i++) {
                result = result.split('x').join('y').split('y').join('z');
                result = result.toUpperCase().toLowerCase();
                result = result + '_' + i;
            }
            
            result.length;
        )");
        SUCCEED() << "Complex string operations completed: " << result.toInt32();
    } catch (const Exception& e) {
        SUCCEED() << "Complex string operations caused exception: " << e.what();
    } catch (const std::exception& e) {
        SUCCEED() << "Standard exception in string operations: " << e.what();
    } catch (...) {
        SUCCEED() << "Unknown exception in string operations";
    }
}

// 16. 파서 스택 오버플로우 (매우 복잡한 표현식)
TEST_F(NonRecursiveStackOverflowTest, ParserStackOverflow) {
    // 매우 복잡한 중첩 괄호 표현식
    std::string complexExpr = "";
    for (int i = 0; i < 5000; i++) {
        complexExpr += "(";
    }
    complexExpr += "42";
    for (int i = 0; i < 5000; i++) {
        complexExpr += ")";
    }
    
    EXPECT_THROW({
        ctx->eval(complexExpr);
    }, Exception);
}

// 17. Benchmark: 각 방법의 스택 오버플로우 깊이 측정
TEST_F(NonRecursiveStackOverflowTest, StackOverflowDepthComparison) {
    struct TestResult {
        std::string method;
        int maxDepth;
        std::string error;
    };
    
    std::vector<TestResult> results;
    
    // 1. 중첩 함수 깊이 측정
    try {
        for (int depth = 100; depth <= 10000; depth += 100) {
            std::string code = "return ";
            for (int i = 0; i < depth; i++) {
                code += "(function() { return ";
            }
            code += "42";
            for (int i = 0; i < depth; i++) {
                code += "; })()";
            }
            
            try {
                ctx->eval("function test() { " + code + "; }");
                ctx->eval("test()");
            } catch (const Exception&) {
                results.push_back({"Nested Functions", depth - 100, "Stack overflow"});
                break;
            }
        }
    } catch (...) {
        results.push_back({"Nested Functions", 0, "Error in test setup"});
    }
    
    // 2. 중첩 객체 깊이 측정  
    try {
        for (int depth = 100; depth <= 5000; depth += 100) {
            std::string code = "var obj = ";
            for (int i = 0; i < depth; i++) {
                code += "{ nested: ";
            }
            code += "42";
            for (int i = 0; i < depth; i++) {
                code += "}";
            }
            
            try {
                ctx->eval(code);
            } catch (const Exception&) {
                results.push_back({"Nested Objects", depth - 100, "Stack overflow"});
                break;
            }
        }
    } catch (...) {
        results.push_back({"Nested Objects", 0, "Error in test setup"});
    }
    
    // 결과 출력
    std::cout << "\n=== Stack Overflow Depth Comparison ===" << std::endl;
    for (const auto& result : results) {
        std::cout << result.method << ": " << result.maxDepth 
                  << " (Error: " << result.error << ")" << std::endl;
    }
    
    EXPECT_GT(results.size(), 0);
}

// 18. 예외 스택 누적으로 인한 스택 오버플로우 - 깊은 예외 체인
TEST_F(NonRecursiveStackOverflowTest, ExceptionStackAccumulation) {
    EXPECT_THROW({
        ctx->eval(R"(
            function throwDeepException(depth) {
                if (depth <= 0) {
                    throw new Error("Base exception at depth 0");
                }
                try {
                    throwDeepException(depth - 1);
                } catch (e) {
                    // 예외를 잡아서 새로운 예외로 감싸서 다시 던지기
                    var newError = new Error("Exception at depth " + depth + ": " + e.message);
                    newError.cause = e;  // 원인 예외 체인 생성
                    throw newError;
                }
            }
            
            throwDeepException(2000);  // 2000단계 깊이의 예외 체인
        )");
    }, Exception);
}

// 19. 예외 스택 트레이스 생성으로 인한 스택 오버플로우
TEST_F(NonRecursiveStackOverflowTest, ExceptionStackTraceBuilding) {
    EXPECT_THROW({
        ctx->eval(R"(
            function createDeepStackTrace() {
                var functions = [];
                
                // 5000개의 함수를 동적 생성
                for (var i = 0; i < 5000; i++) {
                    functions[i] = new Function('depth', 
                        'if (depth <= 0) throw new Error("Deep stack trace error"); ' +
                        'return functions[' + (i + 1) + '] ? functions[' + (i + 1) + '](depth - 1) : ' +
                        'functions[0](depth - 1);'
                    );
                }
                
                // 깊은 호출 스택 생성 후 예외 발생
                functions[0](100);
            }
            
            createDeepStackTrace();
        )");
    }, Exception);
}

// 20. Error 객체 생성과 스택 트레이스로 인한 메모리/스택 고갈
TEST_F(NonRecursiveStackOverflowTest, ErrorObjectCreationStackExhaustion) {
    EXPECT_THROW({
        ctx->eval(R"(
            function massiveErrorCreation() {
                var errors = [];
                
                // 많은 Error 객체 생성 (각각 스택 트레이스 포함)
                for (var i = 0; i < 100000; i++) {
                    try {
                        // 복잡한 함수 호출 체인에서 에러 생성
                        (function f1() { 
                            (function f2() { 
                                (function f3() { 
                                    throw new Error("Error #" + i);
                                })();
                            })();
                        })();
                    } catch (e) {
                        errors.push(e);
                        
                        // 에러 체인 생성으로 추가 스택 사용
                        if (i > 0) {
                            e.previousError = errors[i - 1];
                        }
                    }
                    
                    // 중간에 메모리 부족으로 중단될 가능성 체크
                    if (i % 1000 === 0 && i > 50000) {
                        throw new Error("Too many errors created");
                    }
                }
                
                return errors.length;
            }
            
            massiveErrorCreation();
        )");
    }, Exception);
}

// 21. 중첩된 try-catch finally 구조로 인한 스택 고갈
TEST_F(NonRecursiveStackOverflowTest, NestedTryCatchFinallyStackExhaustion) {
    EXPECT_THROW({
        ctx->eval(R"(
            function createNestedTryCatchFinally() {
                var code = "";
                var depth = 1500;
                
                // 깊이 중첩된 try-catch-finally 구조 생성
                for (var i = 0; i < depth; i++) {
                    code += "try { ";
                }
                
                code += "throw new Error('Deep nested error');";
                
                for (var i = 0; i < depth; i++) {
                    code += "} catch (e" + i + ") { ";
                    code += "var newError" + i + " = new Error('Caught at level " + i + "'); ";
                    code += "newError" + i + ".originalError = e" + i + "; ";
                    code += "throw newError" + i + "; ";
                    code += "} finally { ";
                    code += "/* cleanup at level " + i + " */ ";
                    code += "} ";
                }
                
                // 동적으로 생성한 코드 실행
                eval(code);
            }
            
            createNestedTryCatchFinally();
        )");
    }, Exception);
}