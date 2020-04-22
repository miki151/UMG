
namespace variant_helpers {

template <typename... Lambdas>
struct LambdaVisitor;

template <typename Lambda1, typename... Lambdas>
struct LambdaVisitor<Lambda1 , Lambdas...> : public Lambda1, public LambdaVisitor<Lambdas...> {
  using Lambda1::operator();
  using LambdaVisitor<Lambdas...>::operator();
  LambdaVisitor(Lambda1 l1, Lambdas... lambdas)
    : Lambda1(l1), LambdaVisitor<Lambdas...> (lambdas...)
  {}
};

template <typename Lambda1>
struct LambdaVisitor<Lambda1> : public Lambda1 {
  using Lambda1::operator();
  LambdaVisitor(Lambda1 l1) :Lambda1(l1)
  {}
};
}

struct VARIANT_NAME {
  VARIANT_NAME() : index(0), elem0() {}
  int index;
  template <typename T>
  optional<T> getValueMaybe() const {
    return none;
  }
  template <typename T>
  optional<const T&> getReferenceMaybe() const {
    return none;
  }
  template <typename T>
  bool contains() const {
    return !!getReferenceMaybe<T>();
  }
#define X(Type, Index) \
  VARIANT_NAME(Type&& t) noexcept : index(Index), elem##Index(std::move(t)) {}\
  VARIANT_NAME(const Type& t) noexcept : index(Index), elem##Index(t) {}
  VARIANT_TYPES_LIST
#undef X

  template<typename RetType = void, typename... Fs>
  RetType visit(Fs... fs) const {
    auto f = variant_helpers::LambdaVisitor<Fs...>(fs...);
    switch (index) {
#define X(Type, Index)\
      case Index: return f(elem##Index); break;
      VARIANT_TYPES_LIST
#undef X
      default: fail();
    }
  }
  VARIANT_NAME(const VARIANT_NAME& t) noexcept : index(t.index) {
    switch (index) {
#define X(Type, Index)\
      case Index: new(&elem##Index) Type(t.elem##Index); break;
      VARIANT_TYPES_LIST
#undef X
      default: fail();
    }
  }
  VARIANT_NAME(VARIANT_NAME&& t) noexcept : index(t.index) {
    switch (index) {
#define X(Type, Index)\
      case Index: new(&elem##Index) Type(std::move(t.elem##Index)); break;
      VARIANT_TYPES_LIST
#undef X
      default: fail();
    }
  }
  VARIANT_NAME& operator = (const VARIANT_NAME& t) noexcept {
    if (index == t.index)
      switch (index) {
  #define X(Type, Index)\
        case Index: elem##Index = t.elem##Index; break;
        VARIANT_TYPES_LIST
  #undef X
        default: fail();
      }
    else {
      this->~VARIANT_NAME();
      new (this) VARIANT_NAME(t);
    }
    return *this;
  }
  VARIANT_NAME& operator = (VARIANT_NAME&& t) noexcept {
    if (index == t.index)
      switch (index) {
  #define X(Type, Index)\
        case Index: elem##Index = std::move(t.elem##Index); break;
        VARIANT_TYPES_LIST
  #undef X
        default: fail();
      }
    else {
      this->~VARIANT_NAME();
      new (this) VARIANT_NAME(std::move(t));
    }
    return *this;
  }
  ~VARIANT_NAME() {
    switch (index) {
#define X(Type, Index)\
      case Index: elem##Index.~Type(); break;
      VARIANT_TYPES_LIST
#undef X
      default: fail();
    }
  }
  union {
#define X(Type, Index) \
    Type elem##Index;
    VARIANT_TYPES_LIST
#undef X
  };
};
#define X(Type, Index) \
  template<>\
  inline optional<Type> VARIANT_NAME::getValueMaybe<Type>() const {\
    if (index == Index)\
      return elem##Index;\
    return none;\
  }\
  template<>\
  inline optional<const Type&> VARIANT_NAME::getReferenceMaybe() const {\
    if (index == Index)\
      return elem##Index;\
    return none;\
  }
  VARIANT_TYPES_LIST
#undef X
