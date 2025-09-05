# QuickJS 안전성 검증 테스트

QuickJS JavaScript 엔진의 안전성과 견고성을 종합적으로 검증하는 테스트 프로젝트입니다. 테스트를 위해 현대적인 C++ 래퍼를 구현하여 사용합니다.

## 주요 기능

- **QuickJS 안전성 검증**: 복잡한 작업 시 엔진의 견고성과 안전성 확인
- **C++ 래퍼 구현**: 테스트를 위해 RAII 기반 현대적 C++ 인터페이스 제공
- **포괄적인 안전성 테스팅**: QuickJS가 복잡한 작업을 안전하게 처리하는지 검증하는 64개의 테스트 케이스
- **예외 처리**: JavaScript 오류에서 적절한 C++ 예외 변환
- **네이티브 함수 바인딩**: C++ 함수를 JavaScript로 쉽게 바인딩
- **메모리 관리**: 내장된 가비지 컬렉션 및 메모리 사용량 모니터링

## 빌드 방법

```bash
# 서브모듈과 함께 클론
git clone --recursive https://github.com/newmassrael/quickjs-cpp-wrapper-test.git
cd quickjs-cpp-wrapper-test

# 빌드
mkdir build && cd build
cmake ..
make -j4
```

## 사용법

### 기본 예제

```cpp
#include "quickjs_wrapper.h"

using namespace QuickJSWrapper;

int main() {
    try {
        Context ctx;
        
        // JavaScript 실행
        auto result = ctx.eval("2 + 3 * 4");
        std::cout << "결과: " << result.toInt32() << std::endl; // 14
        
        // 객체 생성 및 조작
        auto obj = ctx.newObject();
        obj.setProperty("name", ctx.newString("안녕하세요"));
        obj.setProperty("value", ctx.newInt32(42));
        
        // 네이티브 함수 바인딩
        ctx.setGlobalFunction("add", [](const std::vector<Value>& args) -> Value {
            if (args.size() != 2) throw Exception("add()는 2개의 인자가 필요합니다");
            double a = args[0].toNumber();
            double b = args[1].toNumber();
            JSContext* js_ctx = args[0].getContext();
            return Value(js_ctx, JS_NewFloat64(js_ctx, a + b), true);
        });
        
        auto sum = ctx.eval("add(10, 32)");
        std::cout << "합계: " << sum.toInt32() << std::endl; // 42
        
    } catch (const Exception& e) {
        std::cerr << "오류: " << e.what() << std::endl;
    }
    
    return 0;
}
```

## 테스팅

프로젝트는 QuickJS의 안전성과 견고성을 검증하는 광범위한 테스트를 포함합니다:

```bash
# 모든 테스트 실행
./quickjs_wrapper_tests

# 특정 안전성 검증 카테고리 실행
./quickjs_wrapper_tests --gtest_filter="BasicFunctionalityTest.*"
./quickjs_wrapper_tests --gtest_filter="*Safety*"
```

### QuickJS 안전성 검증 테스트 카테고리

1. **기본 기능**: 핵심 래퍼 기능의 안전성 테스트
2. **재귀 처리 안전성**: QuickJS가 복잡한 재귀 패턴을 안전하게 처리하는지 검증
3. **복잡한 작업 처리 안전성**: QuickJS가 고강도 작업을 견고하게 처리하는지 검증:
   - 깊이 중첩된 함수 호출 안전 처리
   - 복잡한 객체/배열 구조 안전 관리
   - 파서 강건성 검증
   - 정규 표현식 처리 안전성
   - 예외 처리 체인 견고성
   - 메모리 관리 안전성

### 검증된 안전성 지표

- **안전한 최대 재귀 깊이**: 1,281 (측정됨)
- **21가지 복잡한 작업 안전성** 검증
- **13가지 재귀 패턴 안전성** 검증
- **모든 테스트 통과율**: 100% (64/64 테스트)

### 주요 안전성 검증 방법

**복잡한 작업 처리 안전성 검증:**
1. 깊이 중첩된 함수 체인 안전 처리 (10,000+ 중첩)
2. 복잡한 객체 구조 안전 관리
3. 거대한 배열 리터럴 안전 처리 (100,000+ 요소)
4. 정규 표현식 복잡도 안전 처리
5. 예외 처리 체인 견고성 검증
6. JSON 파싱/직렬화 안전성 검증
7. 클로저 체인 안전 처리
8. 프로토타입 체인 안전 관리

**재귀 패턴 처리 안전성 검증:**
1. 단순 재귀 함수 안전 처리
2. 상호 재귀 함수 안전성 검증
3. 네이티브-JavaScript 혼합 재귀 안전성
4. 객체 메서드 재귀 안전 처리
5. 생성자 함수 재귀 안전성
6. 클로저 재귀 패턴 안전 처리

## 요구사항

- C++17 또는 이상
- CMake 3.12+
- Google Test (서브모듈로 포함됨)
- QuickJS (서브모듈로 포함됨)

## 기술적 특징

### RAII 패턴 구현
- 자동 메모리 관리
- 예외 안전성 보장
- 리소스 누수 방지

### QuickJS 엔진 안전성 분석
QuickJS 엔진에서 안전성이 중요한 8가지 주요 영역을 분석하여 검증 테스트 케이스를 구성:

1. **함수 호출 스택**: 일반적인 JavaScript 함수 호출
2. **파서 스택**: JavaScript 코드 파싱 시 사용되는 내부 스택
3. **평가 스택**: 표현식 평가를 위한 연산자 스택
4. **예외 처리 스택**: try-catch 블록의 중첩된 예외 처리
5. **가비지 컬렉터 스택**: 순환 참조 해결을 위한 내부 스택
6. **정규 표현식 엔진**: 백트래킹을 위한 상태 스택
7. **JSON 파서**: 중첩된 객체/배열 파싱용 스택
8. **바이트코드 인터프리터**: 가상 머신 실행 스택

## 라이센스

이 프로젝트는 MIT 라이센스에 따라 라이센스가 부여됩니다. 자세한 내용은 LICENSE 파일을 참조하세요.

## 서브모듈 의존성

이 프로젝트는 다음 라이브러리들을 git 서브모듈로 사용합니다:

### QuickJS-NG
- **저장소**: https://github.com/quickjs-ng/quickjs
- **목적**: 테스트 대상 JavaScript 엔진
- **버전**: v0.10.1+ 기반

### Google Test
- **저장소**: https://github.com/google/googletest
- **목적**: C++ 단위 테스트 프레임워크
- **버전**: release-1.8.0+ 기반