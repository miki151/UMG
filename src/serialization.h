#pragma once



#define SERIAL(X) X

#define SERIALIZE_ALL(...) \
  template <class Archive> \
  void serialize(Archive& ar1, const unsigned int) { \
    ar1(__VA_ARGS__); \
  }

#define SERIALIZE_EMPTY() \
  template <class Archive> \
  void serialize(Archive&, const unsigned int) { \
  }


template <typename T>
struct NameValuePair {
  const char* name;
  T& value;
};

template <typename T>
struct OptionalNameValuePair {
  const char* name;
  T& value;
};

template <typename T>
NameValuePair<T> make_nvp(T& t, const char* name) {
  return NameValuePair<T>{name, t};
}

template <typename T>
OptionalNameValuePair<T> make_optional_nvp(T& t, const char* name) {
  return OptionalNameValuePair<T>{name, t};
}

#define NAMED(x) make_nvp(x, #x)
#define OPTION(x) make_optional_nvp(x, #x)
